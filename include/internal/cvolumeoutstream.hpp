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

#ifndef CVOLUMEOUTSTREAM_HPP
#define CVOLUMEOUTSTREAM_HPP

#include "internal/cfileoutstream.hpp"

namespace bit7z {
    class CVolumeOutStream final : public CFileOutStream {
        public:
            explicit CVolumeOutStream( const tstring& volumeName );

            BIT7Z_NODISCARD fs::path path() const;

            BIT7Z_NODISCARD uint64_t currentOffset() const;

            BIT7Z_NODISCARD uint64_t currentSize() const;

            void setCurrentSize( uint64_t currentSize );

            // IOutStream
            BIT7Z_STDMETHOD( Write, void const* data, UInt32 size, UInt32* processedSize );

            BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

            BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

        private:
            fs::path mPath;

            uint64_t mCurrentOffset;

            uint64_t mCurrentSize;
    };
}  // namespace bit7z

#endif //CVOLUMEOUTSTREAM_HPP
