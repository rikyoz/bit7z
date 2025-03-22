/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FIXEDBUFFEREXTRACTCALLBACK_HPP
#define FIXEDBUFFEREXTRACTCALLBACK_HPP

#include "bittypes.hpp"
#include "internal/extractcallback.hpp"

#include <cstddef>
#include <cstdint>

namespace bit7z {

class BitInputArchive;

class FixedBufferExtractCallback final : public ExtractCallback {
    public:
        FixedBufferExtractCallback( const BitInputArchive& inputArchive, byte_t* buffer, std::size_t size );

        FixedBufferExtractCallback( const FixedBufferExtractCallback& ) = delete;

        FixedBufferExtractCallback( FixedBufferExtractCallback&& ) = delete;

        auto operator=( const FixedBufferExtractCallback& ) -> FixedBufferExtractCallback& = delete;

        auto operator=( FixedBufferExtractCallback&& ) -> FixedBufferExtractCallback& = delete;

        ~FixedBufferExtractCallback() override = default;

    private:
        byte_t* mBuffer;
        std::size_t mSize;
        CMyComPtr< ISequentialOutStream > mOutMemStream;

        void releaseStream() override;

        auto getOutStream( std::uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z

#endif // FIXEDBUFFEREXTRACTCALLBACK_HPP
