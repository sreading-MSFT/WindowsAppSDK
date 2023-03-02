// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "AddPackageToImageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct AddPackageToImageOptions : AddPackageToImageOptionsT<AddPackageToImageOptions>
    {
        AddPackageToImageOptions() = default;

        hstring PackageRootDirectoryInImage();
        void PackageRootDirectoryInImage(hstring);

        hstring mPackageRootDirectoryInImage{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct AddPackageToImageOptions : AddPackageToImageOptionsT<AddPackageToImageOptions, implementation::AddPackageToImageOptions>
    {
    };
}
