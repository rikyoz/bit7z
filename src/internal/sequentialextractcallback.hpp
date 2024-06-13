/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SEQUENTIALEXTRACTCALLBACK_HPP
#define SEQUENTIALEXTRACTCALLBACK_HPP

#include "internal/extractcallback.hpp"
#include "internal/bufferqueue.hpp"

#include <cstdint>

namespace bit7z {

class BitInputArchive;

class SequentialExtractCallback final : public ExtractCallback {
    public:
        SequentialExtractCallback( const BitInputArchive& inputArchive, BufferQueue& queue );

        SequentialExtractCallback( const SequentialExtractCallback& ) = delete;

        SequentialExtractCallback( SequentialExtractCallback&& ) = delete;

        auto operator=( const SequentialExtractCallback& ) -> SequentialExtractCallback& = delete;

        auto operator=( SequentialExtractCallback&& ) -> SequentialExtractCallback& = delete;

        auto finishOperation( OperationResult operationResult ) -> HRESULT override;

        ~SequentialExtractCallback() override = default;

    private:
        CMyComPtr< ISequentialOutStream > mSeqOutStream;
        BufferQueue& mBufferQueue;

        void releaseStream() override;

        auto getOutStream( std::uint32_t index, ISequentialOutStream** outStream ) -> HRESULT override;
};

}  // namespace bit7z

#endif // SEQUENTIALEXTRACTCALLBACK_HPP
