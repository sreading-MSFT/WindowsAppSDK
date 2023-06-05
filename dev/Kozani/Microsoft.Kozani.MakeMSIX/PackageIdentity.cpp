// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "PackageIdentity.h"
#include "PackageIdentity.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    hstring PackageIdentity::Name()
    {
        return mName;
    }
    void PackageIdentity::Name(hstring value)
    {
        mName = value;
    }
    hstring PackageIdentity::FamilyName()
    {
        return mFamilyName;
    }
    void PackageIdentity::FamilyName(hstring value)
    {
        mFamilyName = value;
    }
    void PackageIdentity::FullName(hstring value)
    {
        mFullName = value;
    }
    hstring PackageIdentity::FullName()
    {
        return mFullName;
    }
    void PackageIdentity::Publisher(hstring value)
    {
        mPublisher = value;
    }
    hstring PackageIdentity::Publisher()
    {
        return mPublisher;
    }
    void PackageIdentity::ResourceId(hstring value)
    {
        mResourceId = value;
    }
    hstring PackageIdentity::ResourceId()
    {
        return mResourceId;
    }
    Windows::ApplicationModel::PackageVersion PackageIdentity::Version()
    {
        return mVersion;
    }
    void PackageIdentity::Version(Windows::ApplicationModel::PackageVersion value)
    {
        mVersion = value;
    }
    Windows::System::ProcessorArchitecture PackageIdentity::Architecture()
    {
        return mArchitecture;
    }
    void PackageIdentity::Architecture(Windows::System::ProcessorArchitecture value)
    {
        mArchitecture = value;
    }
}
