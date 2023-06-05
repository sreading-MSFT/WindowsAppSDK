// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateKozaniPackageOptions.h"
#include "CreateKozaniPackageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    void CreateKozaniPackageOptions::Publisher(hstring value)
    {
        mPublisher = value;
    }
    hstring CreateKozaniPackageOptions::Publisher()
    {
        return mPublisher;
    }
    void CreateKozaniPackageOptions::Name(hstring value)
    {
        mName = value;
    }
    hstring CreateKozaniPackageOptions::Name()
    {
        return mName;
    }
    Windows::ApplicationModel::PackageVersion CreateKozaniPackageOptions::Version()
    {
        return mVersion;
    }
    void CreateKozaniPackageOptions::Version(Windows::ApplicationModel::PackageVersion value)
    {
        mVersion = value;
    }
    void CreateKozaniPackageOptions::RemoveExtensions(bool value)
    {
        mRemoveExtensions = value;
    }
    bool CreateKozaniPackageOptions::RemoveExtensions()
    {
        return mRemoveExtensions;
    }
    Windows::Foundation::Collections::IVector<hstring> CreateKozaniPackageOptions::Languages()
    {
        return mLanguages;
    }
    Windows::Foundation::Collections::IVector<unsigned int> CreateKozaniPackageOptions::ScaleFactors()
    {
        return mScaleFactors;
    }
    void CreateKozaniPackageOptions::OverwriteOutputFileIfExists(bool value)
    {
        mOverwriteOutputFileIfExists = value;
    }
    bool CreateKozaniPackageOptions::OverwriteOutputFileIfExists()
    {
        return mOverwriteOutputFileIfExists;
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
