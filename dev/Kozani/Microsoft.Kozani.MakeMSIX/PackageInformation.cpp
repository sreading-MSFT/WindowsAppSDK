// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "PackageInformation.h"
#include "PackageInformation.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    PackageIdentity PackageInformation::Identity()
    {
        return mPackageIdentity;
    }

    Windows::Foundation::Collections::IVectorView<hstring> PackageInformation::Languages()
    {
        return mLanguages.GetView();
    }

    Windows::Foundation::Collections::IVectorView<unsigned int> PackageInformation::ScaleFactors()
    {
        return mScaleFactors.GetView();
    }

    void PackageInformation::SetIdentity(winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity identity)
    {
        mPackageIdentity = identity;
    }

    void PackageInformation::AddLanguage(hstring language)
    {
        mLanguages.Append(language);
    }

    void PackageInformation::AddScale(unsigned int scale)
    {
        mScaleFactors.Append(scale);
    }
}
