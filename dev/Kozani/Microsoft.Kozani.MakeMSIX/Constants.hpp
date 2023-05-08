#pragma once
#include <string>

static const std::wstring blockMapFile = L"\\AppxBlockMap.xml";
static const std::wstring manifestFile = L"\\AppxManifest.xml";
static const std::wstring manifestFileName = L"AppxManifest.xml";
static const std::wstring resourcesPriFile = L"\\resources.pri";
static const std::wstring resourcesPriFileName = L"resources.pri";
static const std::wstring resourcesPriXmlDumpFile = L"\\resourcesPriDetailedSchema.xml";

static const std::wstring packageIdentityNameQuery = L"/*[local-name()='Package']/*[local-name()='Identity']/@Name";
static const std::wstring packageIdentityNameAttributeName = L"Name";
static const std::wstring namedResourceQuery = L"//NamedResource";
static const std::wstring priDefaultQualifiersQuery = L"/*[local-name()='PriInfo']/*[local-name()='QualifierInfo']/*[local-name()='Qualifiers']/*[local-name()='Qualifier']";
static const std::wstring priQualifiersQuery = L"*[local-name()='QualifierSet']/*[local-name()='Qualifier']";
static const std::wstring priCandidateQuery = L"*[local-name()='Candidate']";
static const std::wstring priResolvedValueQuery = L"*[local-name()='Value']";
static const std::wstring priNameAttribute = L"name";
static const std::wstring priUriAttribute = L"uri";
static const std::wstring priValueAttribute = L"value";
static const std::wstring priTypeAttribute = L"type";
static const std::wstring priScoreAsDefaultAttribute = L"scoreAsDefault";
static const std::wstring priPathCandidateTypeName = L"Path";
static const std::wstring priStringCandidateTypeName = L"String";
static const std::wstring msResourceScheme = L"ms-resource:";
static const std::wstring msResourceFileUriPath = L"Files";
static const std::wstring msResourceResourceUriPath = L"Resources";
static const std::wstring priDefaultQualifierScore = L"1.0";
static const std::wstring manifestApplicationQuery = L"/*[local-name()='Package']/*[local-name()='Applications']/*[local-name()='Application']";
static const std::wstring manifestUap10Namespace = L"http://schemas.microsoft.com/appx/manifest/uap/windows10/10";
static const std::wstring manifestXmlNamespace = L"http://www.w3.org/2000/xmlns/";
static const std::wstring manifestKozaniHostRuntime = L"KozaniHostRuntime";
static const std::wstring packageIdentityPublisherQuery = L"/*[local-name()='Package']/*[local-name()='Identity']/@Publisher";
static const std::wstring manifestDependenciesQuery = L"/*[local-name()='Package']/*[local-name()='Dependencies']";
static const std::wstring manifestDependenciesPackageDependenciesQuery = L"*[local-name()='PackageDependency']";
static const std::wstring manifestTargetDeviceFamilyQuery = L"*[local-name()='TargetDeviceFamily']";
