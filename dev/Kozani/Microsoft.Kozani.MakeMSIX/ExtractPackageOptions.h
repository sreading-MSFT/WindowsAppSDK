// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "ExtractPackageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct ExtractPackageOptions : ExtractPackageOptionsT<ExtractPackageOptions>
    {
        ExtractPackageOptions() = default;

        bool OverwriteOutputFilesIfExists();
        void OverwriteOutputFilesIfExists(bool);
    private:
        bool mOverwriteOutputFilesIfExists{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct ExtractPackageOptions : ExtractPackageOptionsT<ExtractPackageOptions, implementation::ExtractPackageOptions>
    {
    };
}
