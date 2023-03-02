// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "PackOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct PackOptions : PackOptionsT<PackOptions>
    {
        PackOptions() = default;

        hstring KozaniPackageFilePath();
        void KozaniPackageFilePath(hstring);
        bool CreateAsKozaniPackage();
        void CreateAsKozaniPackage(bool);
        hstring PackageFilePath();
        void PackageFilePath(hstring);
        bool OverwriteFiles();
        void OverwriteFiles(bool);
        bool ValidateFiles();
        void ValidateFiles(bool);

        hstring mKozaniPackageFilePath{};
        hstring mPackageFilePath{};
        bool mCreateAsKozaniPackage{};
        bool mOverwriteFiles{};
        bool mValidateFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct PackOptions : PackOptionsT<PackOptions, implementation::PackOptions>
    {
    };
}
