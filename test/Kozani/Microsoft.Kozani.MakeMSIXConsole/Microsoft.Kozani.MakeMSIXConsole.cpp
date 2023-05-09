// Microsoft.Kozani.MakeMSIX.exe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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
            createKozaniPackageOptions.OverwriteFiles(true);

            std::wstring fileNameWithoutExtension = directoryEntry.path().stem().wstring();
            std::wstring packageOutputFileName{ fileNameWithoutExtension + L"Kozani" + directoryEntry.path().extension().c_str() };
            std::filesystem::path packageOutputPath{ kozaniPath };
            packageOutputPath /= packageOutputFileName;

            createKozaniPackageOptions.PackageFilePath(packageOutputPath.c_str());

            MakeMSIXManager::CreateKozaniPackage(directoryEntry.path().c_str(), createKozaniPackageOptions).get();
        }
    }
    else
    {
        CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
        createKozaniPackageOptions.OverwriteFiles(true);
        if (std::filesystem::is_directory(kozaniPath))
        {
            std::wstring fileNameWithoutExtension = convertPath.stem().wstring();
            std::wstring packageOutputFileName{ fileNameWithoutExtension + L"Kozani" + convertPath.extension().c_str() };
            kozaniPath /= packageOutputFileName;
        }
        createKozaniPackageOptions.PackageFilePath(kozaniPath.c_str());

        MakeMSIXManager::CreateKozaniPackage(convertPath.c_str(), createKozaniPackageOptions).get();
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
