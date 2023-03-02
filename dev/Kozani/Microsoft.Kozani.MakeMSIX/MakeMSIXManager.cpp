// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MakeMSIXManager.h"
#include "MakeMSIXManager.g.cpp"
#include "CreateKozaniPackageProvider.hpp"
#include <filesystem>

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    /// <summary>
    /// Create a process and synchronously wait for it to exit.
    /// </summary>
    /// <param name="commandLine">CommandLine argument for CreateProcess</param>
    /// <param name="exitCode">Exit code of the process</param>
    /// <returns>Success if process is created and exit code is returned.</returns>
    HRESULT CreateProcessAndWaitForExitCode(std::wstring commandLine, DWORD& exitCode)
    {
        STARTUPINFO startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION processInformation;
        ZeroMemory(&processInformation, sizeof(processInformation));

        if (!CreateProcess(
            nullptr,
            const_cast<LPWSTR>(commandLine.c_str()),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &startupInfo,
            &processInformation
        ))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        DWORD waitResult = WaitForSingleObject(processInformation.hProcess, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0:
            break;
        case WAIT_FAILED:
            return HRESULT_FROM_WIN32(GetLastError());
        case WAIT_ABANDONED:
        case WAIT_TIMEOUT:
        default:
            return E_UNEXPECTED;
        }
        if (!GetExitCodeProcess(processInformation.hProcess, &exitCode))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        return S_OK;
    }

    HRESULT LaunchMakeAppxWithArguments(std::list<std::wstring>& arguments)
    {
        wil::unique_hkey hKey;
        std::wstring sdkRegPath{ LR"(SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)" };
        RETURN_IF_FAILED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sdkRegPath.c_str(), 0, KEY_READ, &hKey));

        std::wstring installPathKeyName{ L"InstallationFolder"};
        WCHAR installPathBuffer[MAX_PATH];
        DWORD installPathBufferLength = sizeof(installPathBuffer);
        RETURN_IF_FAILED(RegGetValueW(
            hKey.get(),
            nullptr,
            installPathKeyName.c_str(),
            RRF_RT_REG_SZ,
            nullptr /* pdwType */,
            &installPathBuffer,
            &installPathBufferLength));
        std::wstring installPathString{ installPathBuffer };

        std::wstring productVersionKeyName{ L"ProductVersion" };
        WCHAR productVersionBuffer[MAX_PATH];
        DWORD productVersionBufferLength = sizeof(productVersionBuffer);
        RETURN_IF_FAILED(RegGetValueW(
            hKey.get(),
            nullptr,
            productVersionKeyName.c_str(),
            RRF_RT_REG_SZ,
            nullptr /* pdwType */,
            &productVersionBuffer,
            &productVersionBufferLength));
        std::wstring productVersionString{ productVersionBuffer };
            
        std::filesystem::path makeAppxPath{ installPathString };
        makeAppxPath /= L"bin";
        makeAppxPath /= productVersionString + L".0"; //TODO: Only add last digit to version string when necessary
        makeAppxPath /= L"x64"; //TODO: lookup architecture
        makeAppxPath /= L"makeappx.exe";
        std::wstring commandLine{ makeAppxPath };
        for (std::wstring argument : arguments)
        {
            commandLine += L" " + argument;
        }
        DWORD exitCode{};
        RETURN_IF_FAILED(CreateProcessAndWaitForExitCode(commandLine, exitCode));
        if (exitCode == EXIT_FAILURE)
        {
            return E_FAIL;
        }
        return S_OK;
    }

    std::list<std::wstring> GetCommandLineArgumentsForMakeAppxPack(std::wstring directoryToPack, std::wstring packageOutputPath, PackOptions packOptions)
    {
        std::list<std::wstring> arguments;
        arguments.push_back(L"pack");
        arguments.push_back(L"/d");
        arguments.push_back(directoryToPack);
        arguments.push_back(L"/p");
        arguments.push_back(packageOutputPath);
        if (packOptions.OverwriteFiles())
        {
            arguments.push_back(L"/o");
        }
        else
        {
            arguments.push_back(L"/no");
        }
        if (!packOptions.ValidateFiles())
        {
            arguments.push_back(L"/nv");
            arguments.push_back(L"/nfv");
        }
        /*if (!packOptions.EncryptionKeyFilePath().empty())
        {
            arguments.push_back(L"/kf");
            arguments.push_back(packOptions.EncryptionKeyFilePath().c_str());
        }*/
        return arguments;
    }

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

    HRESULT RunCreatePackageOperation(hstring directoryPathToPack, PackOptions packOptions)
    {
        if (!packOptions.PackageFilePath().empty())
        {
            std::list<std::wstring > arguments = GetCommandLineArgumentsForMakeAppxPack(directoryPathToPack.c_str(), packOptions.PackageFilePath().c_str(), packOptions);
            RETURN_IF_FAILED(LaunchMakeAppxWithArguments(arguments));
        }
        /*if (!packOptions.KozaniPackageFilePath().empty())
        {
            std::wstring tempDirectory{};
            CreateTempDirectory(tempDirectory);
            RETURN_IF_FAILED(CreateKozaniPackageLayout(directoryPathToPack.c_str(), tempDirectory));

            // TODO: SkipValidation currently must be set to false due to known missing required non-resource files (exes and dlls).
            packOptions.ValidateFiles(false);
            std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxPack(tempDirectory, packOptions.KozaniPackageFilePath().c_str(), packOptions);
            RETURN_IF_FAILED(LaunchMakeAppxWithArguments(arguments));
        }*/
        return S_OK;;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Pack(hstring directoryPathToPack, PackOptions packOptions)
    {
        HRESULT hrValidate = ValidateCreatePackageArguments(directoryPathToPack, packOptions);
        if (FAILED(hrValidate))
        {
            winrt::throw_hresult(hrValidate);
        }

        // Package creation is expected to be slow.
        co_await winrt::resume_background();

        HRESULT hrCreate = RunCreatePackageOperation(directoryPathToPack, packOptions);
        if (FAILED(hrCreate))
        {
            winrt::throw_hresult(hrCreate);
        }
        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Unpack(hstring packageFilePathToUnpack, UnpackOptions unpackOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::Bundle(hstring directoryPathToBundle, BundleOptions bundleOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::Unbundle(hstring bundleFilePathToUnbundle, UnbundleOptions unbundleOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateKozaniPackage(hstring packageFilePathToConvert,
            CreateKozaniPackageOptions createKozaniPackageOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Mount(hstring imageFilePathToMount, bool readOnly)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::Unmount(hstring imageFilePathToUnmount)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::CreateMountableImage(hstring imageFilePath,
        CreateMountableImageOptions createMountableImageOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
    Windows::Foundation::IAsyncAction MakeMSIXManager::AddPackageToImage(hstring packageFilePath, hstring imageFilePath,
        AddPackageToImageOptions addPackageToImageOptions)
    {
        winrt::throw_hresult(E_NOTIMPL);
    }
}
