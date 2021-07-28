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

#ifndef COUTMULTIVOLSTREAM_HPP
#define COUTMULTIVOLSTREAM_HPP

#include <vector>
#include <string>
#include <cstdint>

#include <7zip/IStream.h>
#include <Common/MyCom.h>

#include "internal/cfileoutstream.hpp"

using std::vector;
using std::wstring;

namespace bit7z {
    class CMultiVolOutStream final : public IOutStream, public CMyUnknownImp {

            uint64_t mVolSize;
            tstring mVolPrefix;
            size_t mStreamIndex; // required stream
            uint64_t mOffsetPos;   // offset from start of _streamIndex index
            uint64_t mAbsPos;
            uint64_t mLength;

            struct CAltStreamInfo {
                CMyComPtr< CFileOutStream > stream;
                tstring name;
                uint64_t pos{};
                uint64_t realSize{};
            };

            vector< CAltStreamInfo > mVolStreams;

        public:
            CMultiVolOutStream( uint64_t volSize, const tstring& archiveName );

            MY_UNKNOWN_DESTRUCTOR( ~CMultiVolOutStream() ) = default;

            UInt64 GetSize() const;

            MY_UNKNOWN_IMP1( IOutStream ) // NOLINT(modernize-use-noexcept)

            // IOutStream
            STDMETHOD( Write )( const void* data, UInt32 size, UInt32* processedSize ) override;

            STDMETHOD( Seek )( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) override;

            STDMETHOD( SetSize )( UInt64 newSize ) override;
    };
}

#endif // COUTMULTIVOLSTREAM_HPP
