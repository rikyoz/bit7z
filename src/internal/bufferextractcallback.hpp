/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BUFFEREXTRACTCALLBACK_HPP
#define BUFFEREXTRACTCALLBACK_HPP

#include "bitabstractarchivehandler.hpp"
#include "bittypes.hpp"
#include "internal/extractcallback.hpp"

#include <cstdint>

namespace bit7z {

class BitInputArchive;

class BufferExtractCallback final : public ExtractCallback {
    public:
        BufferExtractCallback( const BitInputArchive& inputArchive, BufferCallback callback );

        BufferExtractCallback( const BufferExtractCallback& ) = delete;

        BufferExtractCallback( BufferExtractCallback&& ) = delete;

        auto operator=( const BufferExtractCallback& ) -> BufferExtractCallback& = delete;

        auto operator=( BufferExtractCallback&& ) -> BufferExtractCallback& = delete;

        ~BufferExtractCallback() override = default;

    private:
        BufferCallback mBufferCallback;
        CMyComPtr< ISequentialOutStream > mOutMemStream;

        void releaseStream() override;

        auto getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z
#endif // BUFFEREXTRACTCALLBACK_HPP
