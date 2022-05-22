// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "internal/cmultivolumeoutstream.hpp"

#include "bitexception.hpp"

using namespace bit7z;

CMultiVolumeOutStream::CMultiVolumeOutStream( uint64_t volSize, const tstring& archiveName )
    : mMaxVolumeSize( volSize ),
      mVolumePrefix( archiveName + BIT7Z_STRING( "." ) ),
      mCurrentVolumeIndex( 0 ),
      mCurrentVolumeOffset( 0 ),
      mAbsoluteOffset( 0 ),
      mFullSize( 0 ) {}

UInt64 CMultiVolumeOutStream::GetSize() const noexcept { return mFullSize; }

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }

    mCurrentVolumeIndex += ( mCurrentVolumeOffset / mMaxVolumeSize );
    mCurrentVolumeOffset = mCurrentVolumeOffset % mMaxVolumeSize;

    while ( mCurrentVolumeIndex >= mVolumes.size() ) {
        /* The current volume stream still doesn't exist, so we need to create it. */
        tstring name = to_tstring( static_cast< uint64_t >( mCurrentVolumeIndex ) + 1 );
        name.insert( 0, 3 - name.length(), L'0' );
        name.insert( 0, mVolumePrefix );

        try {
            mVolumes.emplace_back( make_com< CVolumeOutStream >( name ) );
        } catch ( const BitException& ex ) {
            return ex.nativeCode();
        }
    }

    /* Getting the current volume stream. */
    CMyComPtr< CVolumeOutStream >& volume = mVolumes[ mCurrentVolumeIndex ];

    if ( mCurrentVolumeOffset != volume->currentOffset() ) {
        /* The offset we must write to is different from the last offset we wrote to. */
        RINOK( volume->Seek( static_cast< int64_t >( mCurrentVolumeOffset ), STREAM_SEEK_SET, nullptr ) )
    }

    /* Determining how much we can write to the volume stream */
    const auto writeSize = static_cast< uint32_t >( ( std::min )( static_cast< uint64_t >( size ),
                                                                  mMaxVolumeSize - volume->currentOffset() ) );

    /* Writing to the volume stream */
    UInt32 writtenSize{};
    RINOK( volume->Write( data, writeSize, &writtenSize ) )

    /* Updating the offsets */
    mCurrentVolumeOffset += writtenSize;
    mAbsoluteOffset += writtenSize;

    if ( mAbsoluteOffset > mFullSize ) {
        /* We wrote beyond the old known full size of output archive, updating it. */
        mFullSize = mAbsoluteOffset;
    }

    if ( mCurrentVolumeOffset > volume->currentSize() ) {
        volume->setCurrentSize( mCurrentVolumeOffset );
    }

    if ( processedSize != nullptr ) {
        *processedSize += writtenSize;
    }

    if ( volume->currentOffset() == mMaxVolumeSize ) {
        /* We reached the max size for the current volume, so we need to continue on the next one. */
        ++mCurrentVolumeIndex;
        mCurrentVolumeOffset = 0;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) noexcept {
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            mAbsoluteOffset = static_cast< uint64_t >( offset );
            break;
        case STREAM_SEEK_CUR:
            mAbsoluteOffset += static_cast< uint64_t >( offset );
            break;
        case STREAM_SEEK_END:
            mAbsoluteOffset = mFullSize + static_cast< uint64_t >( offset );
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }
    mCurrentVolumeOffset = mAbsoluteOffset;
    if ( newPosition != nullptr ) {
        *newPosition = mAbsoluteOffset;
    }
    mCurrentVolumeIndex = 0;
    return S_OK;
}

COM_DECLSPEC_NOTHROW
STDMETHODIMP CMultiVolumeOutStream::SetSize( UInt64 newSize ) {
    for ( auto& volume : mVolumes ) {
        if ( newSize < volume->currentSize() ) {
            RINOK( volume->SetSize( newSize ) )
            break;
        }
        newSize -= volume->currentSize();
    }
    while ( !mVolumes.empty() ) {
        fs::path volumePath = mVolumes.back()->path();
        mVolumes.pop_back();
        std::error_code ec;
        fs::remove( volumePath, ec );
        if ( ec ) {
            return E_FAIL;
        }
    }
    mCurrentVolumeOffset = mAbsoluteOffset;
    mCurrentVolumeIndex = 0;
    mFullSize = newSize;
    return S_OK;
}
