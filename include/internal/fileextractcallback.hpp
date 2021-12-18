/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
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
            FileExtractCallback( const BitInputArchive& inputArchive,
                                 const tstring& directoryPath );

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

            HRESULT getOutStream( uint32_t index, ISequentialOutStream** outStream, int32_t askExtractMode ) override;
    };
}
#endif // FILEEXTRACTCALLBACK_HPP
