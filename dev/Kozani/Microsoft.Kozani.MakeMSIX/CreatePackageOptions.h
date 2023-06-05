// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreatePackageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreatePackageOptions : CreatePackageOptionsT<CreatePackageOptions>
    {
        CreatePackageOptions() = default;

        void Name(hstring value);
        hstring Name();
        void Publisher(hstring value);
        hstring Publisher();
        Windows::ApplicationModel::PackageVersion Version();
        void Version(Windows::ApplicationModel::PackageVersion value);
        bool OverwriteOutputFileIfExists();
        void OverwriteOutputFileIfExists(bool);
        bool ValidateFiles();
        void ValidateFiles(bool);

    private:
        hstring mName{};
        hstring mPublisher{};
        Windows::ApplicationModel::PackageVersion mVersion{};
        bool mOverwriteOutputFileIfExists{};
        bool mValidateFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreatePackageOptions : CreatePackageOptionsT<CreatePackageOptions, implementation::CreatePackageOptions>
    {
    };
}
