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

#ifndef FSITEM_HPP
#define FSITEM_HPP

#include "internal/fsutil.hpp"
#include "internal/genericitem.hpp"

namespace bit7z {
    namespace filesystem {
        class FSItem : public GenericItem {
            public:
                explicit FSItem( const fs::path& itemPath, fs::path inArchivePath = fs::path() );

                explicit FSItem( fs::directory_entry entry, const fs::path& searchPath );

                bool isDots() const;

                bool isDir() const override;

                uint64_t size() const override;

                FILETIME creationTime() const override;

                FILETIME lastAccessTime() const override;

                FILETIME lastWriteTime() const override;

                tstring name() const override;

                fs::path path() const override;

                fs::path inArchivePath() const override;

                uint32_t attributes() const override;

                HRESULT getStream( ISequentialInStream** inStream ) const override;

            private:
                fs::directory_entry mFileEntry;
                WIN32_FILE_ATTRIBUTE_DATA mFileAttributeData;
                fs::path mInArchivePath;

                void initAttributes( const fs::path& itemPath );
        };
    }
}
#endif // FSITEM_HPP
