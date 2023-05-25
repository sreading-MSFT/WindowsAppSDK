// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <AppxPackaging.h>
#include "ContentType.hpp"

namespace MSIX {

    // Well-known file types to automatically select a MIME content type and compression option to use based on the file extension
    // If the extension is not in this map the default is application/octet-stream and APPX_COMPRESSION_OPTION_NORMAL
    const ContentType& ContentType::GetContentTypeByExtension(std::wstring& ext)
    {
        static const std::map<std::wstring, ContentType> extToContentType = 
        {
            { L".atom",  ContentType(L"application/atom+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".appx",  ContentType(L"application/vnd.ms-appx", APPX_COMPRESSION_OPTION_NONE) },
            { L".b64",   ContentType(L"application/base64", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".cab",   ContentType(L"application/vnd.ms-cab-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { L".doc",   ContentType(L"application/msword", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".dot",   ContentType(L"application/msword", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".docm",  ContentType(L"application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".docx",  ContentType(L"application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE) },
            { L".dotm",  ContentType(L"application/vnd.ms-word.document.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".dotx",  ContentType(L"application/vnd.openxmlformats-officedocument.wordprocessingml.document", APPX_COMPRESSION_OPTION_NONE) },
            { L".dll",   ContentType(L"application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".dtd",   ContentType(L"application/xml-dtd", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".exe",   ContentType(L"application/x-msdownload", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".gz",    ContentType(L"application/x-gzip-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { L".java",  ContentType(L"application/java", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".json",  ContentType(L"application/json", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".p7s",   ContentType(L"application/x-pkcs7-signature", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".pdf",   ContentType(L"application/pdf", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".ps",    ContentType(L"application/postscript", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".potm",  ContentType(L"application/vnd.ms-powerpoint.template.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".potx",  ContentType(L"application/vnd.openxmlformats-officedocument.presentationml.template", APPX_COMPRESSION_OPTION_NONE) },
            { L".ppam",  ContentType(L"application/vnd.ms-powerpoint.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".ppsm",  ContentType(L"application/vnd.ms-powerpoint.slideshow.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".ppsx",  ContentType(L"application/vnd.openxmlformats-officedocument.presentationml.slideshow", APPX_COMPRESSION_OPTION_NONE) },
            { L".ppt",   ContentType(L"application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".pot",   ContentType(L"application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".pps",   ContentType(L"application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".ppa",   ContentType(L"application/vnd.ms-powerpoint", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".pptm",  ContentType(L"application/vnd.ms-powerpoint.presentation.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".pptx",  ContentType(L"application/vnd.openxmlformats-officedocument.presentationml.presentation", APPX_COMPRESSION_OPTION_NONE) },
            { L".rar",   ContentType(L"application/x-rar-compressed", APPX_COMPRESSION_OPTION_NONE) },
            { L".rss",   ContentType(L"application/rss+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".soap",  ContentType(L"application/soap+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".tar",   ContentType(L"application/x-tar", APPX_COMPRESSION_OPTION_NONE) },
            { L".xaml",  ContentType(L"application/xaml+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xap",   ContentType(L"application/x-silverlight-app", APPX_COMPRESSION_OPTION_NONE) },
            { L".xbap",  ContentType(L"application/x-ms-xbap", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xhtml", ContentType(L"application/xhtml+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xlam",  ContentType(L"application/vnd.ms-excel.addin.macroenabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".xls",   ContentType(L"application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xlt",   ContentType(L"application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xla",   ContentType(L"application/vnd.ms-excel", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xlsb",  ContentType(L"application/vnd.ms-excel.sheet.binary.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".xlsm",  ContentType(L"application/vnd.ms-excel.sheet.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".xlsx",  ContentType(L"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", APPX_COMPRESSION_OPTION_NONE) },
            { L".xltm",  ContentType(L"application/vnd.ms-excel.template.macroEnabled.12", APPX_COMPRESSION_OPTION_NONE) },
            { L".xltx",  ContentType(L"application/vnd.openxmlformats-officedocument.spreadsheetml.template", APPX_COMPRESSION_OPTION_NONE) },
            { L".xsl",   ContentType(L"application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xslt",  ContentType(L"application/xslt+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".zip",   ContentType(L"application/x-zip-compressed", APPX_COMPRESSION_OPTION_NONE) },
            // Text types
            { L".c",     ContentType(L"text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".cpp",   ContentType(L"text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".cs",    ContentType(L"text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".css",   ContentType(L"text/css", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".csv",   ContentType(L"text/csv", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".h",     ContentType(L"text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".htm",   ContentType(L"text/html", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".html",  ContentType(L"text/html", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".js",    ContentType(L"application/x-javascript", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".rtf",   ContentType(L"text/richtext", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".sct",   ContentType(L"text/scriptlet", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".txt",   ContentType(L"text/plain", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xml",   ContentType(L"text/xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".xsd",   ContentType(L"text/xml", APPX_COMPRESSION_OPTION_NORMAL) },
            // Audio types
            { L".aiff",  ContentType(L"audio/x-aiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".au",    ContentType(L"audio/basic", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".m4a",   ContentType(L"audio/mp4", APPX_COMPRESSION_OPTION_NONE) },
            { L".mid",   ContentType(L"audio/mid", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".mp3",   ContentType(L"audio/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { L".smf",   ContentType(L"audio/mid", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".wav",   ContentType(L"audio/wav", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".wma",   ContentType(L"audio/x-ms-wma", APPX_COMPRESSION_OPTION_NONE) },
            // Image types
            { L".bmp",   ContentType(L"image/bmp", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".emf",   ContentType(L"image/x-emf", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".gif",   ContentType(L"image/gif", APPX_COMPRESSION_OPTION_NONE) },
            { L".ico",   ContentType(L"image/vnd.microsoft.icon", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".jpg",   ContentType(L"image/jpeg", APPX_COMPRESSION_OPTION_NONE) },
            { L".jpeg",  ContentType(L"image/jpeg", APPX_COMPRESSION_OPTION_NONE) },
            { L".png",   ContentType(L"image/png", APPX_COMPRESSION_OPTION_NONE) },
            { L".svg",   ContentType(L"image/svg+xml", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".tif",   ContentType(L"image/tiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".tiff",  ContentType(L"image/tiff", APPX_COMPRESSION_OPTION_NORMAL) },
            { L".wmf",   ContentType(L"image/x-wmf", APPX_COMPRESSION_OPTION_NORMAL) },
            // Video types
            { L".avi",   ContentType(L"video/avi", APPX_COMPRESSION_OPTION_NONE) },
            { L".mpeg",  ContentType(L"video/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { L".mpg",   ContentType(L"video/mpeg", APPX_COMPRESSION_OPTION_NONE) },
            { L".mov",   ContentType(L"video/quicktime", APPX_COMPRESSION_OPTION_NONE) },
            { L".wmv",   ContentType(L"video/x-ms-wmv", APPX_COMPRESSION_OPTION_NONE) }
        };
        // if the extension is not in the map these are the defaults
        static const ContentType defaultContentType = ContentType(L"application/octet-stream", APPX_COMPRESSION_OPTION_NORMAL);

        auto findExt = extToContentType.find(ext);
        if (findExt == extToContentType.end())
        {
            return defaultContentType;
        }
        return findExt->second;
    }

    const std::wstring ContentType::GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile)
    {
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_MANIFEST)
        {
            return L"application/vnd.ms-appx.manifest+xml";
        }
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_BLOCKMAP)
        {
            return L"application/vnd.ms-appx.blockmap+xml";
        }
        if (footprintFile == APPX_FOOTPRINT_FILE_TYPE_SIGNATURE)
        {
            return L"application/vnd.ms-appx.signature";
        }
        // TODO: add other ones if needed, otherwise throw
        winrt::throw_hresult(E_INVALIDARG);
    }

    const std::wstring ContentType::GetBundlePayloadFileContentType(APPX_BUNDLE_FOOTPRINT_FILE_TYPE footprintFile)
    {
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_MANIFEST)
        {
            return L"application/vnd.ms-appx.bundlemanifest+xml";
        }
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_BLOCKMAP)
        {
            return L"application/vnd.ms-appx.blockmap+xml";
        }
        if (footprintFile == APPX_BUNDLE_FOOTPRINT_FILE_TYPE_SIGNATURE)
        {
            return L"application/vnd.ms-appx.signature";
        }
        // TODO: add other ones if needed, otherwise throw
        winrt::throw_hresult(E_INVALIDARG);
    }
}
