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

#ifndef BUFFERITEM_HPP
#define BUFFERITEM_HPP

#include <string>

#include "internal/genericstreamitem.hpp"

namespace bit7z {
    using std::vector;

    class BufferItem final : public GenericStreamItem {
        public:
            explicit BufferItem( const vector <byte_t>& buffer, const tstring& name );

            tstring name() const override;

            tstring path() const override;

            fs::path inArchivePath() const override;

            HRESULT getStream( ISequentialInStream** inStream ) const override;

            bool isDir() const noexcept override;

            uint64_t size() const noexcept override;

            FILETIME creationTime() const noexcept override;

            FILETIME lastAccessTime() const noexcept override;

            FILETIME lastWriteTime() const noexcept override;

            uint32_t attributes() const noexcept override;

        private:
            const vector< byte_t >& mBuffer;
            const fs::path mBufferName;
    };
}

#endif //BUFFERITEM_HPP
