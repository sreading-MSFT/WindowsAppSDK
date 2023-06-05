// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreatePackageOptions.h"
#include "CreatePackageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void CreatePackageOptions::Publisher(hstring value)
    {
        mPublisher = value;
    }
    hstring CreatePackageOptions::Publisher()
    {
        return mPublisher;
    }
    void CreatePackageOptions::Name(hstring value)
    {
        mName = value;
    }
    hstring CreatePackageOptions::Name()
    {
        return mName;
    }
    Windows::ApplicationModel::PackageVersion CreatePackageOptions::Version()
    {
        return mVersion;
    }
    void CreatePackageOptions::Version(Windows::ApplicationModel::PackageVersion value)
    {
        mVersion = value;
    }
    bool CreatePackageOptions::OverwriteOutputFileIfExists()
    {
        return mOverwriteOutputFileIfExists;
    }
    void CreatePackageOptions::OverwriteOutputFileIfExists(bool value)
    {
        mOverwriteOutputFileIfExists = value;
    }
    bool CreatePackageOptions::ValidateFiles()
    {
        return mValidateFiles;
    }
    void CreatePackageOptions::ValidateFiles(bool value)
    {
        mValidateFiles = value;
    }
}
