/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CBUFFERINSTREAM_HPP
#define CBUFFERINSTREAM_HPP

#include "bittypes.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>
#include <Common/MyCom.h>

namespace bit7z {

using std::vector;

class CBufferInStream final : public IInStream, public CMyUnknownImp {
    public:
        explicit CBufferInStream( const vector< byte_t >& in_buffer );

        CBufferInStream( const CBufferInStream& ) = delete;

        CBufferInStream( CBufferInStream&& ) = delete;

        CBufferInStream& operator=( const CBufferInStream& ) = delete;

        CBufferInStream& operator=( CBufferInStream&& ) = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CBufferInStream() ) = default;

        MY_UNKNOWN_IMP1( IInStream ) // NOLINT(modernize-use-noexcept)

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD_NOEXCEPT( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

    private:
        const buffer_t& mBuffer;
        buffer_t::const_iterator mCurrentPosition;
};

}  // namespace bit7z

#endif // CBUFFERINSTREAM_HPP
