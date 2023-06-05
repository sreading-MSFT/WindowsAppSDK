// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "PackageIdentity.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct PackageIdentity : PackageIdentityT<PackageIdentity>
    {
        PackageIdentity() = default;

        hstring Name();
        void Name(hstring);
        hstring FamilyName();
        void FamilyName(hstring);
        hstring FullName();
        void FullName(hstring);
        hstring Publisher();
        void Publisher(hstring);
        hstring ResourceId();
        void ResourceId(hstring);
        Windows::ApplicationModel::PackageVersion Version();
        void Version(Windows::ApplicationModel::PackageVersion);
        Windows::System::ProcessorArchitecture Architecture();
        void Architecture(Windows::System::ProcessorArchitecture);

        hstring mName{};
        hstring mFamilyName{};
        hstring mFullName{};
        hstring mPublisher{};
        hstring mResourceId{};
        Windows::ApplicationModel::PackageVersion mVersion{};
        Windows::System::ProcessorArchitecture mArchitecture{};
    };
}
