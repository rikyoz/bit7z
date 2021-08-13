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

#ifndef CBUFFEROUTSTREAM_HPP
#define CBUFFEROUTSTREAM_HPP

#include <7zip/IStream.h>
#include <Common/MyCom.h>

#include "bittypes.hpp"

namespace bit7z {
    using std::vector;

    class CBufferOutStream final : public IOutStream, public CMyUnknownImp {
        public:
            explicit CBufferOutStream( vector< byte_t >& out_buffer );

            CBufferOutStream( const CBufferOutStream& ) = delete;

            CBufferOutStream( CBufferOutStream&& ) = delete;

            CBufferOutStream& operator=( const CBufferOutStream& ) = delete;

            CBufferOutStream& operator=( CBufferOutStream&& ) = delete;

            MY_UNKNOWN_DESTRUCTOR( ~CBufferOutStream() ) = default;

            MY_UNKNOWN_IMP1( IOutStream ) // NOLINT(modernize-use-noexcept)

            // IOutStream
            BIT7Z_STDMETHOD( Write, const void* data, UInt32 size, UInt32* processedSize );

            BIT7Z_STDMETHOD_NOEXCEPT( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

            BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

        private:
            buffer_t& mBuffer;
            buffer_t::iterator mCurrentPosition;
    };
}
#endif // CBUFFEROUTSTREAM_HPP
