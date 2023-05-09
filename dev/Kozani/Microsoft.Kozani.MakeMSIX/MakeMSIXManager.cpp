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

    std::list<std::wstring> GetCommandLineArgumentsForMakeAppxPack(std::wstring directoryToPack, PackOptions packOptions)
    {
        std::list<std::wstring> arguments;
        arguments.push_back(L"pack");
        arguments.push_back(L"/d");
        arguments.push_back(directoryToPack);
        arguments.push_back(L"/p");
        arguments.push_back(packOptions.PackageFilePath().c_str());
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
        return arguments;
    }
    std::list<std::wstring> GetCommandLineArgumentsForMakeAppxUnpack(std::wstring packageToUnpack, UnpackOptions unpackOptions)
    {
        std::list<std::wstring> arguments;
        arguments.push_back(L"unpack");
        arguments.push_back(L"/p");
        arguments.push_back(packageToUnpack);
        arguments.push_back(L"/d");
        arguments.push_back(unpackOptions.UnpackedPackageRootDirectory().c_str());
        if (unpackOptions.OverwriteFiles())
        {
            arguments.push_back(L"/o");
        }
        else
        {
            arguments.push_back(L"/no");
        }
        return arguments;
    }
    std::list<std::wstring> GetCommandLineArgumentsForMakeAppxUnbundle(std::wstring packageToUnbundle, UnbundleOptions unbundleOptions)
    {
        std::list<std::wstring> arguments;
        arguments.push_back(L"unbundle");
        arguments.push_back(L"/p");
        arguments.push_back(packageToUnbundle);
        arguments.push_back(L"/d");
        arguments.push_back(unbundleOptions.UnbundledPackageRootDirectory().c_str());
        if (unbundleOptions.OverwriteFiles())
        {
            arguments.push_back(L"/o");
        }
        else
        {
            arguments.push_back(L"/no");
        }
        return arguments;
    }
    std::list<std::wstring> GetCommandLineArgumentsForMakeAppxBundle(std::wstring directoryToBundle, BundleOptions bundleOptions)
    {
        std::list<std::wstring> arguments;
        arguments.push_back(L"bundle");
        arguments.push_back(L"/d");
        arguments.push_back(directoryToBundle);
        arguments.push_back(L"/p");
        arguments.push_back(bundleOptions.BundleFilePath().c_str());
        if (bundleOptions.OverwriteFiles())
        {
            arguments.push_back(L"/o");
        }
        else
        {
            arguments.push_back(L"/no");
        }
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

    Windows::Foundation::IAsyncAction MakeMSIXManager::Pack(hstring directoryPathToPack, PackOptions packOptions)
    {
        winrt::check_hresult(ValidateCreatePackageArguments(directoryPathToPack, packOptions));

        // Package creation is expected to be slow.
        co_await winrt::resume_background();

        std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxPack(directoryPathToPack.c_str(), packOptions);
        winrt::check_hresult(LaunchMakeAppxWithArguments(arguments));

        co_return;
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Unpack(hstring packageFilePathToUnpack, UnpackOptions unpackOptions)
    {
        co_await winrt::resume_background();

        std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxUnpack(packageFilePathToUnpack.c_str(), unpackOptions);
        winrt::check_hresult(LaunchMakeAppxWithArguments(arguments));
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Bundle(hstring directoryPathToBundle, BundleOptions bundleOptions)
    {
        co_await winrt::resume_background();

        std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxBundle(directoryPathToBundle.c_str(), bundleOptions);
        winrt::check_hresult(LaunchMakeAppxWithArguments(arguments));
    }

    Windows::Foundation::IAsyncAction MakeMSIXManager::Unbundle(hstring bundleFilePathToUnbundle, UnbundleOptions unbundleOptions)
    {
        co_await winrt::resume_background();

        std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxUnbundle(bundleFilePathToUnbundle.c_str(), unbundleOptions);
        winrt::check_hresult(LaunchMakeAppxWithArguments(arguments));
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
        CreateTempDirectory(tempKozaniLayoutDirectory);
        winrt::check_hresult(CreateKozaniPackageLayout(tempFullPackageUnpackDirectory, tempKozaniLayoutDirectory));

        PackOptions packOptions = PackOptions();
        packOptions.OverwriteFiles(true);
        packOptions.PackageFilePath(createKozaniPackageOptions.PackageFilePath().c_str());
        std::list<std::wstring> arguments = GetCommandLineArgumentsForMakeAppxPack(tempKozaniLayoutDirectory, packOptions);
        winrt::check_hresult(LaunchMakeAppxWithArguments(arguments));
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
