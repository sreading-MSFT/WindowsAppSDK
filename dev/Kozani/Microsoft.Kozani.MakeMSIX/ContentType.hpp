// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
// 
//  Helper file for determining content type and compression settings for adding files to a package
#pragma once

#include <AppxPackaging.h>

#include <map>
#include <string>

namespace MSIX {

    class ContentType
    {
    public:
        ContentType(std::wstring contentType, APPX_COMPRESSION_OPTION compressionOpt) :
            m_contentType(contentType), m_compressionOpt(compressionOpt)
        {}

        const std::wstring& GetContentType() { return m_contentType; }
        APPX_COMPRESSION_OPTION GetCompressionOpt() { return m_compressionOpt; }

        static const ContentType& GetContentTypeByExtension(std::wstring& ext);
        static const std::wstring GetPayloadFileContentType(APPX_FOOTPRINT_FILE_TYPE footprintFile);
        static const std::wstring GetBundlePayloadFileContentType(APPX_BUNDLE_FOOTPRINT_FILE_TYPE footprintFile);
    
    private:
        APPX_COMPRESSION_OPTION m_compressionOpt;
        std::wstring m_contentType;
    };
}
