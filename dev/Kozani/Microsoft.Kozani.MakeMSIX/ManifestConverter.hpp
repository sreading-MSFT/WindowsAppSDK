// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#pragma once
#include "pch.h"
#include <string>
#include <set>
#include <winrt/windows.data.xml.dom.h>
#include <winrt/windows.storage.h>


bool CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right);

bool CaseInsensitiveStartsWith(const std::wstring& string, const std::wstring& prefix);

HRESULT GetDocElementFromFile(std::wstring filePath, winrt::Windows::Data::Xml::Dom::XmlDocument& docElement);

HRESULT FindNodeTextValue(winrt::Windows::Data::Xml::Dom::XmlDocument doc, std::wstring xPathQuery, bool failIfNotFound, _Inout_ std::wstring& nodeText);

HRESULT ModifyManifest(
    _In_ std::wstring manifestFilePath,
    _In_ std::wstring kozaniManifestOutputDirPath);
