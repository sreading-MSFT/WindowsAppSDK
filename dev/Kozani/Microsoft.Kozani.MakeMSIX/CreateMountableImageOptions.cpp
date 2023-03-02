// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "CreateMountableImageOptions.h"
#include "CreateMountableImageOptions.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    UINT32 CreateMountableImageOptions::FixedImageSizeMegabytes()
    {
        return mFixedImageSizeMegabytes;
    }
    void CreateMountableImageOptions::FixedImageSizeMegabytes(UINT32 value)
    {
        mFixedImageSizeMegabytes = value;
    }
    bool CreateMountableImageOptions::DynamicallyExpandable()
    {
        return mDynamicallyExpandable;
    }
    void CreateMountableImageOptions::DynamicallyExpandable(bool value)
    {
        mDynamicallyExpandable = value;
    }
    UINT32 CreateMountableImageOptions::MaximumExpandableImageSizeMegabytes()
    {
        return mMaximumExpandableImageSizeMegabytes;
    }
    void CreateMountableImageOptions::MaximumExpandableImageSizeMegabytes(UINT32 value)
    {
        mMaximumExpandableImageSizeMegabytes = value;
    }
}
