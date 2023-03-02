// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "PackOptions.h"
#include "PackOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    hstring PackOptions::KozaniPackageFilePath()
    {
        return mKozaniPackageFilePath;
    }
    void PackOptions::KozaniPackageFilePath(hstring value)
    {
        mKozaniPackageFilePath = value;
    }
    bool PackOptions::CreateAsKozaniPackage()
    {
        return mCreateAsKozaniPackage;
    }
    void PackOptions::CreateAsKozaniPackage(bool value)
    {
        mCreateAsKozaniPackage = value;
    }
    void PackOptions::PackageFilePath(hstring value)
    {
        mPackageFilePath = value;
    }
    hstring PackOptions::PackageFilePath()
    {
        return mPackageFilePath;
    }
    bool PackOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    void PackOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
    bool PackOptions::ValidateFiles()
    {
        return mValidateFiles;
    }
    void PackOptions::ValidateFiles(bool value)
    {
        mValidateFiles = value;
    }
}
