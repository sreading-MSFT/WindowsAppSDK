// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "pch.h"

UINT64 ConvertToQuadVersion(winrt::Windows::ApplicationModel::PackageVersion packageVersion);

winrt::Microsoft::Kozani::MakeMSIX::PackageIdentity GetPackageIdentityFromManifest(IAppxManifestPackageId* manifestPackageId);

winrt::Microsoft::Kozani::MakeMSIX::PackageInformation GetPackageInformationFromPackageManifest(IAppxManifestReader* manifestReader);

winrt::Microsoft::Kozani::MakeMSIX::PackageInformation GetPackageInformationFromBundleManifest(IAppxBundleManifestReader* manifestReader);

winrt::Windows::Foundation::IAsyncAction ExtractAppxFileToDirectory(IAppxFile* appxFile, winrt::Windows::Storage::StorageFolder unpackDestinationFolder);

winrt::Windows::Foundation::IAsyncAction ExtractAppxFilesToDirectory(IAppxFilesEnumerator* appxFilesEnumerator, winrt::Windows::Storage::StorageFolder unpackDestinationFolder);

HRESULT CreateAppxFactory(IAppxFactory** appxFactory);

HRESULT CreateAppxBundleFactory(IAppxBundleFactory** appxBundleFactory);
