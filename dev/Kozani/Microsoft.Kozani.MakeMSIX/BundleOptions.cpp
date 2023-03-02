// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "BundleOptions.h"
#include "BundleOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void BundleOptions::BundleFilePath(hstring value)
    {
        mBundleFilePath = value;
    }
    hstring BundleOptions::BundleFilePath()
    {
        return mBundleFilePath;
    }
    bool BundleOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    void BundleOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
    bool BundleOptions::FlatBundle()
    {
        return mFlatBundle;
    }
    void BundleOptions::FlatBundle(bool value)
    {
        mFlatBundle = value;
    }
    Windows::ApplicationModel::PackageVersion BundleOptions::BundleVersion()
    {
        return mBundleVersion;
    }
    void BundleOptions::BundleVersion(Windows::ApplicationModel::PackageVersion value)
    {
        mBundleVersion = value;
    }
}
