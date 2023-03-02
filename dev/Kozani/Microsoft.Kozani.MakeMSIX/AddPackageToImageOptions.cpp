// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "AddPackageToImageOptions.h"
#include "AddPackageToImageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    hstring AddPackageToImageOptions::PackageRootDirectoryInImage()
    {
        return mPackageRootDirectoryInImage;
    }
    void AddPackageToImageOptions::PackageRootDirectoryInImage(hstring value)
    {
        mPackageRootDirectoryInImage = value;
    }
}
