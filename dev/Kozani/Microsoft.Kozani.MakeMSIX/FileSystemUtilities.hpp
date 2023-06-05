// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "pch.h"

HRESULT CreateTempDirectory(_Out_ std::wstring& tempDirPathString);

HRESULT GetFileStream(
    _In_ PCWSTR filePath,
    _In_ OPC_STREAM_IO_MODE ioMode,
    _Outptr_ IStream** fileStream);

enum PackageType
{
    Unrecognized,
    LayoutBundleDirectory,
    LayoutPackageDirectory,
    Bundle,
    Package
};

PackageType GetPackageTypeFromPath(std::filesystem::path packagePath);

std::filesystem::path GetManifestFilePath(std::filesystem::path path, PackageType packageType);

bool IsFootprintFile(std::filesystem::directory_entry entry);

bool IsFileWithPackageExtension(std::filesystem::directory_entry entry);

