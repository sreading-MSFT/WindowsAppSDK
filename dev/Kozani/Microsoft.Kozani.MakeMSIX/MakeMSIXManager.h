// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "MakeMSIXManager.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct MakeMSIXManager
    {
        MakeMSIXManager() = default;

        static Windows::Foundation::IAsyncAction Pack(hstring directoryPathToPack, PackOptions packOptions);
        static Windows::Foundation::IAsyncAction Unpack(hstring packageFilePathToUnpack, UnpackOptions unpackOptions);
        static Windows::Foundation::IAsyncAction Bundle(hstring directoryPathToBundle, BundleOptions bundleOptions);
        static Windows::Foundation::IAsyncAction Unbundle(hstring packageFilePathToUnbundle, UnbundleOptions unbundleOptions);
        static Windows::Foundation::IAsyncAction CreateKozaniPackage(hstring packageFilePathToConvert, CreateKozaniPackageOptions createKozaniPackageOptions);
        static Windows::Foundation::IAsyncAction CreateMountableImage(
            winrt::Windows::Foundation::Collections::IVector<hstring> packageFilePathsToAdd,
            CreateMountableImageOptions createMountableImageOptions);
        static Windows::Foundation::IAsyncOperation<PackageId> GetPackageIdentity(
            hstring packagePath);
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct MakeMSIXManager : MakeMSIXManagerT<MakeMSIXManager, implementation::MakeMSIXManager, static_lifetime>
    {
    };
}
