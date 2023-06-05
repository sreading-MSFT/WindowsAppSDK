// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateKozaniPackageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateKozaniPackageOptions : CreateKozaniPackageOptionsT<CreateKozaniPackageOptions>
    {
        CreateKozaniPackageOptions() = default;

        void Name(hstring value);
        hstring Name();
        void Publisher(hstring value);
        hstring Publisher();
        Windows::ApplicationModel::PackageVersion Version();
        void Version(Windows::ApplicationModel::PackageVersion value);
        bool RemoveExtensions();
        void RemoveExtensions(bool);
        Windows::Foundation::Collections::IVector<hstring> Languages();
        Windows::Foundation::Collections::IVector<unsigned int> ScaleFactors();
        bool OverwriteOutputFileIfExists();
        void OverwriteOutputFileIfExists(bool);
        bool ValidateFiles();
        void ValidateFiles(bool);

    private:
        hstring mName{};
        hstring mPublisher{};
        Windows::ApplicationModel::PackageVersion mVersion{};
        Windows::Foundation::Collections::IVector<hstring> mLanguages{ winrt::single_threaded_vector<hstring>() };
        Windows::Foundation::Collections::IVector<unsigned int> mScaleFactors{ winrt::single_threaded_vector<unsigned int>() };
        bool mRemoveExtensions{};
        bool mOverwriteOutputFileIfExists{};
        bool mValidateFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateKozaniPackageOptions : CreateKozaniPackageOptionsT<CreateKozaniPackageOptions, implementation::CreateKozaniPackageOptions>
    {
    };
}
