/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FILEEXTRACTCALLBACK_HPP
#define FILEEXTRACTCALLBACK_HPP

#include <string>

#include "internal/cfileoutstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/processeditem.hpp"

namespace bit7z {
using std::wstring;

class FileExtractCallback final : public ExtractCallback {
    public:
        FileExtractCallback( const BitInputArchive& inputArchive, const tstring& directoryPath );

        FileExtractCallback( const FileExtractCallback& ) = delete;

        FileExtractCallback( FileExtractCallback&& ) = delete;

        FileExtractCallback& operator=( const FileExtractCallback& ) = delete;

        FileExtractCallback& operator=( FileExtractCallback&& ) = delete;

        ~FileExtractCallback() override = default;

        void throwException( HRESULT error ) override;

    private:
        fs::path mInFilePath;     // Input file path
        fs::path mDirectoryPath;  // Output directory
        fs::path mFilePathOnDisk; // Full path to the file on disk
        bool mRetainDirectories;

        ProcessedItem mCurrentItem;

        CMyComPtr< CFileOutStream > mFileOutStream;

        void finishOperation() override;

        void releaseStream() override;

        HRESULT getOutStream( uint32_t index, ISequentialOutStream** outStream ) override;
};
}  // namespace bit7z
#endif // FILEEXTRACTCALLBACK_HPP
