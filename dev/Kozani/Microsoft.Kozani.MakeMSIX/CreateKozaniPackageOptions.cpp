// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateKozaniPackageOptions.h"
#include "CreateKozaniPackageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void CreateKozaniPackageOptions::PackageFilePath(hstring value)
    {
        mPackageFilePath = value;
    }
    hstring CreateKozaniPackageOptions::PackageFilePath()
    {
        return mPackageFilePath;
    }
    void CreateKozaniPackageOptions::PackagePublisher(hstring value)
    {
        mPackagePublisher = value;
    }
    hstring CreateKozaniPackageOptions::PackagePublisher()
    {
        return mPackagePublisher;
    }
    void CreateKozaniPackageOptions::PackageName(hstring value)
    {
        mPackageName = value;
    }
    hstring CreateKozaniPackageOptions::PackageName()
    {
        return mPackageName;
    }
    void CreateKozaniPackageOptions::OverwriteFiles(bool value)
    {
        mOverwriteFiles = value;
    }
    bool CreateKozaniPackageOptions::OverwriteFiles()
    {
        return mOverwriteFiles;
    }
    bool CreateKozaniPackageOptions::ValidateFiles()
    {
        return mValidateFiles;
    }
    void CreateKozaniPackageOptions::ValidateFiles(bool value)
    {
        mValidateFiles = value;
    }
}
