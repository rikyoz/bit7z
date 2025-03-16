// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitexception.hpp"
#include "internal/csynchronizedinstream.hpp"
#include "internal/extractcallback.hpp"

#include <algorithm> // For std::copy_n

namespace bit7z {

CSynchronizedInStream::CSynchronizedInStream( std::uint64_t maxMemoryUsage,
                                              const BitInputArchive& parentArchive,
                                              std::uint32_t index )
    : mBufferQueue{ maxMemoryUsage },
      mParentArchive{ parentArchive },
      mIndex{ index },
      mExtractionStarted{ false },
      mExtractionFinished{ false } {
    mCurrentReadPosition = mReadBuffer.begin();
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CSynchronizedInStream::Read( void* data, UInt32 size, UInt32* processedSize ) noexcept {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    if ( !mExtractionStarted ) {
        mExtractionStarted = true;
        mExtractorThread = std::thread( &CSynchronizedInStream::extractParentArchive, this );
    }

    if ( mCurrentReadPosition == mReadBuffer.end() ) {
        mReadBuffer = mBufferQueue.pop();
        mCurrentReadPosition = mReadBuffer.begin();

        if ( mReadBuffer.empty() ) {
            return S_OK;
        }
    }

    const auto copyableSize = std::min( static_cast< UInt32 >( mReadBuffer.end() - mCurrentReadPosition ), size );

    std::copy_n( mCurrentReadPosition, copyableSize, static_cast< byte_t* >( data ) ); //-V2571
    std::advance( mCurrentReadPosition, copyableSize );

    if ( processedSize != nullptr ) {
        *processedSize = copyableSize;
    }

    return S_OK;
}

CSynchronizedInStream::~CSynchronizedInStream() {
    if ( mExtractorThread.joinable() ) {
        mExtractorThread.join();
    }
}

void CSynchronizedInStream::extractParentArchive() try {
    mParentArchive.extractSequentially( mBufferQueue, mIndex );
    mBufferQueue.notifyFinished();
} catch ( const BitException& /*ex*/ ) {
    // TODO: Error handling
    mBufferQueue.notifyFinished();
}

} // namespace bit7z