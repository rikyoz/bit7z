// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#define NOMINMAX

#include "../include/cmultivoloutstream.hpp"

#include "../include/bitexception.hpp"

using namespace bit7z;

/* This class is a modified version of COutMultiVolStream you can find in 7zSDK/CPP/7zip/UI/Common/Update.cpp
 * Main changes made:
 *  + Volumes have all the same size, except the last one (original code allowed volumes of different sizes)
 *  + Use of vector instead of CObjectVector
 *  + Use of tstring instead of FString
 *  + Use of uint64_t instead of UInt64
 *  + The work performed originally by the Init method is now performed by the class constructor */

CMultiVolOutStream::CMultiVolOutStream( uint64_t volSize, const tstring& archiveName ) :
    mVolSize( volSize ),
    mVolPrefix( archiveName + TSTRING( "." ) ),
    mStreamIndex( 0 ),
    mOffsetPos( 0 ),
    mAbsPos( 0 ),
    mLength( 0 ) {}

UInt64 CMultiVolOutStream::GetSize() const { return mLength; }

STDMETHODIMP CMultiVolOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }
    while ( size > 0 ) {
        if ( mStreamIndex >= mVolStreams.size() ) {
            tstring name = to_tstring( mStreamIndex + 1 );
            name.insert( 0, 3 - name.length(), L'0' );
            name.insert( 0, mVolPrefix );

            CAltStreamInfo altStream;
            try {
                altStream.stream = new CFileOutStream( name );
            } catch ( const BitException& ex ) {
                return HRESULT_FROM_WIN32( ex.code() == std::errc::file_exists ? ERROR_FILE_EXISTS : ERROR_OPEN_FAILED );
            }

            if ( altStream.stream->fail() ) {
                return HRESULT_FROM_WIN32( ERROR_OPEN_FAILED );
            }

            altStream.pos = 0;
            altStream.realSize = 0;
            altStream.name = std::move( name );
            mVolStreams.push_back( altStream );
            continue;
        }

        CAltStreamInfo& altStream = mVolStreams[ mStreamIndex ];
        if ( mOffsetPos >= mVolSize ) {
            mOffsetPos -= mVolSize;
            mStreamIndex++;
            continue;
        }
        if ( mOffsetPos != altStream.pos ) {
            RINOK( altStream.stream->Seek( static_cast< int64_t >( mOffsetPos ), STREAM_SEEK_SET, nullptr ) )
            altStream.pos = mOffsetPos;
        }

        auto curSize = static_cast< uint32_t >( std::min( static_cast< uint64_t >( size ), mVolSize - altStream.pos ) );
        UInt32 realProcessed;
        RINOK( altStream.stream->Write( data, curSize, &realProcessed ) )
        altStream.pos += realProcessed;
        mOffsetPos += realProcessed;
        mAbsPos += realProcessed;
        if ( mAbsPos > mLength ) {
            mLength = mAbsPos;
        }
        if ( mOffsetPos > altStream.realSize ) {
            altStream.realSize = mOffsetPos;
        }
        if ( processedSize != nullptr ) {
            *processedSize += realProcessed;
        }
        if ( altStream.pos == mVolSize ) {
            mStreamIndex++;
            mOffsetPos = 0;
        }
        if ( realProcessed == 0 && curSize != 0 ) {
            return E_FAIL;
        }
        break;
    }
    return S_OK;
}

STDMETHODIMP CMultiVolOutStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            mAbsPos = static_cast< uint64_t >( offset );
            break;
        case STREAM_SEEK_CUR:
            mAbsPos += static_cast< uint64_t >( offset );
            break;
        case STREAM_SEEK_END:
            mAbsPos = mLength + static_cast< uint64_t >( offset );
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }
    mOffsetPos = mAbsPos;
    if ( newPosition != nullptr ) {
        *newPosition = mAbsPos;
    }
    mStreamIndex = 0;
    return S_OK;
}

STDMETHODIMP CMultiVolOutStream::SetSize( UInt64 newSize ) {
    size_t i = 0;
    while ( i < mVolStreams.size() ) {
        CAltStreamInfo& altStream = mVolStreams[ i++ ];
        if ( newSize < altStream.realSize ) {
            RINOK( altStream.stream->SetSize( newSize ) )
            altStream.realSize = newSize;
            break;
        }
        newSize -= altStream.realSize;
    }
    while ( i < mVolStreams.size() ) {
        /*{
            CAltStreamInfo& altStream = mVolStreams.back();
            altStream.stream.Release();
            NWindows::NFile::NDir::DeleteFileAlways( altStream.name.c_str() );
        }*/
        tstring streamName = mVolStreams.back().name;
        mVolStreams.pop_back();
        //TODO: Remove read only attribute of as in DeleteFileAlways
        fs::remove( streamName );
        //NWindows::NFile::NDir::DeleteFileAlways( streamName.c_str() );
    }
    mOffsetPos = mAbsPos;
    mStreamIndex = 0;
    mLength = newSize;
    return S_OK;
}
