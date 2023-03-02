// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "PackagingResult.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct PackagingResult : PackagingResultT<PackagingResult>
    {
        PackagingResult() = default;

        HRESULT ExtendedErrorCode();
        void ExtendedErrorCode(HRESULT);
        PackagingResultStatus Status();
        void Status(PackagingResultStatus);
        
        HRESULT mExtendedErrorCode{};
        PackagingResultStatus mPackagingResultStatus = PackagingResultStatus::Ok;
    };
}
