/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CRAWOUTSTREAM_HPP
#define CRAWOUTSTREAM_HPP

#include "bitabstractarchivehandler.hpp"
#include "internal/com.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

#include <ostream>

namespace bit7z {

class CRawOutStream : public ISequentialOutStream, public CMyUnknownImp {
    public:
        explicit CRawOutStream( RawDataCallback callback );

        CRawOutStream( const CRawOutStream& ) = delete;

        CRawOutStream( CRawOutStream&& ) = delete;

        auto operator=( const CRawOutStream& ) -> CRawOutStream& = delete;

        auto operator=( CRawOutStream&& ) -> CRawOutStream& = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CRawOutStream() ) = default;

        // IOutStream
        BIT7Z_STDMETHOD( Write, void const* data, UInt32 size, UInt32* processedSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( ISequentialOutStream ) //-V2507 //-V2511 //-V835

    private:
        RawDataCallback mOutputCallback;
};

}  // namespace bit7z

#endif // CRAWOUTSTREAM_HPP
