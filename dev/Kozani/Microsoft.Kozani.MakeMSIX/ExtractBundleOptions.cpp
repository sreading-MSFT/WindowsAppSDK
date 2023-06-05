// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "ExtractBundleOptions.h"
#include "ExtractBundleOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    bool ExtractBundleOptions::OverwriteOutputFilesIfExists()
    {
        return mOverwriteOutputFilesIfExists;
    }
    void ExtractBundleOptions::OverwriteOutputFilesIfExists(bool value)
    {
        mOverwriteOutputFilesIfExists = value;
    }
}
