// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateMountableImageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions>
    {
        CreateMountableImageOptions() = default;

        hstring ImageFilePath();
        void ImageFilePath(hstring);
        hstring PackageRootDirectoryInImage();
        void PackageRootDirectoryInImage(hstring);
        bool OverwriteFiles();
        void OverwriteFiles(bool);

        hstring mImageFilePath{};
        hstring mPackageRootDirectoryInImage{};
        bool mOverwriteFiles{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions, implementation::CreateMountableImageOptions>
    {
    };
}
