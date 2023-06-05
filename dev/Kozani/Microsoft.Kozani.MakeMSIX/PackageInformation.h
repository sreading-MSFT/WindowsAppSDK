// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "PackageInformation.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct PackageInformation : PackageInformationT<PackageInformation>
    {
        PackageInformation() = default;

        winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity Identity();
        Windows::Foundation::Collections::IVectorView<hstring> Languages();
        Windows::Foundation::Collections::IVectorView<unsigned int> ScaleFactors();

        void SetIdentity(winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity identity);
        void AddLanguage(hstring language);
        void AddScale(unsigned int scale);

    private:
        winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity mPackageIdentity{ nullptr };
        Windows::Foundation::Collections::IVector<hstring> mLanguages{ winrt::single_threaded_vector<hstring>() };
        Windows::Foundation::Collections::IVector<unsigned int> mScaleFactors{ winrt::single_threaded_vector<unsigned int>() };
    };
}
