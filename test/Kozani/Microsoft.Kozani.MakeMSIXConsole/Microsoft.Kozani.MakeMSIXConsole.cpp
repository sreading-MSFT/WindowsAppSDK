// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <winrt/base.h>
#include <iostream>
#include <filesystem>
#include <winrt/Microsoft.Kozani.MakeMSIX.h>

using namespace winrt::Microsoft::Kozani::MakeMSIX;

int main()
{
    if (__argc < 3)
    {
        wprintf(L"Arguments must be <input file or directory path> <output file or directory path>\n");
        return -1;
    }

    std::filesystem::path convertPath{ __argv[1] };
    std::filesystem::path kozaniPath{ __argv[2]};

    if (!std::filesystem::exists(convertPath))
    {
        wprintf(L"Path to convert must exist\n");
        return -1;
    }
    if (std::filesystem::is_directory(convertPath))
    {
        if (!std::filesystem::is_directory(kozaniPath))
        {
            wprintf(L"Output must be directory if input is directory\n");
            return -1;
        }

        for (const std::filesystem::directory_entry& directoryEntry : std::filesystem::directory_iterator(convertPath))
        {
            if (directoryEntry.is_directory())
            {
                continue;
            }

            CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
            createKozaniPackageOptions.OverwriteOutputFileIfExists(true);

            std::wstring fileNameWithoutExtension = directoryEntry.path().stem().wstring();
            std::wstring packageOutputFileName{ fileNameWithoutExtension + L"Kozani" + directoryEntry.path().extension().c_str() };
            std::filesystem::path packageOutputPath{ kozaniPath };
            packageOutputPath /= packageOutputFileName;

            MakeMSIXManager::CreateKozaniPackage(directoryEntry.path().c_str(), packageOutputPath.c_str(), createKozaniPackageOptions).get();
        }
    }
    else
    {
        CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
        createKozaniPackageOptions.OverwriteOutputFileIfExists(true);
        if (std::filesystem::is_directory(kozaniPath))
        {
            std::wstring fileNameWithoutExtension = convertPath.stem().wstring();
            std::wstring packageOutputFileName{ fileNameWithoutExtension + L"Kozani" + convertPath.extension().c_str() };
            kozaniPath /= packageOutputFileName;
        }

        MakeMSIXManager::CreateKozaniPackage(convertPath.c_str(), kozaniPath.c_str(), createKozaniPackageOptions).get();
    }
    return 0;
}
