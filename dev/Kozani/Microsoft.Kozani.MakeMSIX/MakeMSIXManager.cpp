// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "PackageIdentity.h"
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
#include "AppxPackagingUtilities.hpp"
#include "FileSystemUtilities.hpp"
#include "PackageInformation.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    Windows::Foundation::IAsyncAction MakeMSIXManager::CreatePackage(hstring inputPath, hstring outputFileName, CreatePackageOptions createPackageOptions)
    {
        if (!std::filesystem::is_directory(inputPath.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!createPackageOptions.OverwriteOutputFileIfExists() &&
            std::filesystem::exists(outputFileName.c_str()))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_EXISTS));
        }

        std::filesystem::path manifestFilePath{ inputPath.c_str() };
        manifestFilePath /= manifestFileName;
        if (!std::filesystem::is_regular_file(manifestFileName))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_NOT_FOUND));
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        // Get manifest file stream from layout directory.
        winrt::com_ptr<IStream> manifestStream;
        winrt::check_hresult(GetFileStream(manifestFilePath.c_str(), OPC_STREAM_IO_READ, manifestStream.put()));

        // Open a write stream to the output package file.
        winrt::com_ptr<IStream> outputPackageStream;
        winrt::check_hresult(GetFileStream(outputFileName.c_str(), OPC_STREAM_IO_WRITE, outputPackageStream.put()));

        winrt::com_ptr<IAppxFactory> appxFactory;
        winrt::check_hresult(CreateAppxFactory(appxFactory.put()));

        // Use default settings for package. CreatePackageOptions does not have any option for customizing these.
        APPX_PACKAGE_SETTINGS settings{};
        winrt::com_ptr<IUri> hashMethodUri;
        const LPCWSTR sha256AlgorithmUri = L"http://www.w3.org/2001/04/xmlenc#sha256";
        winrt::check_hresult(CreateUri(sha256AlgorithmUri, Uri_CREATE_CANONICALIZE, NULL, hashMethodUri.put()));
        settings.forceZip32 = FALSE;
        settings.hashMethod = hashMethodUri.get();

        winrt::com_ptr<IAppxPackageWriter> packageWriter;
        winrt::check_hresult(appxFactory->CreatePackageWriter(outputPackageStream.get(), &settings, packageWriter.put()));

        for (const auto& file : std::filesystem::directory_iterator(inputPath.c_str()))
        {
            // Add each file to the package. Skip any footprint files.
            if (IsFootprintFile(file))
            {
                continue;
            }

            // Lookup the settings for writing the file to the archives based on file extensions.
            std::wstring fileExtension = file.path().extension().wstring();
            auto contentType = MSIX::ContentType::GetContentTypeByExtension(fileExtension);

            winrt::com_ptr<IStream> fileStream;
            winrt::check_hresult(GetFileStream(file.path().c_str(), OPC_STREAM_IO_READ, fileStream.put()));

            // Write the file from the layout directory to the package.
            winrt::check_hresult(packageWriter->AddPayloadFile(file.path().filename().c_str(), contentType.GetContentType().c_str(), contentType.GetCompressionOpt(), fileStream.get()));
        }

        //TODO: Write changes to the manifest as specified by CreatePackageOptions. Code can be shared with CreateKozaniPackage

        winrt::check_hresult(packageWriter->Close(manifestStream.get()));

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::ExtractPackage(hstring inputFileName, hstring outputPath, ExtractPackageOptions extractPackageOptions)
    {
        if (!std::filesystem::is_regular_file(inputFileName.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!extractPackageOptions.OverwriteOutputFilesIfExists() &&
            std::filesystem::exists(outputPath.c_str()))
        {
            winrt::throw_hresult(ERROR_FILE_EXISTS);
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        winrt::com_ptr<IStream> packageStream;
        winrt::check_hresult(GetFileStream(inputFileName.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

        winrt::com_ptr<IAppxFactory> appxFactory;
        winrt::check_hresult(CreateAppxFactory(appxFactory.put()));

        winrt::com_ptr<IAppxPackageReader> packageReader;
        winrt::check_hresult(appxFactory->CreatePackageReader(packageStream.get(), packageReader.put()));

        winrt::com_ptr<IAppxFilesEnumerator> appxFilesEnumerator;
        winrt::check_hresult(packageReader->GetPayloadFiles(appxFilesEnumerator.put()));

        // TODO: Check if this works with folders where intermediate paths don't exist. Plan to create intermediate folders if necessary.
        Windows::Storage::StorageFolder unpackDestinationFolder{ Windows::Storage::StorageFolder::GetFolderFromPathAsync(outputPath).get() };
        ExtractAppxFilesToDirectory(appxFilesEnumerator.get(), unpackDestinationFolder).get();

        // Extract all footprint files from package.
        for (UINT32 footprintType = APPX_FOOTPRINT_FILE_TYPE_MANIFEST; footprintType <= APPX_FOOTPRINT_FILE_TYPE_CONTENTGROUPMAP; footprintType++)
        {
            winrt::com_ptr<IAppxFile> appxFile;
            HRESULT hrGetFile = packageReader->GetFootprintFile(static_cast<APPX_FOOTPRINT_FILE_TYPE>(footprintType), appxFile.put());
            if ((footprintType == APPX_FOOTPRINT_FILE_TYPE_MANIFEST) ||
                (hrGetFile != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
            {
                // Some footprint files won't exist in every package.
                // The manifest is required to exist.
                winrt::check_hresult(hrGetFile);
                ExtractAppxFileToDirectory(appxFile.get(), unpackDestinationFolder).get();
            }
        }

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateBundle(hstring inputPath, hstring outputFileName, CreateBundleOptions createBundleOptions)
    {
        if (!std::filesystem::is_directory(inputPath.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!createBundleOptions.OverwriteOutputFileIfExists() &&
            std::filesystem::exists(outputFileName.c_str()))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_EXISTS));
        }

        std::filesystem::path manifestFilePath = GetManifestFilePath(inputPath.c_str(), PackageType::Bundle);
        if (!std::filesystem::is_regular_file(manifestFilePath))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_NOT_FOUND));
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
        winrt::check_hresult(CreateAppxBundleFactory(appxBundleFactory.put()));

        winrt::com_ptr<IStream> bundleStream;
        winrt::check_hresult(GetFileStream(outputFileName.c_str(), OPC_STREAM_IO_WRITE, bundleStream.put()));

        winrt::com_ptr<IAppxBundleWriter> bundleWriter;
        winrt::check_hresult(appxBundleFactory->CreateBundleWriter(bundleStream.get(), ConvertToQuadVersion(createBundleOptions.Version()), bundleWriter.put()));

        for (const auto& file : std::filesystem::directory_iterator(inputPath.c_str()))
        {
            // Add each package to the bundle. Skip any footprint files or other miscellaneous files.
            if (!IsFileWithPackageExtension(file))
            {
                continue;
            }

            winrt::com_ptr<IStream> fileStream;
            winrt::check_hresult(GetFileStream(file.path().c_str(), OPC_STREAM_IO_READ, fileStream.put()));

            winrt::check_hresult(bundleWriter->AddPayloadPackage(file.path().filename().c_str(), fileStream.get()));
        }

        winrt::check_hresult(bundleWriter->Close());

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::ExtractBundle(hstring inputFileName, hstring outputPath, ExtractBundleOptions extractBundleOptions)
    {
        if (!std::filesystem::is_regular_file(inputFileName.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!extractBundleOptions.OverwriteOutputFilesIfExists() &&
            std::filesystem::exists(outputPath.c_str()))
        {
            winrt::throw_hresult(ERROR_FILE_EXISTS);
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        Windows::Storage::StorageFolder unpackDestinationFolder{ Windows::Storage::StorageFolder::GetFolderFromPathAsync(outputPath).get() };

        winrt::com_ptr<IStream> packageStream;
        winrt::check_hresult(GetFileStream(inputFileName.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

        winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
        winrt::check_hresult(CreateAppxBundleFactory(appxBundleFactory.put()));

        winrt::com_ptr<IAppxBundleReader> bundleReader;
        winrt::check_hresult(appxBundleFactory->CreateBundleReader(packageStream.get(), bundleReader.put()));

        winrt::com_ptr<IAppxFilesEnumerator> appxFilesEnumerator;
        winrt::check_hresult(bundleReader->GetPayloadPackages(appxFilesEnumerator.put()));

        ExtractAppxFilesToDirectory(appxFilesEnumerator.get(), unpackDestinationFolder).get();

        // Extract all footprint files from bundle.
        for (UINT32 footprintType = APPX_BUNDLE_FOOTPRINT_FILE_TYPE_FIRST; footprintType <= APPX_BUNDLE_FOOTPRINT_FILE_TYPE_LAST; footprintType++)
        {
            winrt::com_ptr<IAppxFile> appxFile;
            HRESULT hrGetFile = bundleReader->GetFootprintFile(static_cast<APPX_BUNDLE_FOOTPRINT_FILE_TYPE>(footprintType), appxFile.put());
            if ((footprintType == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST) ||
                (hrGetFile != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
            {
                // Some footprint files won't exist in every package.
                // The manifest is required to exist.
                winrt::check_hresult(hrGetFile);
                ExtractAppxFileToDirectory(appxFile.get(), unpackDestinationFolder).get();
            }
        }

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateKozaniPackage(
        hstring inputFileName,
        hstring outputFileName,
        CreateKozaniPackageOptions createKozaniPackageOptions)
    {
        if (!std::filesystem::is_directory(inputFileName.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!createKozaniPackageOptions.OverwriteOutputFileIfExists() &&
            std::filesystem::exists(outputFileName.c_str()))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_EXISTS));
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        PackageType packageType = GetPackageTypeFromPath(inputFileName.c_str());
        if (packageType == PackageType::LayoutPackageDirectory)
        {
            winrt::check_hresult(CreateKozaniPackageLayoutFromPackageLayout(inputFileName.c_str(), outputFileName.c_str()));
        }
        else if (packageType == PackageType::Package)
        {
            std::wstring tempFullPackageUnpackDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempFullPackageUnpackDirectory));
            ExtractPackageOptions extractPackageOptions = ExtractPackageOptions();
            extractPackageOptions.OverwriteOutputFilesIfExists(true);
            ExtractPackage(inputFileName, tempFullPackageUnpackDirectory.c_str(), extractPackageOptions).get();

            std::wstring tempKozaniLayoutDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempKozaniLayoutDirectory));
            winrt::check_hresult(CreateKozaniPackageLayoutFromPackageLayout(tempFullPackageUnpackDirectory, tempKozaniLayoutDirectory));

            CreatePackageOptions createPackageOptions = CreatePackageOptions();
            createPackageOptions.OverwriteOutputFileIfExists(true);
            CreatePackage(tempKozaniLayoutDirectory.c_str(), outputFileName, createPackageOptions).get();
        }
        else if (packageType == PackageType::Bundle)
        {
            winrt::Microsoft::Kozani::MakeMSIX::PackageInformation bundlePackageInfo = co_await GetPackageInformation(inputFileName);
            winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity bundlePackageId = bundlePackageInfo.Identity();
            Windows::ApplicationModel::PackageVersion originalBundleVersion = bundlePackageId.Version();

            // Create a directory for the unbundled layout.
            // Files will be <tempUnbundleDirectory>\AppxMetadata\AppxBundleManifest.xml
            // Files will be <tempUnbundleDirectory>\MainPackage.msix
            std::wstring tempUnbundleDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempUnbundleDirectory));

            ExtractBundleOptions extractBundleOptions = ExtractBundleOptions();
            extractBundleOptions.OverwriteOutputFilesIfExists(true);
            ExtractBundle(inputFileName, tempUnbundleDirectory.c_str(), extractBundleOptions).get();

            // Create a directory for the kozani packages for rebundling.
            // Files will be <tempKozaniBundlePackageDirectory>\MainPackage.msix
            std::wstring tempKozaniBundlePackageDirectory{};
            winrt::check_hresult(CreateTempDirectory(tempKozaniBundlePackageDirectory));
            // TODO: need to iterate through packages
            CreateKozaniPackageOptions layoutBundlePackageOptions = CreateKozaniPackageOptions();
            layoutBundlePackageOptions.OverwriteOutputFileIfExists(true);
            CreateKozaniPackage(tempKozaniBundlePackageDirectory.c_str(), tempKozaniBundlePackageDirectory.c_str(), layoutBundlePackageOptions).get();

            // Create the bundle again from Kozani packages at the location specified by createKozaniPackageOptions.
            CreateBundleOptions createBundleOptions = CreateBundleOptions();
            createBundleOptions.FlatBundle(false);
            createBundleOptions.Version(originalBundleVersion);
            createBundleOptions.OverwriteOutputFileIfExists(true);
            CreateBundle(tempKozaniBundlePackageDirectory.c_str(), outputFileName, createBundleOptions).get();
        }
        else if (packageType == PackageType::LayoutBundleDirectory)
        {
            for (const auto& file : std::filesystem::directory_iterator(inputFileName.c_str()))
            {
                // Skip any file that's not a package. Bundle footprint files are all automatically generated
                PackageType filePackageType = GetPackageTypeFromPath(file.path());
                if (filePackageType != PackageType::Package)
                {
                    continue;
                }

                // Build the temp output path for the package.
                // <layout directory>\MainPackage.msix becomes <tempKozaniBundlePackageDirectory>\MainPackage.msix
                std::filesystem::path tempKozaniBundledPackageOutputPath{ outputFileName.c_str()};
                tempKozaniBundledPackageOutputPath /= file.path().filename();

                CreateKozaniPackageOptions bundledKozaniPackageOptions = CreateKozaniPackageOptions();
                bundledKozaniPackageOptions.OverwriteOutputFileIfExists(true);
                CreateKozaniPackage(file.path().c_str(), tempKozaniBundledPackageOutputPath.c_str(), bundledKozaniPackageOptions).get();
            }
        }
        else
        {
            // Package type not recognized
            winrt::throw_hresult(E_INVALIDARG);
        }

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateMountableImage(
        winrt::Windows::Foundation::Collections::IVector<hstring> inputFileNames,
        hstring outputFileName,
        CreateMountableImageOptions createMountableImageOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }

    Windows::Foundation::IAsyncOperation<winrt::Microsoft::Kozani::MakeMSIX::PackageInformation> MakeMSIXManager::GetPackageInformation(
        hstring packageFileName)
    {
        if (std::filesystem::is_directory(packageFileName.c_str()))
        {
            winrt::throw_hresult(E_INVALIDARG);
        }
        if (!std::filesystem::exists(packageFileName.c_str()))
        {
            winrt::throw_hresult(HRESULT(ERROR_FILE_NOT_FOUND));
        }

        // Package operations are expected to be slow.
        // Calls to the underlying AppxPackaging COM API are synchronous.
        co_await winrt::resume_background();

        PackageType packageType = GetPackageTypeFromPath(packageFileName.c_str());
        switch (packageType)
        {
        case PackageType::Bundle:
        {
            winrt::com_ptr<IStream> packageStream;
            winrt::check_hresult(GetFileStream(packageFileName.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

            winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
            winrt::check_hresult(CreateAppxBundleFactory(appxBundleFactory.put()));

            winrt::com_ptr<IAppxBundleReader> bundleReader;
            winrt::check_hresult(appxBundleFactory->CreateBundleReader(packageStream.get(), bundleReader.put()));

            winrt::com_ptr<IAppxBundleManifestReader> manifestReader;
            winrt::check_hresult(bundleReader->GetManifest(manifestReader.put()));

            co_return GetPackageInformationFromBundleManifest(manifestReader.get());
        }
        case PackageType::Package:
        {
            winrt::com_ptr<IStream> packageStream;
            winrt::check_hresult(GetFileStream(packageFileName.c_str(), OPC_STREAM_IO_READ, packageStream.put()));

            winrt::com_ptr<IAppxFactory> appxFactory;
            winrt::check_hresult(CreateAppxFactory(appxFactory.put()));

            winrt::com_ptr<IAppxPackageReader> packageReader;
            winrt::check_hresult(appxFactory->CreatePackageReader(packageStream.get(), packageReader.put()));

            winrt::com_ptr<IAppxManifestReader> manifestReader;
            winrt::check_hresult(packageReader->GetManifest(manifestReader.put()));

            co_return GetPackageInformationFromPackageManifest(manifestReader.get());
        }
        case PackageType::LayoutBundleDirectory:
        {
            std::filesystem::path manifestPath = GetManifestFilePath(packageFileName.c_str(), PackageType::LayoutBundleDirectory);

            winrt::com_ptr<IStream> manifestStream;
            winrt::check_hresult(GetFileStream(manifestPath.c_str(), OPC_STREAM_IO_READ, manifestStream.put()));

            winrt::com_ptr<IAppxBundleFactory> appxBundleFactory;
            winrt::check_hresult(CreateAppxBundleFactory(appxBundleFactory.put()));

            winrt::com_ptr<IAppxBundleManifestReader> manifestReader;
            winrt::check_hresult(appxBundleFactory->CreateBundleManifestReader(manifestStream.get(), manifestReader.put()));

            co_return GetPackageInformationFromBundleManifest(manifestReader.get());
        }
        case PackageType::LayoutPackageDirectory:
        {
            std::filesystem::path manifestPath = GetManifestFilePath(packageFileName.c_str(), PackageType::LayoutPackageDirectory);

            winrt::com_ptr<IStream> manifestStream;
            winrt::check_hresult(GetFileStream(manifestPath.c_str(), OPC_STREAM_IO_READ, manifestStream.put()));

            winrt::com_ptr<IAppxFactory> appxFactory;
            winrt::check_hresult(CreateAppxFactory(appxFactory.put()));

            winrt::com_ptr<IAppxManifestReader> manifestReader;
            winrt::check_hresult(appxFactory->CreateManifestReader(manifestStream.get(), manifestReader.put()));

            co_return GetPackageInformationFromPackageManifest(manifestReader.get());
        }
        default:
            // Package type not recognized
            winrt::throw_hresult(E_INVALIDARG);
        }
    }
}
