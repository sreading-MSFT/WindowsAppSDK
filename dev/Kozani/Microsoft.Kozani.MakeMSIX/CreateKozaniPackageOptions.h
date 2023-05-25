// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateKozaniPackageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateKozaniPackageOptions : CreateKozaniPackageOptionsT<CreateKozaniPackageOptions>
    {
        CreateKozaniPackageOptions() = default;

        hstring PackageFilePath();
        void PackageFilePath(hstring);
        hstring PackagePublisher();
        void PackagePublisher(hstring);
        hstring PackageName();
        void PackageName(hstring);
        Windows::ApplicationModel::PackageVersion PackageVersion();
        void PackageVersion(Windows::ApplicationModel::PackageVersion);
        bool RemoveExtensions();
        void RemoveExtensions(bool);
        bool OverwriteFiles();
        void OverwriteFiles(bool);
        bool ValidateFiles();
        void ValidateFiles(bool);

        hstring mPackageFilePath{};
        hstring mPackagePublisher{};
        hstring mPackageName{};
        Windows::ApplicationModel::PackageVersion mPackageVersion{};
        bool mRemoveExtensions{};
        bool mOverwriteFiles{};
        bool mValidateFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateKozaniPackageOptions : CreateKozaniPackageOptionsT<CreateKozaniPackageOptions, implementation::CreateKozaniPackageOptions>
    {
    };
}
