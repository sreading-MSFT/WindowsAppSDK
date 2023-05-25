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

std::wstring GetResourceStringPrefix(std::wstring packageName)
{
    return msResourceScheme + L"//" + packageName + L"/" + msResourceResourceUriPath + L"/";
}
std::wstring GetResourceFilePrefix(std::wstring packageName)
{
    return msResourceScheme + L"//" + packageName + L"/" + msResourceFileUriPath + L"/";
}
std::wstring ConvertToForwardSlashes(std::wstring uriString)
{
    std::wstring stringWithForwardSlashes = uriString;
    std::replace(stringWithForwardSlashes.begin(), stringWithForwardSlashes.end(), '\\', '/');
    return stringWithForwardSlashes;
}
std::wstring ConvertToBackwardSlashes(std::wstring uriString)
{
    std::wstring stringWithBackwardSlashes = uriString;
    std::replace(stringWithBackwardSlashes.begin(), stringWithBackwardSlashes.end(), '/', '\\');
    return stringWithBackwardSlashes;
}

HRESULT GetResourceQualifierStringFromQualifiersList(winrt::Windows::Data::Xml::Dom::IXmlNodeList qualifierNodes, bool defaultQualifiersOnly, std::wstring& resourceQualifier)
{
    for (winrt::Windows::Data::Xml::Dom::IXmlNode qualifierNode : qualifierNodes)
    {
        winrt::Windows::Data::Xml::Dom::IXmlNode scoreAsDefaultNode = qualifierNode.Attributes().GetNamedItem(priScoreAsDefaultAttribute);
        if (priDefaultQualifierScore.compare(scoreAsDefaultNode.InnerText()) != 0 && defaultQualifiersOnly)
        {
            continue;
        }

        winrt::Windows::Data::Xml::Dom::IXmlNode resourceQualifierName = qualifierNode.Attributes().GetNamedItem(priNameAttribute);
        winrt::Windows::Data::Xml::Dom::IXmlNode resourceQualifierValue = qualifierNode.Attributes().GetNamedItem(priValueAttribute);
        if (!resourceQualifier.empty())
        {
            resourceQualifier += L"_";
        }
        resourceQualifier += resourceQualifierName.InnerText() + L"-" + resourceQualifierValue.InnerText();
    }
    return S_OK;
}

HRESULT GetDocElementFromFile(std::wstring filePath, winrt::Windows::Data::Xml::Dom::XmlDocument& docElement)
{
    using namespace winrt::Windows::Storage;
    StorageFile file = StorageFile::GetFileFromPathAsync(filePath).get();
    docElement = winrt::Windows::Data::Xml::Dom::XmlDocument::LoadFromFileAsync(file).get();
    return S_OK;
}

// Create a version of the resource pri file that only contains resources in the namedResources set, which is the set of resources required by the manifest.
HRESULT CreateKozaniResourcePriWithNamedResources(
    std::filesystem::path resourcePriFilePath,
    std::wstring outputDirectoryPath,
    std::wstring packageName,
    std::set<std::wstring>& namedResources,
    std::set<std::wstring>& requiredFilesRelativePaths)
{
    std::filesystem::path tempOutputDumpedXmlFilePath{ outputDirectoryPath };
    tempOutputDumpedXmlFilePath /= L"resourcesPriDumped.xml";
    RETURN_IF_FAILED(MrmDumpPriFile(
        resourcePriFilePath.c_str(),
        resourcePriFilePath.c_str(),
        MrmDumpType_Detailed,
        tempOutputDumpedXmlFilePath.c_str()));

    auto scopeExitToCleanDumpedXml{ wil::scope_exit([&]()
    {
        // remove the dumped xml, which is no longer needed.
        std::filesystem::remove(tempOutputDumpedXmlFilePath);
    }
    ) };

    winrt::Windows::Data::Xml::Dom::XmlDocument resourcePriDocElement;
    RETURN_IF_FAILED(GetDocElementFromFile(tempOutputDumpedXmlFilePath, resourcePriDocElement));

    std::wstring defaultQualifiers;
    winrt::Windows::Data::Xml::Dom::XmlNodeList defaultQualifierNodes = resourcePriDocElement.SelectNodes(priDefaultQualifiersQuery.c_str());
    RETURN_IF_FAILED(GetResourceQualifierStringFromQualifiersList(defaultQualifierNodes, true, defaultQualifiers));

    // MrmCreateResourceIndexer is documented as taking package family name, but when dumping resource pris from existing packages and then recreating them,
    // it only matches when package name is used instead.
    MrmResourceIndexerHandle indexer;
    RETURN_IF_FAILED(MrmCreateResourceIndexer(
        packageName.c_str(),
        outputDirectoryPath.c_str(),
        MrmPlatformVersion_Windows10_0_0_5,
        defaultQualifiers.c_str(),
        &indexer));

    auto scopeExitToCleanIndexer{ wil::scope_exit([&]()
    {
        ::MrmDestroyIndexerAndMessages(indexer);
    }
    ) };

    winrt::Windows::Data::Xml::Dom::XmlNodeList namedResourceNodes = resourcePriDocElement.SelectNodes(namedResourceQuery.c_str());
    for (winrt::Windows::Data::Xml::Dom::IXmlNode namedResourceNode : namedResourceNodes)
    {
        winrt::Windows::Data::Xml::Dom::IXmlNode uri = namedResourceNode.Attributes().GetNamedItem(priUriAttribute);
        std::wstring resourceUri{ uri.InnerText() };
        if (!namedResources.contains(resourceUri))
        {
            continue;
        }

        winrt::Windows::Data::Xml::Dom::XmlNodeList candidateNodes = namedResourceNode.SelectNodes(priCandidateQuery.c_str());
        for (winrt::Windows::Data::Xml::Dom::IXmlNode candidateNode : candidateNodes)
        {
            winrt::Windows::Data::Xml::Dom::IXmlNode candidateTypeNode = candidateNode.Attributes().GetNamedItem(priTypeAttribute);
            std::wstring candidateType{ candidateTypeNode.InnerText() };
            if (candidateType.compare(priPathCandidateTypeName) != 0 &&
                candidateType.compare(priStringCandidateTypeName) != 0)
            {
                // Can't reindex embedded or unknown data
                continue;
            }
            // Resource has been found, no need to keep looking.
            namedResources.erase(resourceUri);

            // Get the resource qualifiers. Example string: "language-en-US_scale-200"
            std::wstring resourceQualifiers{};
            winrt::Windows::Data::Xml::Dom::XmlNodeList qualifierNodes = candidateNode.SelectNodes(priQualifiersQuery.c_str());
            RETURN_IF_FAILED(GetResourceQualifierStringFromQualifiersList(qualifierNodes, false, resourceQualifiers));

            // Get the resolved value.
            // Example: if the resourceQualifier string is "language-en-US" the resolvedResourceValue might be "Assets\EnglishLogo.png".
            // If the resourceQualifier string is "language-fr-FR" the resolvedResourceValue might be "Assets\FrenchLogo.png".
            winrt::Windows::Data::Xml::Dom::IXmlNode resolvedValueNode = candidateNode.SelectSingleNode(priResolvedValueQuery.c_str());
            std::wstring resolvedResourceValue{ resolvedValueNode.InnerText() };
                
            if (candidateType.compare(priPathCandidateTypeName) == 0)
            {
                // Remove the unresolved path used by the manifest.
                // The unresolved path is never used and the file, if it exists, does not need to be copied.
                std::wstring resourceUriUnresolvedRelativePath = ConvertToBackwardSlashes(resourceUri.substr(GetResourceFilePrefix(packageName).length()));
                requiredFilesRelativePaths.erase(resourceUriUnresolvedRelativePath);

                // Resource has resolved to a file in the package. (It's possible this is the same value as the unresolved path)
                requiredFilesRelativePaths.insert(resolvedResourceValue);

                RETURN_IF_FAILED(MrmIndexFile(
                    indexer,
                    resourceUri.c_str(),
                    resolvedResourceValue.c_str(),
                    resourceQualifiers.c_str()));
            }
            else if (candidateType.compare(priStringCandidateTypeName) == 0)
            {
                RETURN_IF_FAILED(MrmIndexString(
                    indexer,
                    resourceUri.c_str(),
                    resolvedResourceValue.c_str(),
                    resourceQualifiers.c_str()));
            }
        }
    }
        
    // Write the new resource file to the output directory. The file name will always be resources.pri
    RETURN_IF_FAILED(MrmCreateResourceFile(
        indexer,
        MrmPackagingModeStandaloneFile,
        MrmPackagingOptionsNone,
        outputDirectoryPath.c_str()));

#if _DEBUG
    // Dump created file for manual verification.
    std::filesystem::path tempOutputPriFilePath{ outputDirectoryPath };
    tempOutputPriFilePath /= resourcesPriFileName;
    std::filesystem::path tempOutputTrimmedXmlFilePath{ outputDirectoryPath };
    tempOutputTrimmedXmlFilePath /= L"resourcesPriTrimmed.xml";
    HRESULT dumpPriResult2 = MrmDumpPriFile(
        tempOutputPriFilePath.c_str(),
        tempOutputPriFilePath.c_str(),
        MrmDumpType_Detailed,
        tempOutputTrimmedXmlFilePath.c_str());
    RETURN_IF_FAILED(dumpPriResult2);
#endif
    return S_OK;
}

bool EndsWithSupportedExtension(std::wstring fileName)
{
    const UINT32 WINDOWS_EXTENSION_LENGTH = 4;
    if (!(fileName.length() > WINDOWS_EXTENSION_LENGTH))
    {
        return false;
    }
    // Supported image filenames for resource resolution are all at least one character, a period, and a three character image extension.
    std::set<std::wstring> supportedExtensions{
        L".jpg",
        L".png",
        L".gif",
    };
    std::wstring extension = fileName.substr(fileName.length() - WINDOWS_EXTENSION_LENGTH, WINDOWS_EXTENSION_LENGTH);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);
    return supportedExtensions.contains(extension);
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

HRESULT EvaluateStringAndAddToSetIfResource(
    std::wstring stringToEvaluate,
    std::wstring packageName,
    std::set<std::wstring>& namedResources,
    std::set<std::wstring>& requiredFilesRelativePaths)
{
    if (stringToEvaluate.empty())
    {
        return S_OK;
    }

    if ((stringToEvaluate.length() > msResourceScheme.length()) && CaseInsensitiveStartsWith(stringToEvaluate, msResourceScheme))
    {
        // Sample node from resource pri xml:
        // <NamedResource name="ApplicationTitle" index="0" uri="ms-resource://Microsoft.KozaniTestApp/Resources/ApplicationTitle">
        std::wstring resourceUri = GetResourceStringPrefix(packageName) + stringToEvaluate.substr(msResourceScheme.length());
        namedResources.insert(resourceUri);
    }
    else if (EndsWithSupportedExtension(stringToEvaluate))
    {
        // This may resolve to many different files via the resource pri, while not existing itself.
        // e.g. for an attribute with value Square44x44Logo="Assets\AppListLogo.png", that file may not exist in the package
        // but "\Assets\AppListLogo.scale-100.png", "\Assets\AppListLogo.scale-200.png" will.
        // Add the path now. It will be removed if found as a named resource in the resource.pri.
        requiredFilesRelativePaths.insert(stringToEvaluate);

        // Sample node from resource pri xml:
        //<NamedResource name="AppListLogo.png" index="191" uri="ms-resource://Microsoft.KozaniTestApp/Files/Assets/AppListLogo.png">
        std::wstring resourceUri = GetResourceFilePrefix(packageName) + ConvertToForwardSlashes(stringToEvaluate);
        namedResources.insert(resourceUri);
    }
    return S_OK;
}

std::list<std::wstring> GetQueries()
{
    std::list<std::wstring> queryList{
        L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='DisplayName']",
        L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='Description']",
        L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='PublisherDisplayName']",
        L"/*[local-name()='Package']/*[local-name()='Properties']/*[local-name()='Logo']",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@DisplayName",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@SmallLogo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@Description",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@Square150x150Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/@Square44x44Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@WideLogo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@Wide310x150Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@Square70x70Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@Square71x71Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@Square310x310Logo",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='DefaultTile']/@ShortName",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='SplashScreen']/@Image",
        L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']/*[local-name()='VisualElements']/*[local-name()='LockScreen']/@BadgeLogo",
    };
    return queryList;
}

HRESULT FindResourceInfoInManifest(
    winrt::Windows::Data::Xml::Dom::XmlDocument manifestDocElement,
    _Inout_ std::wstring& packageName,
    std::set<std::wstring>& namedResources,
    std::set<std::wstring>& requiredFilesRelativePaths)
{
    RETURN_IF_FAILED(FindNodeTextValue(manifestDocElement, packageIdentityNameQuery, true, packageName));

    std::list<std::wstring> queries = GetQueries();
    for (std::wstring query : queries)
    {
        std::wstring potentialResource{};
        RETURN_IF_FAILED(FindNodeTextValue(manifestDocElement, query, false, potentialResource));
        if (!potentialResource.empty())
        {
            RETURN_IF_FAILED(EvaluateStringAndAddToSetIfResource(potentialResource, packageName, namedResources, requiredFilesRelativePaths));
        }
    }
        
    return S_OK;
}

HRESULT CopyRequiredFiles(std::wstring originalDirectory, std::wstring outputDirectory, std::set<std::wstring>& requiredFilesRelativePaths)
{
    for (std::wstring relativeFilePath : requiredFilesRelativePaths)
    {
        std::filesystem::path originalFilePath{ originalDirectory };
        originalFilePath /= relativeFilePath;
        std::filesystem::path newFilePath{ outputDirectory };
        newFilePath /= relativeFilePath;

        std::filesystem::path dirPath = newFilePath.parent_path();
        std::filesystem::create_directories(dirPath);
        std::error_code copyError{};
        std::filesystem::copy_options copyOptions = std::filesystem::copy_options::overwrite_existing;
        std::filesystem::copy_file(originalFilePath, newFilePath, copyOptions, copyError);
        if (copyError.value() != 0)
        {
            std::error_code existsCheckErrorCode{};
            bool fileExists = std::filesystem::exists(originalFilePath, existsCheckErrorCode);
            if (existsCheckErrorCode.value() != 0 || fileExists)
            {
                return E_UNEXPECTED;
            }
        }
    }

    return S_OK;
}

HRESULT CreateKozaniResourcePriFromManifestAndResourcePri(
    _In_ std::filesystem::path manifestFilePath,
    _In_ std::filesystem::path resourcesPriFilePath,
    _In_ std::wstring outputDirectoryPath,
    std::set<std::wstring>& requiredFilesRelativePaths)
{
    winrt::Windows::Data::Xml::Dom::XmlDocument manifestDocElement;
    RETURN_IF_FAILED(GetDocElementFromFile(manifestFilePath, manifestDocElement));

    // Manifest is always required for any package.
    requiredFilesRelativePaths.insert(manifestFileName);

    if (std::filesystem::exists(resourcesPriFilePath))
    {
        //Get all required resource uris and all required filenames from the appxmanifest.xml
        std::wstring packageName = L"";
        std::set<std::wstring> namedResourcesFromManifest;
        RETURN_IF_FAILED(FindResourceInfoInManifest(manifestDocElement, packageName, namedResourcesFromManifest, requiredFilesRelativePaths));

        // Get the list of required files (e.g. tile images) to fulfill manifest resource resolution.
        RETURN_IF_FAILED(CreateKozaniResourcePriWithNamedResources(
            resourcesPriFilePath,
            outputDirectoryPath,
            packageName,
            namedResourcesFromManifest,
            requiredFilesRelativePaths));
    }

    return S_OK;
}

std::set<std::wstring> GetAllowedChildren()
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
    std::set<std::wstring> allowedChildren = GetAllowedChildren();
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

HRESULT CreateKozaniPackageLayout(
    _In_ std::wstring sourceDirectoryPath,
    _In_ std::wstring outputDirectoryPath)
{
    if (!filesystem::is_directory(sourceDirectoryPath) ||
        !filesystem::is_directory(outputDirectoryPath) ||
        (sourceDirectoryPath.compare(outputDirectoryPath) == 0))
    {
        return E_INVALIDARG;
    }
        
    std::filesystem::path manifestFilePath{sourceDirectoryPath};
    manifestFilePath /= manifestFileName;
    std::filesystem::path resourcesPriFilePath{ sourceDirectoryPath };
    resourcesPriFilePath /= resourcesPriFileName;

    std::set<std::wstring> requiredFilesRelativePaths;
    RETURN_IF_FAILED(CreateKozaniResourcePriFromManifestAndResourcePri(manifestFilePath, resourcesPriFilePath, outputDirectoryPath, requiredFilesRelativePaths));
    
    RETURN_IF_FAILED(CopyRequiredFiles(sourceDirectoryPath, outputDirectoryPath, requiredFilesRelativePaths));

    RETURN_IF_FAILED(ModifyManifest(manifestFilePath, outputDirectoryPath));

    return S_OK;
}
