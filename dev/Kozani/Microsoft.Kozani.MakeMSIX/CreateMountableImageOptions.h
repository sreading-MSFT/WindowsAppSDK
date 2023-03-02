// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "CreateMountableImageOptions.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions>
    {
        CreateMountableImageOptions() = default;

        UINT32 FixedImageSizeMegabytes();
        void FixedImageSizeMegabytes(UINT32);
        bool DynamicallyExpandable();
        void DynamicallyExpandable(bool);
        UINT32 MaximumExpandableImageSizeMegabytes();
        void MaximumExpandableImageSizeMegabytes(UINT32);

        UINT32 mFixedImageSizeMegabytes{};
        UINT32 mMaximumExpandableImageSizeMegabytes{};
        bool mDynamicallyExpandable{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct CreateMountableImageOptions : CreateMountableImageOptionsT<CreateMountableImageOptions, implementation::CreateMountableImageOptions>
    {
    };
}
