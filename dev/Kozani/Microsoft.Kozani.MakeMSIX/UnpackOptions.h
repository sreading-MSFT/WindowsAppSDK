// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "UnpackOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct UnpackOptions : UnpackOptionsT<UnpackOptions>
    {
        UnpackOptions() = default;

        hstring UnpackedPackageRootDirectory();
        void UnpackedPackageRootDirectory(hstring);
        bool OverwriteFiles();
        void OverwriteFiles(bool);

        hstring mUnpackedPackageRootDirectory{};
        bool mOverwriteFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct UnpackOptions : UnpackOptionsT<UnpackOptions, implementation::UnpackOptions>
    {
    };
}
