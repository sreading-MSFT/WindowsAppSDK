// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "PackagingResult.h"
#include "PackagingResult.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    HRESULT PackagingResult::ExtendedErrorCode()
    {
        return mExtendedErrorCode;
    }
    void PackagingResult::ExtendedErrorCode(HRESULT value)
    {
        mExtendedErrorCode = value;
    }
    PackagingResultStatus PackagingResult::Status()
    {
        return mPackagingResultStatus;
    }
    void PackagingResult::Status(PackagingResultStatus value)
    {
        mPackagingResultStatus = value;
    }
}
