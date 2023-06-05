// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <winrt/base.h>
#include <winrt/Microsoft.Kozani.MakeMSIX.h>
#include <winrt/windows.storage.h>
#include <winrt/windows.data.xml.dom.h>

using namespace winrt::Microsoft::Kozani::MakeMSIX;

namespace TB = ::Test::Bootstrap;
namespace TP = ::Test::Packages;
namespace TF = ::Test::FileSystem;

namespace Test::MakeMSIXTests
{
    class MakeMSIXTests
    {
    public:
        BEGIN_TEST_CLASS(MakeMSIXTests)
            TEST_CLASS_PROPERTY(L"Description", L"Windows App SDK Kozani Packaging")
            TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
            TEST_CLASS_PROPERTY(L"RunAs:Class", L"RestrictedUser")
            TEST_CLASS_PROPERTY(L"IsolationLevel", L"Class")
        END_TEST_CLASS()

        TEST_CLASS_SETUP(ClassSetup)
        {
            ::TB::Setup();
            return true;
        }

        TEST_CLASS_CLEANUP(ClassCleanup)
        {
            ::TB::Cleanup();
            return true;
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

        const PCWSTR c_testBundleLayoutDirectory = L"TestBundleLayout";
        const PCWSTR c_testPackageLayoutDirectory = L"TestPackageLayout";
        const PCWSTR c_testPackageInputFile = L"TestPackage.msix";
        const PCWSTR c_testBundleInputFile = L"TestBundle.msixbundle";

        const PCWSTR c_testBundleOutputFile = L"TestBundle.msixbundle";
        const PCWSTR c_testPackageOutputFile = L"TestPackage.msix";
        const PCWSTR c_testKozaniPackageOutputFile = L"TestKozaniPackage.msix";
        TEST_METHOD(BundleTest)
        {
            winrt::init_apartment();
            try
            {
                auto solutionOutDirPath = TF::GetSolutionOutDirPath();
                auto msix(solutionOutDirPath);
                msix /= c_testBundleLayoutDirectory;

                std::wstring tempBundleOutputDir{};
                winrt::check_hresult(CreateTempDirectory(tempBundleOutputDir));
                std::filesystem::path outputBundle(tempBundleOutputDir);
                outputBundle /= c_testBundleOutputFile;

                auto createBundleOptions = CreateBundleOptions();
                createBundleOptions.OverwriteOutputFileIfExists(true);
                createBundleOptions.Version(winrt::Windows::ApplicationModel::PackageVersion(1, 1, 0, 0));

                MakeMSIXManager::CreateBundle(solutionOutDirPath.c_str(), outputBundle.c_str(), createBundleOptions).get();

                // TODO: Compare created bundle to expected bundle file.
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }

        TEST_METHOD(ExtractBundleTest)
        {
            winrt::init_apartment();
            try
            {
                auto solutionOutDirPath = TF::GetSolutionOutDirPath();
                auto bundleFileName(solutionOutDirPath);
                bundleFileName /= c_testBundleInputFile;

                std::wstring tempBundleOutputDir{};
                winrt::check_hresult(CreateTempDirectory(tempBundleOutputDir));
                std::filesystem::path outputBundleDir(tempBundleOutputDir);
                outputBundleDir /= c_testBundleLayoutDirectory;

                auto extractBundleOptions = ExtractBundleOptions();
                extractBundleOptions.OverwriteOutputFilesIfExists(true);

                MakeMSIXManager::ExtractBundle(bundleFileName.c_str(), outputBundleDir.c_str(), extractBundleOptions).get();

                // TODO: Compare extracted bundle directory to expected bundle layout directory
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }

        TEST_METHOD(CreatePackageTest)
        {
            winrt::init_apartment();
            try
            {
                auto solutionOutDirPath = TF::GetSolutionOutDirPath();
                auto packageLayoutPath(solutionOutDirPath);
                packageLayoutPath /= c_testPackageLayoutDirectory;

                std::wstring tempDir{};
                winrt::check_hresult(CreateTempDirectory(tempDir));
                std::filesystem::path outputPackage(tempDir);
                outputPackage /= c_testPackageOutputFile;

                auto createPackageOptions = CreatePackageOptions();
                createPackageOptions.OverwriteOutputFileIfExists(true);
                createPackageOptions.Version(winrt::Windows::ApplicationModel::PackageVersion(1, 1, 0, 0));

                MakeMSIXManager::CreatePackage(packageLayoutPath.c_str(), outputPackage.c_str(), createPackageOptions).get();

                // TODO: Compare created package to expected package file.
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }
        TEST_METHOD(ExtractPackageTest)
        {
            winrt::init_apartment();
            try
            {
                auto solutionOutDirPath = TF::GetTestAbsoluteFilename();
                solutionOutDirPath = solutionOutDirPath.remove_filename();
                auto msix(solutionOutDirPath);
                msix /= c_testPackageInputFile;

                std::wstring tempExtractPackageOutput{};
                winrt::check_hresult(CreateTempDirectory(tempExtractPackageOutput));
                std::filesystem::path outputPackageDir(tempExtractPackageOutput);
                outputPackageDir /= c_testPackageLayoutDirectory;

                auto extractPackageOptions = ExtractPackageOptions();
                extractPackageOptions.OverwriteOutputFilesIfExists(true);

                MakeMSIXManager::ExtractPackage(msix.c_str(), outputPackageDir.c_str(), extractPackageOptions).get();

                // TODO: Compare extracted package directory to expected package layout directory
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }
        TEST_METHOD(TestCreateKozaniPackage)
        {
            auto solutionOutDirPath = TF::GetTestAbsoluteFilename();
            solutionOutDirPath = solutionOutDirPath.remove_filename();
            auto msix(solutionOutDirPath);
            msix /= c_testPackageInputFile;

            std::wstring tempDir{};
            winrt::check_hresult(CreateTempDirectory(tempDir));
            std::filesystem::path outputPackageFile(tempDir);
            outputPackageFile /= c_testKozaniPackageOutputFile;

            // Append "Kozani" to the package name, and trim non-english languages from the package.
            PackageInformation packageInfo = MakeMSIXManager::GetPackageInformation(msix.c_str()).get();
            std::wstring newPackageName{ packageInfo.Identity().Name() + L"Kozani" };

            CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
            createKozaniPackageOptions.OverwriteOutputFileIfExists(true);
            createKozaniPackageOptions.Name(newPackageName);
            createKozaniPackageOptions.Languages().Append(L"en-US");

            MakeMSIXManager::CreateKozaniPackage(msix.c_str(), outputPackageFile.c_str(), createKozaniPackageOptions).get();

            // TODO: Compare extracted package directory to expected package layout directory
        }

        // Examples for API spec below. Can be removed.

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

        std::filesystem::path GetPlatformSDKPath()
        {
            wil::unique_hkey hKey;
            std::wstring sdkRegPath{ LR"(SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)" };
            winrt::check_hresult(RegOpenKeyEx(HKEY_LOCAL_MACHINE, sdkRegPath.c_str(), 0, KEY_READ, &hKey));

            std::wstring installPathKeyName{ L"InstallationFolder" };
            WCHAR installPathBuffer[MAX_PATH];
            DWORD installPathBufferLength = sizeof(installPathBuffer);
            winrt::check_hresult(RegGetValueW(
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
            winrt::check_hresult(RegGetValueW(
                hKey.get(),
                nullptr,
                productVersionKeyName.c_str(),
                RRF_RT_REG_SZ,
                nullptr /* pdwType */,
                &productVersionBuffer,
                &productVersionBufferLength));
            std::wstring productVersionString{ productVersionBuffer };
            // The install path of the platform sdk in the filesystem always uses version format x.x.x.x
            // even though the version string in the registry may be stored as x.x if the full version is
            // x.x.0.0 
            auto versionPartsFound = std::count(productVersionString.begin(), productVersionString.end(), '.') + 1;
            INT64 versionPartsRequired = 4;
            INT64 versionDigitsToAdd = versionPartsRequired - versionPartsFound;
            for (auto i = 0; i < versionDigitsToAdd; i++)
            {
                productVersionString += L".0";
            }
            std::wstring processorArchitecture{};
            SYSTEM_INFO systemInfo;
            GetNativeSystemInfo(&systemInfo);
            switch (systemInfo.wProcessorArchitecture)
            {
            case PROCESSOR_ARCHITECTURE_AMD64:
                processorArchitecture += L"x64";
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                processorArchitecture += L"arm";
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                processorArchitecture += L"arm64";
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                processorArchitecture += L"x86";
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                winrt::throw_hresult(E_UNEXPECTED);
            }

            std::filesystem::path platformSDKExecutablePath = installPathString;
            platformSDKExecutablePath /= L"bin";
            platformSDKExecutablePath /= productVersionString;
            platformSDKExecutablePath /= processorArchitecture;

            return platformSDKExecutablePath;
        }

        void LaunchSignToolWithArguments(std::wstring arguments)
        {
            std::filesystem::path signToolPath = GetPlatformSDKPath();
            signToolPath /= L"signtool.exe";

            std::wstring commandLine{ signToolPath };
            commandLine += L" " + arguments;
            DWORD exitCode{};
            winrt::check_hresult(CreateProcessAndWaitForExitCode(commandLine, exitCode));
            if (exitCode == EXIT_FAILURE)
            {
                winrt::throw_hresult(E_FAIL);
            }
        }

        void Example_SignPackage(std::wstring pfxFile, std::wstring packageFile)
        {
            std::wstring signToolArguments;
            signToolArguments.append(L"sign /fd SHA256 /f ");
            signToolArguments.append(pfxFile);
            signToolArguments.append(L" " + packageFile);
            LaunchSignToolWithArguments(signToolArguments);
        }

        void CreatePackagesFromFolderExample()
        {
            winrt::init_apartment();

            // Scenario: A developer is creating their own app. A build tool wants to create every type of distributable package for them.

            // Initial Conditions: The build has already produced a directory that contains the exact layout of each individual package
            // that it wants to bundle.
            // Each folder has an appxmanifest.xml in the root directory.

            // A full non-relative path to those layout folder is needed, as below.
            // Layout directories are inside this root, for example: packageLayoutRootPath\\ContosoApp1_2023.302.1739.686_x64__8wekyb3d8bbwe.
            std::wstring packageLayoutRootPath{ L"D:\\test\\ContosoApp1\\BuildOutput\\PackageLayouts" };

            // The build tool packages each folder and puts the resulting packages at this location
            std::wstring packageOutputRootDirectoryPath{ L"D:\\test\\ContosoApp1\\BuildOutput\\packagedOutput\\" };
            // The build tool wants to create the full bundle at this location
            std::wstring bundleOutputFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\bundleOutput.msixbundle" };
            // The build tool wants to create the kozani package at this location
            std::wstring kozaniPackageOutputFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\kozaniPackagedOutput.msix" };
            // The build tool wants to create the app attach image at this location
            std::wstring appAttachImageFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\appAttachOutput.vhdx" };
            // The certificate to sign the package is at
            std::wstring developerCertPfxFile{ L"D:\\test\\ContosoApp1\\developerCert.pfx" };

            // Step 1. Create the full bundle.
            PackageIdentity packageIdentity = nullptr;
            // Iterate through package layout folders and pack each one.
            for (const std::filesystem::directory_entry& directoryEntry : std::filesystem::directory_iterator(packageLayoutRootPath))
            {
                if (!directoryEntry.is_directory())
                {
                    continue;
                }

                CreatePackageOptions createPackageOptions = CreatePackageOptions();
                createPackageOptions.OverwriteOutputFileIfExists(true);
                std::wstring packageFolderName = directoryEntry.path().filename().wstring();
                std::wstring packageOutputFilePath{ packageOutputRootDirectoryPath + packageFolderName + L".appx" };

                MakeMSIXManager::CreatePackage(directoryEntry.path().c_str(), packageOutputFilePath.c_str(), createPackageOptions).get();

                // All packages in the bundle have the same version in this example, grab one of them to set on the bundle.
                if (packageIdentity == nullptr)
                {
                    packageIdentity = MakeMSIXManager::GetPackageInformation(directoryEntry.path().c_str()).get().Identity();
                }
            }

            // Confirm the directory did contain package layouts.
            if (packageIdentity == nullptr)
            {
                winrt::throw_hresult(E_INVALIDARG);
            }

            // Bundle all the packages together.
            CreateBundleOptions createbundleOptions = CreateBundleOptions();
            createbundleOptions.OverwriteOutputFileIfExists(true);
            createbundleOptions.FlatBundle(true);
            createbundleOptions.Version(packageIdentity.Version());
            MakeMSIXManager::CreateBundle(packageOutputRootDirectoryPath, bundleOutputFilePath.c_str(), createbundleOptions).get();

            // Sign the bundle
            Example_SignPackage(developerCertPfxFile, bundleOutputFilePath);

            // Step 2. Create the kozani package from the bundle created in Step 1.
            CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
            createKozaniPackageOptions.OverwriteOutputFileIfExists(true);
            createKozaniPackageOptions.RemoveExtensions(true);
            // Append "Kozani" to the package name, and trim non-english languages from the package.
            std::wstring newPackageName{ packageIdentity.Name() + L"Kozani" };
            createKozaniPackageOptions.Name(newPackageName);
            createKozaniPackageOptions.Languages().Append(L"en-US");
            MakeMSIXManager::CreateKozaniPackage(bundleOutputFilePath, kozaniPackageOutputFilePath.c_str(), createKozaniPackageOptions).get();

            // Step 3. Create app attach vhd from the bundle created in Step 1.
            CreateMountableImageOptions mountableImageOptions = CreateMountableImageOptions();
            mountableImageOptions.OverwriteOutputFileIfExists(true);
            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> packagesToAddToImage{ winrt::single_threaded_vector<winrt::hstring>() };
            packagesToAddToImage.Append(bundleOutputFilePath);

            MakeMSIXManager::CreateMountableImage(packagesToAddToImage, appAttachImageFilePath.c_str(), mountableImageOptions).get();
        }

        void ChangeVersionOfAllPackagesInBundle()
        {
            winrt::init_apartment();
            try
            {
                // Scenario: A developer is writing code to update the version of all packages in a bundle.

                std::filesystem::path bundleFilePath{ L"D:\\test\\bundle.msixbundle" };
                std::wstring outputDirRoot{ L"D:\\test\\unpackedBundleOutput" };
                winrt::Windows::ApplicationModel::PackageVersion newVersion = winrt::Windows::ApplicationModel::PackageVersion(1, 0, 0, 0);

                std::filesystem::path outputDirForBundle{ outputDirRoot };
                outputDirForBundle /= L"bundle";
                std::filesystem::path outputDirRootForPackages{ outputDirRoot };
                outputDirRootForPackages /= L"unpackedPackages";

                // Unbundle the bundle to its own folder. After the operation the folder will contain the
                // bundled packages as appx\msix packages, as well as the bundle metadata.
                auto extractBundleOptions = ExtractBundleOptions();
                extractBundleOptions.OverwriteOutputFilesIfExists(true);
                MakeMSIXManager::ExtractBundle(bundleFilePath.c_str(), outputDirForBundle.c_str(), extractBundleOptions).get();

                // Iterate through the bundled packages and unpack each one.
                for (const auto& file : std::filesystem::directory_iterator(outputDirForBundle))
                {
                    // Skip the metadata files.
                    std::wstring fileExtension{ file.path().extension() };
                    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), towlower);
                    if ((fileExtension.compare(L".appx") != 0) &&
                        (fileExtension.compare(L".msix") != 0))
                    {
                        continue;
                    }

                    // Name the unpacked folders based on the package file name.
                    std::filesystem::path outputDirForPackage{ outputDirRootForPackages };
                    outputDirForPackage /= file.path().stem();

                    auto extractPackageOptions = ExtractPackageOptions();
                    extractPackageOptions.OverwriteOutputFilesIfExists(true);
                    MakeMSIXManager::ExtractPackage(file.path().c_str(), outputDirForPackage.c_str(), extractPackageOptions).get();
                }

                std::filesystem::path outputDirRootForPackedPackages{ outputDirRoot };
                outputDirRootForPackedPackages /= L"packedPackages";
                std::filesystem::path outputPathForBundle{ outputDirRoot };
                outputPathForBundle /= bundleFilePath.filename();

                // Iterate through packages and repack each one.
                for (const auto& packageDir : std::filesystem::directory_iterator(outputDirRootForPackages))
                {
                    std::filesystem::path outputPackagePath{ outputDirRootForPackedPackages };
                    std::wstring outputPackageFileName = packageDir.path().filename().wstring() + L".appx";
                    outputPackagePath /= outputPackageFileName;

                    // Change the version when re-packing
                    auto createPackageOptions = CreatePackageOptions();
                    createPackageOptions.OverwriteOutputFileIfExists(true);
                    createPackageOptions.Version(newVersion);
                    MakeMSIXManager::CreatePackage(packageDir.path().c_str(), outputPackagePath.c_str(), createPackageOptions).get();
                }

                // Re-bundle with the new version
                auto createBundleOptions = CreateBundleOptions();
                createBundleOptions.OverwriteOutputFileIfExists(true);
                createBundleOptions.Version(newVersion);
                MakeMSIXManager::CreateBundle(outputDirRootForPackedPackages.c_str(), outputPathForBundle.c_str(), createBundleOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }

        void CreatePackageVariants()
        {
            // The build tool wants to create the full bundle at this location
            std::wstring bundleOutputFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\bundleOutput.msixbundle" };
            // The build tool wants to create the kozani package at this location
            std::wstring kozaniPackageOutputFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\kozaniPackagedOutput.msix" };
            // The build tool wants to create the kozani package at this location
            std::wstring appAttachImageFilePath{ L"D:\\test\\ContosoApp1\\BuildOutput\\Package\\appAttach.vhdx" };

            CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
            createKozaniPackageOptions.OverwriteOutputFileIfExists(true);
            MakeMSIXManager::CreateKozaniPackage(bundleOutputFilePath, kozaniPackageOutputFilePath.c_str(), createKozaniPackageOptions).get();

            CreateMountableImageOptions createMountableImageOptions = CreateMountableImageOptions();
            createMountableImageOptions.OverwriteOutputFileIfExists(true);
            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> packagesToAddToImage{ winrt::single_threaded_vector<winrt::hstring>() };
            packagesToAddToImage.Append(bundleOutputFilePath);

            MakeMSIXManager::CreateMountableImage(packagesToAddToImage, appAttachImageFilePath, createMountableImageOptions).get();
        }

        TEST_METHOD(TestCreatePackageVariants)
        {
            CreatePackageVariants();
        }

        TEST_METHOD(TestCreatePackageScenarios)
        {
            CreatePackagesFromFolderExample();
        }
    };
}
