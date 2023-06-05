// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateMountableImageOptions.h"
#include "CreateMountableImageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void CreateMountableImageOptions::PackageRootDirectoryInImage(hstring value)
    {
        mPackageRootDirectoryInImage = value;
    }
    hstring CreateMountableImageOptions::PackageRootDirectoryInImage()
    {
        return mPackageRootDirectoryInImage;
    }
    bool CreateMountableImageOptions::OverwriteOutputFileIfExists()
    {
        return mOverwriteOutputFileIfExists;
    }
    void CreateMountableImageOptions::OverwriteOutputFileIfExists(bool value)
    {
        mOverwriteOutputFileIfExists = value;
    }
}
