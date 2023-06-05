// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "pch.h"

HRESULT CreateKozaniResourcePriFromManifestAndResourcePri(
    _In_ std::filesystem::path manifestFilePath,
    _In_ std::filesystem::path resourcesPriFilePath,
    _In_ std::wstring outputDirectoryPath,
    std::set<std::wstring>& requiredFilesRelativePaths);
