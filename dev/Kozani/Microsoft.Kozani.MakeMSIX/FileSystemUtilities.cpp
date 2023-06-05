// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <filesystem>
#include <msopc.h>
#include "FileSystemUtilities.hpp"

using namespace std;


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

PackageType GetPackageTypeFromPath(std::filesystem::path packagePath)
{
    if (std::filesystem::is_directory(packagePath))
    {
        if (std::filesystem::is_regular_file(GetManifestFilePath(packagePath, PackageType::LayoutBundleDirectory)))
        {
            return PackageType::LayoutBundleDirectory;
        }
        else if(std::filesystem::is_regular_file(GetManifestFilePath(packagePath, PackageType::LayoutPackageDirectory)))
        {
            return PackageType::LayoutPackageDirectory;
        }
        return PackageType::Unrecognized;
    }
    std::wstring packagePathExtension{ packagePath.extension() };
    std::transform(packagePathExtension.begin(), packagePathExtension.end(), packagePathExtension.begin(), towlower);
    if (packagePathExtension.ends_with(L".msixbundle") ||
        packagePathExtension.ends_with(L".appxbundle"))
    {
        return PackageType::Bundle;
    }
    else if (packagePathExtension.ends_with(L".msix") ||
                packagePathExtension.ends_with(L".appx"))
    {
        return PackageType::Package;
    }
    return PackageType::Unrecognized;
}

std::filesystem::path GetManifestFilePath(std::filesystem::path path, PackageType packageType)
{
    std::filesystem::path manifestPath{ path };
    switch (packageType)
    {
    case PackageType::LayoutBundleDirectory:
    case PackageType::Bundle:
        manifestPath /= L"AppxMetadata\\BundleManifest.xml";
        return manifestPath;
    case PackageType::LayoutPackageDirectory:
    case PackageType::Package:
        manifestPath /= L"AppxManifest.xml";
        return manifestPath;
    case PackageType::Unrecognized:
    default:
        winrt::throw_hresult(E_INVALIDARG);
    }
}

bool IsFootprintFile(std::filesystem::directory_entry entry)
{
    std::wstring fileName = entry.path().filename();
    std::transform(fileName.begin(), fileName.end(), fileName.begin(), towlower);
    if ((fileName == L"appxmanifest.xml") ||
        (fileName == L"appxblockmap.xml") ||
        (fileName == L"appxsignature.p7x"))
    {
        return true;
    }
    return false;
}

bool IsFileWithPackageExtension(std::filesystem::directory_entry entry)
{
    std::wstring fileExtension = entry.path().extension();
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), towlower);
    if ((fileExtension == L".appx") ||
        (fileExtension == L".msix"))
    {
        return true;
    }
    return false;
}
