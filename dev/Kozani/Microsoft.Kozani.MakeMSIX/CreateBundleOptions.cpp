// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateBundleOptions.h"
#include "CreateBundleOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    bool CreateBundleOptions::OverwriteOutputFileIfExists()
    {
        return mOverwriteOutputFileIfExists;
    }
    void CreateBundleOptions::OverwriteOutputFileIfExists(bool value)
    {
        mOverwriteOutputFileIfExists = value;
    }
    bool CreateBundleOptions::FlatBundle()
    {
        return mFlatBundle;
    }
    void CreateBundleOptions::FlatBundle(bool value)
    {
        mFlatBundle = value;
    }
    Windows::ApplicationModel::PackageVersion CreateBundleOptions::Version()
    {
        return mVersion;
    }
    void CreateBundleOptions::Version(Windows::ApplicationModel::PackageVersion value)
    {
        mVersion = value;
    }
}
