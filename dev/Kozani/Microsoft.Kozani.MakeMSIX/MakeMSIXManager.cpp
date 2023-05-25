// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MakeMSIXManager.h"
#include "MakeMSIXManager.g.cpp"
#include "CreateKozaniPackageProvider.hpp"
#include <filesystem>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.streams.h>
#include <msopc.h>
#include <AppxPackaging.h>
#include "ContentType.hpp"
#include "Constants.hpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    HRESULT ValidateCreatePackageArguments(hstring directoryPathToPack, PackOptions packOptions)
    {
        if (!std::filesystem::is_directory(directoryPathToPack.c_str()))
        {
            return E_INVALIDARG;
        }
        return S_OK;
    }

    HRESULT CreateGUIDString(std::wstring& guidString)
    {
        GUID newGuid;
        THROW_IF_FAILED(CoCreateGuid(&newGuid));

        wil::unique_cotaskmem_string newGuidString;
        THROW_IF_FAILED(StringFromCLSID(newGuid, &newGuidString));
        guidString.append(newGuidString.get());
        return S_OK;
    }

    /// <summary>
    /// Create a new directory in the temp folder using a guid.
    /// </summary>
    /// <param name="tempDirPathString"></param>
    /// <returns></returns>
    HRESULT CreateTempDirectory(_Out_ std::wstring& tempDirPathString)
    {
        // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
        // Append long path prefix to temporary directory path to handle paths that exceed the maximum path length limit
        std::wstring uniqueIdString{};
        RETURN_IF_FAILED(CreateGUIDString(uniqueIdString));
        std::filesystem::path tempDirPath{ std::filesystem::temp_directory_path() };
        tempDirPath /= uniqueIdString;

        std::error_code createDirectoryError;
        bool createTempDirResult = std::filesystem::create_directory(tempDirPath, createDirectoryError);
        if (!createTempDirResult || createDirectoryError.value() != 0)
        {
            return E_UNEXPECTED;
        }
        tempDirPathString.append(tempDirPath);
        return S_OK;
    }

    UINT64 ConvertToQuadVersion(Windows::ApplicationModel::PackageVersion packageVersion)
    {
        // a dot-quad version such as 4.2.5.6 is represented as a 64-bit little endian number as
        //   0006 0005 0002 0004 
        // with the least significant 16-bit word first.
        uint64_t quadVersion = ((uint64_t)packageVersion.Revision << 48) +
            ((uint64_t)packageVersion.Build << 32) +
            ((uint32_t)packageVersion.Minor << 16) +
            ((uint32_t)packageVersion.Major << 0);

        return quadVersion;
    }

    HRESULT GetFileStream(
        _In_ PCWSTR filePath,
        _In_ OPC_STREAM_IO_MODE ioMode,
        _Outptr_ IStream** fileStream)
    {
        winrt::com_ptr<IOpcFactory> opcFactory;
        winrt::check_hresult(CoCreateInstance(__uuidof(OpcFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(opcFactory), opcFactory.put_void()));
        winrt::check_hresult(opcFactory->CreateStreamOnFile(filePath, ioMode, NULL, FILE_ATTRIBUTE_NORMAL, fileStream));
        return S_OK;
    }

    Windows::Foundation::IAsyncAction ExtractFileToDirectory(IAppxFile* appxFile, Windows::Storage::StorageFolder unpackDestinationFolder)
    {
        winrt::com_ptr<IStream> fileStream;
        winrt::check_hresult(appxFile->GetStream(fileStream.put()));

        wil::unique_cotaskmem_string appxFileName;
        winrt::check_hresult(appxFile->GetName(&appxFileName));

        auto outputFile{ unpackDestinationFolder.CreateFileAsync(static_cast<std::wstring>(appxFileName.get()), Windows::Storage::CreationCollisionOption::ReplaceExisting).get() };
        Windows::Storage::Streams::IRandomAccessStream outputFileRandomStream{ co_await outputFile.OpenAsync(Windows::Storage::FileAccessMode::ReadWrite) };
        Windows::Storage::Streams::IOutputStream outputStream{ outputFileRandomStream.GetOutputStreamAt(0) };
        Windows::Storage::Streams::DataWriter dataWriter{ outputStream };

        LARGE_INTEGER start = { 0 };
        ULARGE_INTEGER end = { 0 };
        winrt::check_hresult(fileStream->Seek(start, STREAM_SEEK_END, &end));
        winrt::check_hresult(fileStream->Seek(start, STREAM_SEEK_SET, nullptr));
        std::uint64_t uncompressedSize = static_cast<std::uint64_t>(end.QuadPart);

        std::uint64_t bytesToRead = uncompressedSize;
        while (bytesToRead > 0)
        {
            // Calculate the size of the next block to add
            std::uint32_t blockSize = (bytesToRead > DefaultBlockSize) ? DefaultBlockSize : static_cast<std::uint32_t>(bytesToRead);
            bytesToRead -= blockSize;

            // read block from stream
            std::vector<std::uint8_t> block;
            block.resize(blockSize);
            ULONG bytesRead;
            winrt::check_hresult(fileStream->Read(static_cast<void*>(block.data()), static_cast<ULONG>(blockSize), &bytesRead));
            winrt::check_bool((static_cast<ULONG>(blockSize) == bytesRead));

            dataWriter.WriteBytes(block);
            unsigned int bytesStored{ dataWriter.StoreAsync().get() };
            winrt::check_bool(bytesStored == bytesRead);
        }

        winrt::check_bool(dataWriter.FlushAsync().get());
        dataWriter.Close();
        outputStream.Close();
        outputFileRandomStream.Close();

        co_return;
    }

    Windows::Foundation::IAsyncAction ExtractFilesToDirectory(IAppxFilesEnumerator* appxFilesEnumerator, Windows::Storage::StorageFolder unpackDestinationFolder)
    {
        BOOL hasCurrent = FALSE;
        winrt::check_hresult(appxFilesEnumerator->GetHasCurrent(&hasCurrent));
        while (hasCurrent)
        {
            winrt::com_ptr<IAppxFile> appxFile;
            winrt::check_hresult(appxFilesEnumerator->GetCurrent(appxFile.put()));

            ExtractFileToDirectory(appxFile.get(), unpackDestinationFolder).get();
            
            winrt::check_hresult(appxFilesEnumerator->MoveNext(&hasCurrent));
        }

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Pack(hstring directoryPathToPack, PackOptions packOptions)
    {
        winrt::check_hresult(ValidateCreatePackageArguments(directoryPathToPack, packOptions));

        // Package creation is expected to be slow.
        co_await winrt::resume_background();

        APPX_PACKAGE_SETTINGS settings{};
        winrt::com_ptr<IUri> hashMethodUri;
        const LPCWSTR sha256AlgorithmUri = L"http://www.w3.org/2001/04/xmlenc#sha256";
        winrt::check_hresult(CreateUri(sha256AlgorithmUri, Uri_CREATE_CANONICALIZE, NULL, hashMethodUri.put()));
        settings.forceZip32 = FALSE;
        settings.hashMethod = hashMethodUri.get();

        winrt::com_ptr<IStream> packageStream;
        winrt::check_hresult(GetFileStream(packOptions.PackageFilePath().c_str(), OPC_STREAM_IO_WRITE, packageStream.put()));

        winrt::com_ptr<IAppxFactory> appxFactory;
        winrt::check_hresult(CoCreateInstance(__uuidof(AppxFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxFactory), appxFactory.put_void()));

        winrt::com_ptr<IAppxPackageWriter> packageWriter;
        winrt::check_hresult(appxFactory->CreatePackageWriter(packageStream.get(), &settings, packageWriter.put()));

        for (const auto& file : std::filesystem::directory_iterator(directoryPathToPack.c_str()))
        {
            std::wstring fileName = file.path().filename();
            std::transform(fileName.begin(), fileName.end(), fileName.begin(), towlower);
            if ((fileName == L"appxmanifest.xml") ||
                (fileName == L"appxblockmap.xml") ||
                (fileName == L"appxsignature.p7x"))
            {
                continue;
            }
            std::wstring fileExtension = file.path().extension();
            auto contentType = MSIX::ContentType::GetContentTypeByExtension(fileExtension);

            winrt::com_ptr<IStream> fileStream;
            winrt::check_hresult(GetFileStream(file.path().c_str(), OPC_STREAM_IO_READ, fileStream.put()));

            winrt::check_hresult(packageWriter->AddPayloadFile(file.path().filename().c_str(), contentType.GetContentType().c_str(), contentType.GetCompressionOpt(), fileStream.get()));
        }

        std::filesystem::path manifestFilePath{ directoryPathToPack.c_str()};
        manifestFilePath /= manifestFileName;
        winrt::com_ptr<IStream> manifestStream;
        winrt::check_hresult(GetFileStream(manifestFilePath.c_str(), OPC_STREAM_IO_READ, manifestStream.put()));
        winrt::check_hresult(packageWriter->Close(manifestStream.get()));

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Unpack(hstring packageFilePathToUnpack, UnpackOptions unpackOptions)
    {
        co_await winrt::resume_background();

        Windows::Storage::StorageFolder unpackDestinationFolder{ Windows::Storage::StorageFolder::GetFolderFromPathAsync(unpackOptions.UnpackedPackageRootDirectory()).get() };

        winrt::com_ptr<IStream> packageStream;
        winrt::check_hresult(GetFileStream(packageFilePathToUnpack.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

        winrt::com_ptr<IAppxFactory> appxFactory;
        winrt::check_hresult(CoCreateInstance(__uuidof(AppxFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxFactory), appxFactory.put_void()));

        winrt::com_ptr<IAppxPackageReader> packageReader;
        winrt::check_hresult(appxFactory->CreatePackageReader(packageStream.get(), packageReader.put()));

        winrt::com_ptr<IAppxFilesEnumerator> appxFilesEnumerator;
        winrt::check_hresult(packageReader->GetPayloadFiles(appxFilesEnumerator.put()));

        ExtractFilesToDirectory(appxFilesEnumerator.get(), unpackDestinationFolder).get();

        for (UINT32 footprintType = APPX_FOOTPRINT_FILE_TYPE_MANIFEST; footprintType <= APPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP; footprintType++)
        {
            winrt::com_ptr<IAppxFile> appxFile;
            HRESULT hrGetFile = packageReader->GetFootprintFile(static_cast<APPX_FOOTPRINT_FILE_TYPE>(footprintType), appxFile.put());
            if (hrGetFile != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                winrt::check_hresult(hrGetFile);
                ExtractFileToDirectory(appxFile.get(), unpackDestinationFolder).get();
            }
        }

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Bundle(hstring directoryPathToBundle, BundleOptions bundleOptions)
    {
        co_await winrt::resume_background();

        winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
        winrt::check_hresult(CoCreateInstance(__uuidof(AppxBundleFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxBundleFactory), appxBundleFactory.put_void()));

        winrt::com_ptr<IStream> bundleStream;
        winrt::check_hresult(GetFileStream(bundleOptions.BundleFilePath().c_str(), OPC_STREAM_IO_WRITE, bundleStream.put()));

        winrt::com_ptr<IAppxBundleWriter> bundleWriter;
        winrt::check_hresult(appxBundleFactory->CreateBundleWriter(bundleStream.get(), ConvertToQuadVersion(bundleOptions.BundleVersion()), bundleWriter.put()));

        for (const auto& file : std::filesystem::directory_iterator(directoryPathToBundle.c_str()))
        {
            std::wstring fileExtension = file.path().extension();
            std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), towlower);
            if ((fileExtension != L".appx") &&
                (fileExtension != L".msix"))
            {
                continue;
            }

            winrt::com_ptr<IStream> fileStream;
            winrt::check_hresult(GetFileStream(file.path().c_str(), OPC_STREAM_IO_READ, fileStream.put()));

            winrt::check_hresult(bundleWriter->AddPayloadPackage(file.path().filename().c_str(), fileStream.get()));
        }

        winrt::check_hresult(bundleWriter->Close());
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Unbundle(hstring bundleFilePathToUnbundle, UnbundleOptions unbundleOptions)
    {
        co_await winrt::resume_background();

        Windows::Storage::StorageFolder unpackDestinationFolder{ Windows::Storage::StorageFolder::GetFolderFromPathAsync(unbundleOptions.UnbundledPackageRootDirectory()).get() };

        winrt::com_ptr<IStream> packageStream;
        winrt::check_hresult(GetFileStream(bundleFilePathToUnbundle.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

        winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
        winrt::check_hresult(CoCreateInstance(__uuidof(AppxBundleFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxBundleFactory), appxBundleFactory.put_void()));

        winrt::com_ptr<IAppxBundleReader> bundleReader;
        winrt::check_hresult(appxBundleFactory->CreateBundleReader(packageStream.get(), bundleReader.put()));

        winrt::com_ptr<IAppxFilesEnumerator> appxFilesEnumerator;
        winrt::check_hresult(bundleReader->GetPayloadPackages(appxFilesEnumerator.put()));

        ExtractFilesToDirectory(appxFilesEnumerator.get(), unpackDestinationFolder).get();

        for (UINT32 footprintType = APPX_BUNDLE_FOOTPRINT_FILE_TYPE_FIRST; footprintType <= APPX_BUNDLE_FOOTPRINT_FILE_TYPE_LAST; footprintType++)
        {
            winrt::com_ptr<IAppxFile> appxFile;
            HRESULT hrGetFile = bundleReader->GetFootprintFile(static_cast<APPX_BUNDLE_FOOTPRINT_FILE_TYPE>(footprintType), appxFile.put());

            if (hrGetFile != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                winrt::check_hresult(hrGetFile);
                ExtractFileToDirectory(appxFile.get(), unpackDestinationFolder).get();
            }
        }
    }

    Windows::Foundation::IAsyncAction CreateKozaniPackageFromPackage(hstring packageFilePathToConvert,
            CreateKozaniPackageOptions createKozaniPackageOptions)
    {
        std::wstring tempFullPackageUnpackDirectory{};
        winrt::check_hresult(CreateTempDirectory(tempFullPackageUnpackDirectory));
        UnpackOptions unpackOptions = UnpackOptions();
        unpackOptions.OverwriteFiles(true);
        unpackOptions.UnpackedPackageRootDirectory(tempFullPackageUnpackDirectory);
        co_await MakeMSIXManager::Unpack(packageFilePathToConvert, unpackOptions);

        std::wstring tempKozaniLayoutDirectory{};
        winrt::check_hresult(CreateTempDirectory(tempKozaniLayoutDirectory));
        winrt::check_hresult(CreateKozaniPackageLayout(tempFullPackageUnpackDirectory, tempKozaniLayoutDirectory));

        PackOptions packOptions = PackOptions();
        packOptions.OverwriteFiles(true);
        packOptions.PackageFilePath(createKozaniPackageOptions.PackageFilePath().c_str());

        co_await MakeMSIXManager::Pack(tempKozaniLayoutDirectory.c_str(), packOptions);

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateKozaniPackage(hstring packageFilePathToConvert,
            CreateKozaniPackageOptions createKozaniPackageOptions)
    {
        // Package creation is expected to be slow.
        co_await winrt::resume_background();

        std::filesystem::path packagePath{ packageFilePathToConvert.c_str()};
        std::wstring packagePathExtension{ packagePath.extension() };
        std::transform(packagePathExtension.begin(), packagePathExtension.end(), packagePathExtension.begin(), towlower);
        if (packagePathExtension.ends_with(L".msixbundle") ||
            packagePathExtension.ends_with(L".appxbundle"))
        {
            std::wstring tempUnbundleDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempUnbundleDirectory));

            UnbundleOptions unbundleOptions = UnbundleOptions();
            unbundleOptions.OverwriteFiles(true);
            unbundleOptions.UnbundledPackageRootDirectory(tempUnbundleDirectory);
            Unbundle(packageFilePathToConvert, unbundleOptions).get();

            std::wstring tempKozaniBundlePackageDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempKozaniBundlePackageDirectory));

            for (const auto& file : std::filesystem::directory_iterator(tempUnbundleDirectory))
            {
                std::wstring fileExtension{ file.path().extension() };
                std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), towlower);
                if ((fileExtension.compare(L".appx") != 0) &&
                    (fileExtension.compare(L".msix") != 0))
                {
                    continue;
                }

                std::filesystem::path tempKozaniBundledPackageOutputPath{ tempKozaniBundlePackageDirectory.c_str() };
                tempKozaniBundledPackageOutputPath /= file.path().filename();
                CreateKozaniPackageOptions bundledKozaniPackageOptions = CreateKozaniPackageOptions();
                bundledKozaniPackageOptions.OverwriteFiles(true);
                bundledKozaniPackageOptions.PackageFilePath(tempKozaniBundledPackageOutputPath.c_str());
                CreateKozaniPackageFromPackage(file.path().c_str(), bundledKozaniPackageOptions).get();
            }

            BundleOptions bundleOptions = BundleOptions();
            bundleOptions.OverwriteFiles(true);
            bundleOptions.BundleFilePath(createKozaniPackageOptions.PackageFilePath());
            Bundle(tempKozaniBundlePackageDirectory.c_str(), bundleOptions).get();
        }
        else if (packagePathExtension.ends_with(L".msix") ||
            packagePathExtension.ends_with(L".appx"))
        {
            CreateKozaniPackageFromPackage(packageFilePathToConvert, createKozaniPackageOptions).get();
        }

        co_return;
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateMountableImage(
        winrt::Windows::Foundation::Collections::IVector<hstring> packageFilePathsToAdd,
        CreateMountableImageOptions createMountableImageOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }

    Windows::Foundation::IAsyncOperation<PackageId> MakeMSIXManager::GetPackageIdentity(
        hstring packagePath)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
}
