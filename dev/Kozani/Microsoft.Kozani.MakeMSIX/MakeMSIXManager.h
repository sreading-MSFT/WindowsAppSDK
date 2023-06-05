// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "MakeMSIXManager.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct MakeMSIXManager
    {
        MakeMSIXManager() = default;

        static Windows::Foundation::IAsyncAction CreatePackage(
            hstring inputPath,
            hstring outputFileName,
            CreatePackageOptions createPackageOptions);

        static Windows::Foundation::IAsyncAction ExtractPackage(
            hstring inputFileName,
            hstring outputPath,
            ExtractPackageOptions extractPackageOptions);

        static Windows::Foundation::IAsyncAction CreateBundle(
            hstring inputPath,
            hstring outputFileName,
            CreateBundleOptions createBundleOptions);

        static Windows::Foundation::IAsyncAction ExtractBundle(
            hstring inputFileName,
            hstring outputPath,
            ExtractBundleOptions extractBundleOptions);

        static Windows::Foundation::IAsyncAction CreateKozaniPackage(
            hstring inputFileName,
            hstring outputFileName,
            CreateKozaniPackageOptions createKozaniPackageOptions);

        static Windows::Foundation::IAsyncAction CreateMountableImage(
            winrt::Windows::Foundation::Collections::IVector<hstring> inputFileNames,
            hstring outputFileName,
            CreateMountableImageOptions createMountableImageOptions);

        static Windows::Foundation::IAsyncOperation<winrt::Microsoft::Kozani::MakeMSIX::PackageInformation> GetPackageInformation(
            hstring packagePath);
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct MakeMSIXManager : MakeMSIXManagerT<MakeMSIXManager, implementation::MakeMSIXManager, static_lifetime>
    {
    };
}
