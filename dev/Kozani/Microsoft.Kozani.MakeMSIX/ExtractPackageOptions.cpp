// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "ExtractPackageOptions.h"
#include "ExtractPackageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    bool ExtractPackageOptions::OverwriteOutputFilesIfExists()
    {
        return mOverwriteOutputFilesIfExists;
    }
    void ExtractPackageOptions::OverwriteOutputFilesIfExists(bool value)
    {
        mOverwriteOutputFilesIfExists = value;
    }
}
