// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "UnpackOptions.h"
#include "UnpackOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    hstring UnpackOptions::UnpackedPackageRootDirectory()
    {
        return mUnpackedPackageRootDirectory;
    }
    void UnpackOptions::UnpackedPackageRootDirectory(hstring value)
    {
        mUnpackedPackageRootDirectory = value;
    }
    bool UnpackOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    void UnpackOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
}
