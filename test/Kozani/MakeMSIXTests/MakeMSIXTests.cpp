// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <winrt/base.h>
#include <winrt/Microsoft.Kozani.MakeMSIX.h>

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
        // TODO: Need to ingest test collateral here. Not sure how this is handled in other projects.
        TEST_METHOD(TestPack)
        {
            winrt::init_apartment();

            // Create package from folder.
            std::wstring packageOutputFilePath{ L"E:\\test\\packagedOutput.msix" };
            try
            {
                PackOptions packOptions = PackOptions();
                packOptions.OverwriteFiles(true);
                packOptions.PackageFilePath(packageOutputFilePath);

                MakeMSIXManager::Pack(L"E:\\test\\unpackedPackageInput", packOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }

            // Create kozani package from package.
            try
            {
                CreateKozaniPackageOptions kozaniPackOptions = CreateKozaniPackageOptions();
                kozaniPackOptions.OverwriteFiles(true);
                kozaniPackOptions.PackageFilePath(L"E:\\test\\kozaniPackagedOutput.msix");
                MakeMSIXManager::CreateKozaniPackage(packageOutputFilePath, kozaniPackOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }

            // Create app attach vhd from package.
            try
            {
                std::wstring appAttachImageFilePath{ L"E:\\test\\appAttachOutput.vhdx" };
                CreateMountableImageOptions mountableImageOptions = CreateMountableImageOptions();
                mountableImageOptions.DynamicallyExpandable(true);
                mountableImageOptions.MaximumExpandableImageSizeMegabytes(100);
                MakeMSIXManager::CreateMountableImage(appAttachImageFilePath, mountableImageOptions).get();

                AddPackageToImageOptions addToImageOptions = AddPackageToImageOptions();
                addToImageOptions.PackageRootDirectoryInImage(L"WindowsApps");
                MakeMSIXManager::AddPackageToImage(packageOutputFilePath, appAttachImageFilePath, addToImageOptions).get();
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }

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

        TEST_METHOD(TestUnbundle)
        {
            winrt::init_apartment();
            try
            {
                std::wstring outputDir{ L"E:\\test\\unpackedBundleOutput" };
                auto unbundleOptions = UnbundleOptions();
                unbundleOptions.OverwriteFiles(true);
                unbundleOptions.UnbundledPackageRootDirectory(outputDir);

                MakeMSIXManager::Unbundle(L"E:\\test\\bundle.msixbundle", unbundleOptions).get();

                // Iterate through packages and unpack each one.
                for (const auto& file : std::filesystem::directory_iterator(outputDir))
                {
                    auto unpackOptions = UnpackOptions();
                    unpackOptions.OverwriteFiles(true);
                    std::filesystem::path outputDirForPackage{ outputDir };
                    outputDirForPackage /= file.path().stem();
                    unpackOptions.UnpackedPackageRootDirectory(outputDirForPackage.c_str());

                    MakeMSIXManager::Unpack(file.path().c_str(), unpackOptions).get();
                }
            }
            catch (winrt::hresult_error const& ex)
            {
                OutputDebugString(ex.message().c_str());
                winrt::check_hresult(ex.code());
            }
        }

    };
}
