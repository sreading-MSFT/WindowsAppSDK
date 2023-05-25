// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "pch.h"
#include <string>

HRESULT CreateKozaniPackageLayout(
    _In_ std::wstring sourceDirectoryPath,
    _In_ std::wstring outputDirectoryPath);
