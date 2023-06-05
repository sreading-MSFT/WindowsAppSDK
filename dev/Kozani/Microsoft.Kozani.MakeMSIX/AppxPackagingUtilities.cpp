// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <filesystem>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.streams.h>
#include <msopc.h>
#include <AppxPackaging.h>
#include <AppModel.h>
#include "PackageIdentity.h"
#include "Constants.hpp"
#include "PackageInformation.h"

using namespace std;
using namespace winrt;

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

std::wstring GetPackageFamilyNameFromNameAndsPublisher(std::wstring name, std::wstring publisher)
{
    PACKAGE_ID mainPackageId = { 0 };
    mainPackageId.name = name.data();
    mainPackageId.publisher = publisher.data();

    WCHAR mainPackageFamilyNameBuffer[PACKAGE_FAMILY_NAME_MAX_LENGTH + 1];
    UINT32 mainPackageFamilyNameBufferLength = ARRAYSIZE(mainPackageFamilyNameBuffer);
    winrt::check_hresult(HRESULT_FROM_WIN32(PackageFamilyNameFromId(&mainPackageId, &mainPackageFamilyNameBufferLength, mainPackageFamilyNameBuffer)));

    std::wstring packageFamilyName(mainPackageFamilyNameBuffer);
    return packageFamilyName;
}


winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity GetPackageIdentityFromManifest(IAppxManifestPackageId* manifestPackageId)
{
    wil::unique_cotaskmem_string packageName;
    winrt::check_hresult(manifestPackageId->GetName(&packageName));

    wil::unique_cotaskmem_string packageFamilyName;
    winrt::check_hresult(manifestPackageId->GetPackageFamilyName(&packageFamilyName));

    wil::unique_cotaskmem_string packageFullName;
    winrt::check_hresult(manifestPackageId->GetPackageFullName(&packageFullName));

    wil::unique_cotaskmem_string publisher;
    winrt::check_hresult(manifestPackageId->GetPublisher(&publisher));

    wil::unique_cotaskmem_string resourceId;
    winrt::check_hresult(manifestPackageId->GetResourceId(&resourceId));

    UINT64 packageManifestVersion;
    winrt::check_hresult(manifestPackageId->GetVersion(&packageManifestVersion));

    APPX_PACKAGE_ARCHITECTURE packageArchitecture;
    winrt::check_hresult(manifestPackageId->GetArchitecture(&packageArchitecture));

    Windows::System::ProcessorArchitecture architecture = Windows::System::ProcessorArchitecture::Unknown;

    Windows::ApplicationModel::PackageVersion packageVersion;
    packageVersion.Major = 1;
    packageVersion.Minor = 1;
    packageVersion.Build = 1;
    packageVersion.Revision = 0;


    auto packageId = winrt::make_self<winrt::Microsoft::Kozani::MakeMSIX::implementation::PackageIdentity>();
    packageId->Version(packageVersion);
    packageId->Name(packageName.get());
    packageId->FamilyName(packageFamilyName.get());
    packageId->FullName(packageFullName.get());
    packageId->Publisher(publisher.get());
    packageId->ResourceId(resourceId.get());
    packageId->Architecture(architecture);
    return *packageId;
}

winrt::Microsoft::Kozani::MakeMSIX::PackageInformation GetPackageInformationFromPackageManifest(IAppxManifestReader* manifestReader)
{
    auto packageInformation = winrt::make_self<winrt::Microsoft::Kozani::MakeMSIX::implementation::PackageInformation>();

    winrt::com_ptr<IAppxManifestPackageId> manifestPackageId;
    winrt::check_hresult(manifestReader->GetPackageId(manifestPackageId.put()));

    packageInformation->SetIdentity(GetPackageIdentityFromManifest(manifestPackageId.get()));
    return *packageInformation;
}

winrt::Microsoft::Kozani::MakeMSIX::PackageInformation GetPackageInformationFromBundleManifest(IAppxBundleManifestReader* manifestReader)
{
    auto packageInformation = winrt::make_self<winrt::Microsoft::Kozani::MakeMSIX::implementation::PackageInformation>();

    winrt::com_ptr<IAppxBundleManifestPackageInfoEnumerator> packageInfoEnumerator;
    winrt::check_hresult(manifestReader->GetPackageInfoItems(packageInfoEnumerator.put()));

    BOOL hasCurrentPackageInfo = FALSE;
    packageInfoEnumerator->GetHasCurrent(&hasCurrentPackageInfo);
    while (hasCurrentPackageInfo)
    {
        winrt::com_ptr<IAppxBundleManifestPackageInfo> manifestPackageInfo;
        winrt::check_hresult(packageInfoEnumerator->GetCurrent(manifestPackageInfo.put()));

        winrt::com_ptr<IAppxManifestQualifiedResourcesEnumerator> resourceEnumerator;
        winrt::check_hresult(manifestPackageInfo->GetResources(resourceEnumerator.put()));

        BOOL hasCurrentResource = FALSE;
        resourceEnumerator->GetHasCurrent(&hasCurrentResource);
        while (hasCurrentResource)
        {
            winrt::com_ptr<IAppxManifestQualifiedResource> resourceInfo;
            winrt::check_hresult(resourceEnumerator->GetCurrent(resourceInfo.put()));

            wil::unique_cotaskmem_string language;
            winrt::check_hresult(resourceInfo->GetLanguage(&language));
            packageInformation->AddLanguage(language.get());

            unsigned int scale;
            winrt::check_hresult(resourceInfo->GetScale(&scale));
            packageInformation->AddScale(scale);

            winrt::check_hresult(resourceEnumerator->MoveNext(&hasCurrentResource));
        }

        winrt::check_hresult(packageInfoEnumerator->MoveNext(&hasCurrentPackageInfo));
    }

    winrt::com_ptr<IAppxManifestPackageId> manifestPackageId;
    winrt::check_hresult(manifestReader->GetPackageId(manifestPackageId.put()));

    packageInformation->SetIdentity(GetPackageIdentityFromManifest(manifestPackageId.get()));
    return *packageInformation;
}

Windows::Foundation::IAsyncAction ExtractAppxFileToDirectory(IAppxFile* appxFile, Windows::Storage::StorageFolder unpackDestinationFolder)
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

Windows::Foundation::IAsyncAction ExtractAppxFilesToDirectory(IAppxFilesEnumerator* appxFilesEnumerator, Windows::Storage::StorageFolder unpackDestinationFolder)
{
    BOOL hasCurrent = FALSE;
    winrt::check_hresult(appxFilesEnumerator->GetHasCurrent(&hasCurrent));
    while (hasCurrent)
    {
        winrt::com_ptr<IAppxFile> appxFile;
        winrt::check_hresult(appxFilesEnumerator->GetCurrent(appxFile.put()));

        ExtractAppxFileToDirectory(appxFile.get(), unpackDestinationFolder).get();

        winrt::check_hresult(appxFilesEnumerator->MoveNext(&hasCurrent));
    }

    co_return;
}

HRESULT CreateAppxFactory(IAppxFactory** appxFactory)
{
    winrt::com_ptr<IAppxFactory> appxFactoryLocal;
    HRESULT hr = CoCreateInstance(__uuidof(AppxFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxFactoryLocal), appxFactoryLocal.put_void());
    *appxFactory = appxFactoryLocal.detach();
    return hr;
}

HRESULT CreateAppxBundleFactory(IAppxBundleFactory** appxBundleFactory)
{
    winrt::com_ptr<IAppxBundleFactory> appxBundleFactoryLocal;
    HRESULT hr = CoCreateInstance(__uuidof(AppxBundleFactory), NULL, CLSCTX_INPROC_SERVER, __uuidof(appxBundleFactoryLocal), appxBundleFactoryLocal.put_void());
    *appxBundleFactory = appxBundleFactoryLocal.detach();
    return hr;
}
