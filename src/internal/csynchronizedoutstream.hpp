/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CSYNCHRONIZEDOUTSTREAM_HPP
#define CSYNCHRONIZEDOUTSTREAM_HPP

#include "bitinputarchive.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"
#include "internal/bufferqueue.hpp"

#include <7zip/IStream.h>

namespace bit7z {

class CSynchronizedOutStream final : public ISequentialOutStream, public CMyUnknownImp {
    public:
        explicit CSynchronizedOutStream( BufferQueue& queue );

        explicit CSynchronizedOutStream( const CSynchronizedOutStream& ) = delete;

        CSynchronizedOutStream( CSynchronizedOutStream&& ) = delete;

        auto operator=( const CSynchronizedOutStream& ) -> CSynchronizedOutStream& = delete;

        auto operator=( CSynchronizedOutStream&& ) -> CSynchronizedOutStream& = delete;

        MY_UNKNOWN_DESTRUCTOR( ~CSynchronizedOutStream() ) = default;

        // ISequentialOutStream
        BIT7Z_STDMETHOD( Write, void const* data, UInt32 size, UInt32* processedSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( ISequentialOutStream ); //-V2507 //-V2511 //-V835 //-V3504 //-V3504

    private:
        BufferQueue& mBufferQueue;
};

} // namespace bit7z

#endif //CSYNCHRONIZEDOUTSTREAM_HPP
