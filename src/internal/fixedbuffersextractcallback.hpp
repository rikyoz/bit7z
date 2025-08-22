/*
* bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FIXEDBUFFERSEXTRACTCALLBACK_HPP
#define FIXEDBUFFERSEXTRACTCALLBACK_HPP

#include "extractcallback.hpp"
#include "bytespan.hpp"
#include "fixedbufferextractcallback.hpp"

namespace bit7z{

class FixedBuffersExtractCallback final : public ExtractCallback {
public:
    FixedBuffersExtractCallback( const BitInputArchive& inputArchive, std::map< uint32_t, ByteSpan > buffers );

    FixedBuffersExtractCallback( const FixedBuffersExtractCallback& ) = delete;

    FixedBuffersExtractCallback( FixedBuffersExtractCallback&& ) = delete;

    auto operator=( const FixedBuffersExtractCallback& ) -> FixedBuffersExtractCallback& = delete;

    auto operator=( FixedBuffersExtractCallback&& ) -> FixedBuffersExtractCallback& = delete;

    ~FixedBuffersExtractCallback() override = default;

private:
    std::map< uint32_t, ByteSpan > mBuffers;
    std::map< uint32_t, FixedBufferExtractCallback > mBuffersCallbacks;

    void releaseStream() override;

    auto getOutStream( uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}



#endif //FIXEDBUFFERSEXTRACTCALLBACK_HPP
