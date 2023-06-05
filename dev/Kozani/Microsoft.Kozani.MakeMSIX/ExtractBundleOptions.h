// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "ExtractBundleOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct ExtractBundleOptions : ExtractBundleOptionsT<ExtractBundleOptions>
    {
        ExtractBundleOptions() = default;

        bool OverwriteOutputFilesIfExists();
        void OverwriteOutputFilesIfExists(bool);

    private:
        bool mOverwriteOutputFilesIfExists{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct ExtractBundleOptions : ExtractBundleOptionsT<ExtractBundleOptions, implementation::ExtractBundleOptions>
    {
    };
}
