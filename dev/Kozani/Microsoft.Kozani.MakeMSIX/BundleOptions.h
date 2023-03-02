// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "BundleOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct BundleOptions : BundleOptionsT<BundleOptions>
    {
        BundleOptions() = default;

        
        hstring BundleFilePath();
        void BundleFilePath(hstring);
        bool OverwriteFiles();
        void OverwriteFiles(bool);
        bool FlatBundle();
        void FlatBundle(bool);
        Windows::ApplicationModel::PackageVersion BundleVersion();
        void BundleVersion(Windows::ApplicationModel::PackageVersion);

        hstring mBundleFilePath{};
        bool mOverwriteFiles{};
        bool mFlatBundle{};
        Windows::ApplicationModel::PackageVersion mBundleVersion{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct BundleOptions : BundleOptionsT<BundleOptions, implementation::BundleOptions>
    {
    };
}
