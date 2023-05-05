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

            // Iterate through package layout folders and pack each one.
            for (const std::filesystem::directory_entry& directoryEntry : std::filesystem::directory_iterator(packageLayoutRootPath))
            {
                if (!directoryEntry.is_directory())
                {
                    continue;
                }
                PackOptions packOptions = PackOptions();
                packOptions.OverwriteFiles(true);
                std::wstring packageFolderName = directoryEntry.path().filename().wstring();
                std::wstring packageOutputFilePath{ packageOutputRootDirectoryPath + packageFolderName + L".appx" };
                packOptions.PackageFilePath(packageOutputFilePath);

                MakeMSIXManager::Pack(directoryEntry.path().c_str(), packOptions).get();
            }
            // Bundle all the packages together.
            auto bundleOptions = BundleOptions();
            bundleOptions.OverwriteFiles(true);
            bundleOptions.BundleFilePath(bundleOutputFilePath);
            bundleOptions.BundleVersion(winrt::Windows::ApplicationModel::PackageVersion(1, 1, 0, 0));
            MakeMSIXManager::Bundle(packageOutputRootDirectoryPath, bundleOptions).get();

            bundleOptions.BundleVersion(winrt::Windows::ApplicationModel::PackageVersion(1, 1, 0, 0));

            // Sign the bundle
            Example_SignPackage(developerCertPfxFile, bundleOutputFilePath);

            // Step 2. Create the kozani package from the bundle created in Step 1.
            CreateKozaniPackageOptions kozaniPackOptions = CreateKozaniPackageOptions();
            kozaniPackOptions.OverwriteFiles(true);
            kozaniPackOptions.PackageFilePath(kozaniPackageOutputFilePath);
            MakeMSIXManager::CreateKozaniPackage(bundleOutputFilePath, kozaniPackOptions).get();

            // Step 3. Create app attach vhd from the bundle created in Step 1.
            CreateMountableImageOptions mountableImageOptions = CreateMountableImageOptions();
            mountableImageOptions.ImageFilePath(appAttachImageFilePath);
            mountableImageOptions.OverwriteFiles(true);
            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> packagesToAddToImage{ winrt::single_threaded_vector<winrt::hstring>() };
            packagesToAddToImage.Append(bundleOutputFilePath);

            MakeMSIXManager::CreateMountableImage(packagesToAddToImage, mountableImageOptions).get();
        }
        TEST_METHOD(TestBundle)
        {
            winrt::init_apartment();
            try
            {
                auto bundleOptions = BundleOptions();
                bundleOptions.OverwriteFiles(true);
                bundleOptions.BundleFilePath(L"E:\\test\\bundledOutput.msixbundle");
                bundleOptions.BundleVersion(winrt::Windows::ApplicationModel::PackageVersion(1, 1, 0, 0));

                MakeMSIXManager::Bundle(L"E:\\test\\unbundledPackageInput", bundleOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }
        TEST_METHOD(TestUnpack)
        {
            winrt::init_apartment();
            try
            {
                auto unpackOptions = UnpackOptions();
                unpackOptions.OverwriteFiles(true);
                unpackOptions.UnpackedPackageRootDirectory(L"E:\\test\\unpackedPackageOutput");

                MakeMSIXManager::Unpack(L"E:\\test\\package.msix", unpackOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }
        TEST_METHOD(TestCreatePackageScenarios)
        {
            CreatePackagesFromFolderExample();
        }

        void Example_ChangeManifestVersion(std::wstring appxManifestPath, winrt::Windows::ApplicationModel::PackageVersion newVersion)
        {
            winrt::Windows::Storage::StorageFile manifestFile = winrt::Windows::Storage::StorageFile::GetFileFromPathAsync(appxManifestPath).get();
            winrt::Windows::Data::Xml::Dom::XmlDocument docElement = winrt::Windows::Data::Xml::Dom::XmlDocument::LoadFromFileAsync(manifestFile).get();

            std::wstring packageIdentityVersionQuery{ L"/*[local-name()='Package']/*[local-name()='Identity']/@Version" };
            winrt::Windows::Data::Xml::Dom::IXmlNode identityVersionAttributeNode = docElement.SelectSingleNode(packageIdentityVersionQuery.c_str());

            std::wstring newVersionString = std::to_wstring(newVersion.Major) + L"."
                + std::to_wstring(newVersion.Minor) + L"."
                + std::to_wstring(newVersion.Build) + L"."
                + std::to_wstring(newVersion.Revision);
            identityVersionAttributeNode.InnerText(newVersionString);

            docElement.SaveToFileAsync(manifestFile).get();
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
                auto unbundleOptions = UnbundleOptions();
                unbundleOptions.OverwriteFiles(true);
                unbundleOptions.UnbundledPackageRootDirectory(outputDirForBundle.c_str());
                MakeMSIXManager::Unbundle(bundleFilePath.c_str(), unbundleOptions).get();

                // Iterate through the bundled packages and unpack each one.
                for (const auto& file : std::filesystem::directory_iterator(outputDirForBundle))
                {
                    // Skip the metadata files.
                    std::wstring fileExtension{ file.path().extension() };
                    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);
                    if ((fileExtension.compare(L".appx") != 0) &&
                        (fileExtension.compare(L".msix") != 0))
                    {
                        continue;
                    }

                    // Name the unpacked folders based on the package file name.
                    std::filesystem::path outputDirForPackage{ outputDirRootForPackages };
                    outputDirForPackage /= file.path().stem();

                    auto unpackOptions = UnpackOptions();
                    unpackOptions.OverwriteFiles(true);
                    unpackOptions.UnpackedPackageRootDirectory(outputDirForPackage.c_str());
                    MakeMSIXManager::Unpack(file.path().c_str(), unpackOptions).get();
                }
                                
                std::filesystem::path outputDirRootForPackedPackages{ outputDirRoot };
                outputDirRootForPackedPackages /= L"packedPackages";
                std::filesystem::path outputPathForBundle{ outputDirRoot };
                outputPathForBundle /= bundleFilePath.filename();

                // Iterate through packages and repack each one.
                for (const auto& packageDir : std::filesystem::directory_iterator(outputDirRootForPackages))
                {
                    // Open the manifest and change the version before re-packing
                    std::filesystem::path appxManifestPath{ packageDir };
                    appxManifestPath /= L"AppxManifest.xml";
                    Example_ChangeManifestVersion(appxManifestPath, newVersion);                    

                    std::filesystem::path outputPackagePath{ outputDirRootForPackedPackages };
                    std::wstring outputPackageFileName = packageDir.path().filename().wstring() + L".appx";
                    outputPackagePath /= outputPackageFileName;

                    auto packOptions = PackOptions();
                    packOptions.OverwriteFiles(true);
                    packOptions.PackageFilePath(outputPackagePath.c_str());
                    MakeMSIXManager::Pack(packageDir.path().c_str(), packOptions).get();
                }

                // Re-bundle with the new version
                auto bundleOptions = BundleOptions();
                bundleOptions.OverwriteFiles(true);
                bundleOptions.BundleFilePath(outputPathForBundle.c_str());
                bundleOptions.BundleVersion(newVersion);
                MakeMSIXManager::Bundle(outputDirRootForPackedPackages.c_str(), bundleOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }

        TEST_METHOD(TestUnbundle)
        {
            ChangeVersionOfAllPackagesInBundle();
        }


        TEST_METHOD(TestConvertPackages)
        {
            std::filesystem::path folderContainingPackagedApps{ L"D:\\test\\Packages" };
            std::filesystem::path outputFolderContainingKozaniApps{ L"D:\\test\\KozaniPackages" };
            // Iterate through folder and unpack each package.
            for (const std::filesystem::directory_entry& directoryEntry : std::filesystem::directory_iterator(folderContainingPackagedApps))
            {
                if (directoryEntry.is_directory())
                {
                    continue;
                }

                CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
                createKozaniPackageOptions.OverwriteFiles(true);
                std::wstring fileNameWithoutExtension = directoryEntry.path().stem().wstring();
                std::wstring packageOutputFileName{ fileNameWithoutExtension + L"Kozani" + directoryEntry.path().extension().c_str()};
                std::filesystem::path packageOutputPath{ outputFolderContainingKozaniApps };
                packageOutputPath /= packageOutputFileName;
                createKozaniPackageOptions.PackageFilePath(packageOutputPath.c_str());

                MakeMSIXManager::CreateKozaniPackage(directoryEntry.path().c_str(), createKozaniPackageOptions).get();
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

            CreateKozaniPackageOptions kozaniPackOptions = CreateKozaniPackageOptions();
            kozaniPackOptions.OverwriteFiles(true);
            kozaniPackOptions.PackageFilePath(kozaniPackageOutputFilePath);
            MakeMSIXManager::CreateKozaniPackage(bundleOutputFilePath, kozaniPackOptions).get();

            CreateMountableImageOptions mountableImageOptions = CreateMountableImageOptions();
            mountableImageOptions.ImageFilePath(appAttachImageFilePath);
            mountableImageOptions.OverwriteFiles(true);
            winrt::Windows::Foundation::Collections::IVector<winrt::hstring> packagesToAddToImage{ winrt::single_threaded_vector<winrt::hstring>() };
            packagesToAddToImage.Append(bundleOutputFilePath);

            MakeMSIXManager::CreateMountableImage(packagesToAddToImage, mountableImageOptions).get();
        }

        TEST_METHOD(TestCreatePackageVariants)
        {
            CreatePackageVariants();
        }

    };
}
