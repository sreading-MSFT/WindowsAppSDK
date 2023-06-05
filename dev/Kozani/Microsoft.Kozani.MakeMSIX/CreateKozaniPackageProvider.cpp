// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "Constants.hpp"
#include "CreateKozaniPackageProvider.hpp"
#include <set>
#include <filesystem>
#include <winrt/windows.data.xml.dom.h>
#include <winrt/windows.storage.h>
#include <MrmResourceIndexer.h>
#include "ResourcePriConverter.hpp"
#include "ManifestConverter.hpp"

using namespace std;

HRESULT CopyRequiredFiles(std::wstring originalDirectory, std::wstring outputDirectory, std::set<std::wstring>& requiredFilesRelativePaths)
{
    for (std::wstring relativeFilePath : requiredFilesRelativePaths)
    {
        std::filesystem::path originalFilePath{ originalDirectory };
        originalFilePath /= relativeFilePath;
        std::filesystem::path newFilePath{ outputDirectory };
        newFilePath /= relativeFilePath;

        std::filesystem::path dirPath = newFilePath.parent_path();
        std::filesystem::create_directories(dirPath);
        std::error_code copyError{};
        std::filesystem::copy_options copyOptions = std::filesystem::copy_options::overwrite_existing;
        std::filesystem::copy_file(originalFilePath, newFilePath, copyOptions, copyError);
        if (copyError.value() != 0)
        {
            std::error_code existsCheckErrorCode{};
            bool fileExists = std::filesystem::exists(originalFilePath, existsCheckErrorCode);
            if (existsCheckErrorCode.value() != 0 || fileExists)
            {
                return E_UNEXPECTED;
            }
        }
    }

    return S_OK;
}

HRESULT CreateKozaniPackageLayoutFromPackageLayout(
    _In_ std::wstring sourceDirectoryPath,
    _In_ std::wstring outputDirectoryPath)
{
    if (!filesystem::is_directory(sourceDirectoryPath) ||
        !filesystem::is_directory(outputDirectoryPath) ||
        (sourceDirectoryPath.compare(outputDirectoryPath) == 0))
    {
        return E_INVALIDARG;
    }
        
    std::filesystem::path manifestFilePath{sourceDirectoryPath};
    manifestFilePath /= manifestFileName;
    std::filesystem::path resourcesPriFilePath{ sourceDirectoryPath };
    resourcesPriFilePath /= resourcesPriFileName;

    std::set<std::wstring> requiredFilesRelativePaths;
    RETURN_IF_FAILED(CreateKozaniResourcePriFromManifestAndResourcePri(manifestFilePath, resourcesPriFilePath, outputDirectoryPath, requiredFilesRelativePaths));
    
    RETURN_IF_FAILED(CopyRequiredFiles(sourceDirectoryPath, outputDirectoryPath, requiredFilesRelativePaths));

    RETURN_IF_FAILED(ModifyManifest(manifestFilePath, outputDirectoryPath));

    return S_OK;
}
