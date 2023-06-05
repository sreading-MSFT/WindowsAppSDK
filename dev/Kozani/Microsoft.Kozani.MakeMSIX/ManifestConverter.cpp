// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "Constants.hpp"
#include "CreateKozaniPackageProvider.hpp"
#include <set>
#include <filesystem>
#include <winrt/windows.data.xml.dom.h>
#include <winrt/windows.storage.h>
#include <MrmResourceIndexer.h>

using namespace std;

bool CaseInsensitiveEquals(const std::wstring& left, const std::wstring& right)
{
    return (_wcsicmp(left.c_str(), right.c_str()) == 0);
}

bool CaseInsensitiveStartsWith(const std::wstring& string, const std::wstring& prefix)
{
    if (string.length() < prefix.length())
    {
        return false;
    }
    return CaseInsensitiveEquals(string.substr(0, prefix.length()), prefix);
}

HRESULT GetDocElementFromFile(std::wstring filePath, winrt::Windows::Data::Xml::Dom::XmlDocument& docElement)
{
    using namespace winrt::Windows::Storage;
    StorageFile file = StorageFile::GetFileFromPathAsync(filePath).get();
    docElement = winrt::Windows::Data::Xml::Dom::XmlDocument::LoadFromFileAsync(file).get();
    return S_OK;
}

HRESULT FindNodeTextValue(winrt::Windows::Data::Xml::Dom::XmlDocument doc, std::wstring xPathQuery, bool failIfNotFound, _Inout_ std::wstring& nodeText)
{
    winrt::Windows::Data::Xml::Dom::IXmlNode node = doc.SelectSingleNode(xPathQuery.c_str());
    if(node)
    {
        nodeText = node.InnerText();
    }
    if (failIfNotFound && nodeText.empty())
    {
        return HRESULT_FROM_WIN32(ERROR_XML_PARSE_ERROR);
    }
    return S_OK;
}

std::set<std::wstring> GetAllowedPackageElementChildren()
{
    std::set<std::wstring> allowedChildSet{
        L"Identity",
        L"Properties",
        L"Dependencies",
        L"Resources",
        L"Applications",
        L"Capabilities",
    };
    return allowedChildSet;
}

HRESULT ModifyManifest(
    _In_ std::wstring manifestFilePath,
    _In_ std::wstring kozaniManifestOutputDirPath)
{
    winrt::Windows::Data::Xml::Dom::XmlDocument manifestDocElement;
    RETURN_IF_FAILED(GetDocElementFromFile(manifestFilePath, manifestDocElement));

    winrt::Windows::Data::Xml::Dom::IXmlNode packageNode = manifestDocElement.SelectSingleNode(L"/*[local-name()='Package']");

    // Add uap10 namespace if needed.
    if (packageNode.Attributes().GetNamedItem(L"xmlns:uap10") == nullptr)
    {
        winrt::Windows::Data::Xml::Dom::IXmlNode namespaceNode = manifestDocElement.CreateAttributeNS(winrt::box_value(manifestXmlNamespace), L"xmlns:uap10");
        namespaceNode.InnerText(manifestUap10Namespace);
        packageNode.Attributes().SetNamedItem(namespaceNode);
    }

    // Remove any framework dependencies.
    winrt::Windows::Data::Xml::Dom::IXmlNode dependencyNode = manifestDocElement.SelectSingleNode(manifestDependenciesQuery.c_str());
    winrt::Windows::Data::Xml::Dom::XmlNodeList packageDependencies = dependencyNode.SelectNodes(manifestDependenciesPackageDependenciesQuery.c_str());
    for (winrt::Windows::Data::Xml::Dom::IXmlNode packageDependencyNode : packageDependencies)
    {
        dependencyNode.RemoveChild(packageDependencyNode);
    }

    winrt::Windows::Data::Xml::Dom::XmlNodeList targetDeviceFamilyNodes = dependencyNode.SelectNodes(manifestTargetDeviceFamilyQuery.c_str());
    for (winrt::Windows::Data::Xml::Dom::IXmlNode targetDeviceFamilyNode : targetDeviceFamilyNodes)
    {
        // TODO: MaxVersionTested needs to be confirmed somewhere. Probably better if up front?
        winrt::Windows::Data::Xml::Dom::IXmlNode minVersion = targetDeviceFamilyNode.Attributes().RemoveNamedItem(L"MinVersion");
        minVersion.InnerText(L"10.0.19003.0");
        targetDeviceFamilyNode.Attributes().SetNamedItem(minVersion);
    }

    // Add HostRuntime dependency.
    winrt::Windows::Data::Xml::Dom::IXmlNode hostRuntimeDepNode = manifestDocElement.CreateElementNS(winrt::box_value(manifestUap10Namespace), L"HostRuntimeDependency");
    hostRuntimeDepNode.Prefix(winrt::box_value(L"uap10"));
    winrt::Windows::Data::Xml::Dom::IXmlNode hostRuntimeName = manifestDocElement.CreateAttribute(L"Name");
    hostRuntimeName.InnerText(manifestKozaniHostRuntime);
    hostRuntimeDepNode.Attributes().SetNamedItem(hostRuntimeName);
    winrt::Windows::Data::Xml::Dom::IXmlNode hostRuntimePublisher = manifestDocElement.CreateAttribute(L"Publisher");
    hostRuntimePublisher.InnerText(L"CN=Microsoft Windows, O=Microsoft Corporation, L=Redmond, S=Washington, C=US");
    hostRuntimeDepNode.Attributes().SetNamedItem(hostRuntimePublisher);
    winrt::Windows::Data::Xml::Dom::IXmlNode hostRuntimeVersion = manifestDocElement.CreateAttribute(L"MinVersion");
    hostRuntimeVersion.InnerText(L"1.0.0.0");
    hostRuntimeDepNode.Attributes().SetNamedItem(hostRuntimeVersion);
    dependencyNode.AppendChild(hostRuntimeDepNode);

    // Remove <Package> child elements that aren't needed.
    std::set<std::wstring> allowedChildren = GetAllowedPackageElementChildren();
    for (auto child : packageNode.ChildNodes())
    {
        if (child.NodeType() == winrt::Windows::Data::Xml::Dom::NodeType::ElementNode)
        {
            if (!allowedChildren.contains(child.NodeName().c_str()))
            {
                packageNode.RemoveChild(child);
            }
        }
    }

    // Calculate PackageFamilyName for arguments.
    std::wstring packageName = L"";
    RETURN_IF_FAILED(FindNodeTextValue(manifestDocElement, packageIdentityNameQuery, true, packageName));

    std::wstring publisher = L"";
    RETURN_IF_FAILED(FindNodeTextValue(manifestDocElement, packageIdentityPublisherQuery, true, publisher));

    WCHAR packageFamilyName[PACKAGE_FAMILY_NAME_MAX_LENGTH + 1];
    UINT32 packageFamilyNameLength = _ARRAYSIZE(packageFamilyName);

    PACKAGE_ID packageId;
    packageId.name = const_cast<LPWSTR>(packageName.c_str());
    packageId.publisher = const_cast<LPWSTR>(publisher.c_str());
    packageId.resourceId = NULL;
    packageId.publisherId = NULL;
    RETURN_IF_FAILED(HRESULT_FROM_WIN32(PackageFamilyNameFromId(&packageId, &packageFamilyNameLength, packageFamilyName)));

    std::wstring aumidPrefix{ packageFamilyName };
    aumidPrefix += L"!";

    // Fix application entries to use HostRuntime
    winrt::Windows::Data::Xml::Dom::XmlNodeList applicationNodes = manifestDocElement.SelectNodes(manifestApplicationQuery.c_str());
    for (winrt::Windows::Data::Xml::Dom::IXmlNode applicationNode : applicationNodes)
    {
        winrt::Windows::Data::Xml::Dom::IXmlNode idNode = applicationNode.Attributes().GetNamedItem(L"Id");
        std::wstring idText{ idNode.InnerText() };

        std::list<std::wstring> attributeNames;
        for (winrt::Windows::Data::Xml::Dom::IXmlNode attribute : applicationNode.Attributes())
        {
            attributeNames.push_back(attribute.NodeName().c_str());
        }
        for (std::wstring attribute : attributeNames)
        {
            applicationNode.Attributes().RemoveNamedItem(attribute);
        }

        applicationNode.Attributes().SetNamedItem(idNode);

        winrt::Windows::Data::Xml::Dom::IXmlNode hostIdNode = manifestDocElement.CreateAttributeNS(winrt::box_value(manifestUap10Namespace), L"HostId");
        hostIdNode.InnerText(manifestKozaniHostRuntime);
        applicationNode.Attributes().SetNamedItem(hostIdNode);

        winrt::Windows::Data::Xml::Dom::IXmlNode parametersNode = manifestDocElement.CreateAttributeNS(winrt::box_value(manifestUap10Namespace), L"Parameters");
        std::wstring parametersText{ L"-aumid " };
        parametersText += aumidPrefix + idNode.InnerText();
        parametersNode.InnerText(parametersText);
        applicationNode.Attributes().SetNamedItem(parametersNode);
    }

    // Write manifest
    using namespace winrt::Windows::Storage;
    StorageFolder storageFolder{ StorageFolder::GetFolderFromPathAsync(kozaniManifestOutputDirPath).get() };
    auto outputManifestFile{ storageFolder.CreateFileAsync(manifestFileName, CreationCollisionOption::ReplaceExisting).get() };
    manifestDocElement.SaveToFileAsync(outputManifestFile).get();

    return S_OK;
}
