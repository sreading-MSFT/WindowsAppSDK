// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateBundleOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateBundleOptions : CreateBundleOptionsT<CreateBundleOptions>
    {
        CreateBundleOptions() = default;

        bool OverwriteOutputFileIfExists();
        void OverwriteOutputFileIfExists(bool);
        bool FlatBundle();
        void FlatBundle(bool);
        Windows::ApplicationModel::PackageVersion Version();
        void Version(Windows::ApplicationModel::PackageVersion);

    private:
        bool mOverwriteOutputFileIfExists{};
        bool mFlatBundle{};
        Windows::ApplicationModel::PackageVersion mVersion{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateBundleOptions : CreateBundleOptionsT<CreateBundleOptions, implementation::CreateBundleOptions>
    {
    };
}
