// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateMountableImageOptions.h"
#include "CreateMountableImageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void CreateMountableImageOptions::ImageFilePath(hstring value)
    {
        mImageFilePath = value;
    }
    hstring CreateMountableImageOptions::ImageFilePath()
    {
        return mImageFilePath;
    }
    void CreateMountableImageOptions::PackageRootDirectoryInImage(hstring value)
    {
        mPackageRootDirectoryInImage = value;
    }
    hstring CreateMountableImageOptions::PackageRootDirectoryInImage()
    {
        return mPackageRootDirectoryInImage;
    }
    bool CreateMountableImageOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    void CreateMountableImageOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
}
