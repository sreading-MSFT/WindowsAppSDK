// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "UnbundleOptions.h"
#include "UnbundleOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    hstring UnbundleOptions::UnbundledPackageRootDirectory()
    {
        return mUnbundledPackageRootDirectory;
    }
    void UnbundleOptions::UnbundledPackageRootDirectory(hstring value)
    {
        mUnbundledPackageRootDirectory = value;
    }
    bool UnbundleOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    void UnbundleOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
}
