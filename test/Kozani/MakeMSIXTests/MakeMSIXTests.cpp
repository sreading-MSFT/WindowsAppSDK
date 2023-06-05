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

        HRESULT CreateDirectory(std::filesystem::path tempDirPath)
        {
            std::error_code createDirectoryError;
            bool createTempDirResult = std::filesystem::create_directory(tempDirPath, createDirectoryError);
            if (!createTempDirResult || createDirectoryError.value() != 0)
            {
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        HRESULT CreateTempDirectory(_Out_ std::filesystem::path& tempDirPath)
        {
            // Create a temporary directory to unpack package(s) since we cannot unpack to the CIM directly.
            // Append long path prefix to temporary directory path to handle paths that exceed the maximum path length limit
            std::wstring uniqueIdString{};
            RETURN_IF_FAILED(CreateGUIDString(uniqueIdString));
            tempDirPath.assign(std::filesystem::temp_directory_path());
            tempDirPath /= uniqueIdString;
            RETURN_IF_FAILED(CreateDirectory(tempDirPath));
            return S_OK;
        }

        const PCWSTR c_makeMSIXTestDataDir = L"data";
        const PCWSTR c_testBundleLayoutDirectory = L"TestBundleLayout";
        const PCWSTR c_testPackageLayoutDirectory = L"TestPackageLayout";
        const PCWSTR c_testPackageFile = L"TestPackage.msix";
        const PCWSTR c_testBundleFile = L"TestBundle.msixbundle";

        const PCWSTR c_testKozaniPackageOutputFile = L"TestKozaniPackage.msix";
        TEST_METHOD(CreateBundleTest)
        {
            winrt::init_apartment();
            try
            {
                auto testDataPath = TF::GetTestAbsoluteFilename();
                testDataPath = testDataPath.remove_filename();
                testDataPath /= c_makeMSIXTestDataDir;
                auto bundleLayoutDir(testDataPath);
                bundleLayoutDir /= c_testBundleLayoutDirectory;

                std::filesystem::path outputBundle{};
                winrt::check_hresult(CreateTempDirectory(outputBundle));
                outputBundle /= c_testBundleFile;

                auto createBundleOptions = CreateBundleOptions();
                createBundleOptions.OverwriteOutputFileIfExists(true);
                createBundleOptions.Version(winrt::Windows::ApplicationModel::PackageVersion(1, 0, 0, 0));

                MakeMSIXManager::CreateBundle(bundleLayoutDir.c_str(), outputBundle.c_str(), createBundleOptions).get();

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
                auto testDataPath = TF::GetTestAbsoluteFilename();
                testDataPath = testDataPath.remove_filename();
                testDataPath /= c_makeMSIXTestDataDir;
                auto bundleFileName(testDataPath);
                bundleFileName /= c_testBundleFile;

                std::filesystem::path outputBundleDir;
                winrt::check_hresult(CreateTempDirectory(outputBundleDir));
                outputBundleDir /= c_testBundleLayoutDirectory;
                winrt::check_hresult(CreateDirectory(outputBundleDir));

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
                auto testDataPath = TF::GetTestAbsoluteFilename();
                testDataPath = testDataPath.remove_filename();
                testDataPath /= c_makeMSIXTestDataDir;
                auto packageLayoutPath(testDataPath);
                packageLayoutPath /= c_testPackageLayoutDirectory;

                std::filesystem::path outputPackage;
                winrt::check_hresult(CreateTempDirectory(outputPackage));
                outputPackage /= c_testPackageFile;

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
                auto testDataPath = TF::GetTestAbsoluteFilename();
                testDataPath = testDataPath.remove_filename();
                testDataPath /= c_makeMSIXTestDataDir;
                auto packagePath(testDataPath);
                packagePath /= c_testPackageFile;

                std::filesystem::path outputPackageDir;
                winrt::check_hresult(CreateTempDirectory(outputPackageDir));
                outputPackageDir /= c_testPackageLayoutDirectory;
                winrt::check_hresult(CreateDirectory(outputPackageDir));

                auto extractPackageOptions = ExtractPackageOptions();
                extractPackageOptions.OverwriteOutputFilesIfExists(true);

                MakeMSIXManager::ExtractPackage(packagePath.c_str(), outputPackageDir.c_str(), extractPackageOptions).get();

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
            auto testDataPath = TF::GetTestAbsoluteFilename();
            testDataPath = testDataPath.remove_filename();
            testDataPath /= c_makeMSIXTestDataDir;
            auto packagePath(testDataPath);
            packagePath /= c_testPackageFile;

            std::filesystem::path outputPackageFile;
            winrt::check_hresult(CreateTempDirectory(outputPackageFile));
            outputPackageFile /= c_testKozaniPackageOutputFile;

            // Append "Kozani" to the package name, and trim non-english languages from the package.
            PackageInformation packageInfo = MakeMSIXManager::GetPackageInformation(packagePath.c_str()).get();
            std::wstring newPackageName{ packageInfo.Identity().Name() + L"Kozani" };

            CreateKozaniPackageOptions createKozaniPackageOptions = CreateKozaniPackageOptions();
            createKozaniPackageOptions.OverwriteOutputFileIfExists(true);
            createKozaniPackageOptions.Name(newPackageName);
            createKozaniPackageOptions.Languages().Append(L"en-US");

            MakeMSIXManager::CreateKozaniPackage(packagePath.c_str(), outputPackageFile.c_str(), createKozaniPackageOptions).get();

            // TODO: Compare extracted package directory to expected kozani package layout directory
        }
    };
}
