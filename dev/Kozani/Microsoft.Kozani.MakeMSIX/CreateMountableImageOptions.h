// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateMountableImageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions>
    {
        CreateMountableImageOptions() = default;

        hstring PackageRootDirectoryInImage();
        void PackageRootDirectoryInImage(hstring);
        bool OverwriteOutputFileIfExists();
        void OverwriteOutputFileIfExists(bool);

        hstring mPackageRootDirectoryInImage{};
        bool mOverwriteOutputFileIfExists{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions, implementation::CreateMountableImageOptions>
    {
    };
}
