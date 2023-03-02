// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "UnbundleOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct UnbundleOptions : UnbundleOptionsT<UnbundleOptions>
    {
        UnbundleOptions() = default;

        hstring UnbundledPackageRootDirectory();
        void UnbundledPackageRootDirectory(hstring);
        bool OverwriteFiles();
        void OverwriteFiles(bool);

        hstring mUnbundledPackageRootDirectory{};
        bool mOverwriteFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct UnbundleOptions : UnbundleOptionsT<UnbundleOptions, implementation::UnbundleOptions>
    {
    };
}
