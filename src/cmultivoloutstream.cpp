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

#include "../include/cmultivoloutstream.hpp"

#include <string>

#include "Windows/FileDir.h"

/* This class is a modified version of COutMultiVolStream you can find in 7zSDK/CPP/7zip/UI/Common/Update.cpp
 * Main changes made:
 *  + Volumes have all the same size, except the last one (original code allowed volumes of differents sizes)
 *  + Use of vector instead of CObjectVector
 *  + Use of wstring instead of FString
 *  + Use of uint64_t instead of UInt64
 *  + The work performed originally by the Init method is now performed by the class constructor */

CMultiVolOutStream::CMultiVolOutStream( uint64_t size, const wstring& archiveName ) :
    mVolSize( size ),
    mVolPrefix( archiveName + L"." ),
    mStreamIndex( 0 ),
    mOffsetPos( 0 ),
    mAbsPos( 0 ),
    mLength( 0 ) {}

CMultiVolOutStream::~CMultiVolOutStream() {}

HRESULT CMultiVolOutStream::Close() {
    HRESULT res = S_OK;
    for ( auto it = mVolStreams.cbegin(); it != mVolStreams.cend(); ++it ) {
        COutFileStream* s = ( *it ).streamSpec;
        if ( s ) {
            HRESULT res2 = s->Close();
            if ( res2 != S_OK ) {
                res = res2;
            }
        }
    }
    return res;
}

UInt64 CMultiVolOutStream::GetSize() const { return mLength; }

bool CMultiVolOutStream::SetMTime( const FILETIME* mTime ) {
    bool res = true;
    for ( auto it = mVolStreams.cbegin(); it != mVolStreams.cend(); ++it ) {
        COutFileStream* s = ( *it ).streamSpec;
        if ( s ) {
            if ( !s->SetMTime( mTime ) ) {
                res = false;
            }
        }
    }
    return res;
}

STDMETHODIMP CMultiVolOutStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }
    while ( size > 0 ) {
        if ( mStreamIndex >= mVolStreams.size() ) {
            CAltStreamInfo altStream;

            //FChar temp[16];
            //ConvertUInt64ToString( mStreamIndex + 1, temp );
            wstring name = std::to_wstring( mStreamIndex + 1 );
            name.insert( 0, 3 - name.length(), L'0' );
            name.insert( 0, mVolPrefix );
            altStream.streamSpec = new COutFileStream;
            altStream.stream = altStream.streamSpec;
            if ( !altStream.streamSpec->Create( name.c_str(), false ) ) {
                DWORD last_error = ::GetLastError();
                return ( last_error == 0 ) ? E_FAIL : HRESULT_FROM_WIN32( last_error );
            }

            altStream.pos = 0;
            altStream.realSize = 0;
            altStream.name = name;
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
            RINOK( altStream.stream->Seek( static_cast< int64_t >( mOffsetPos ), STREAM_SEEK_SET, nullptr ) );
            altStream.pos = mOffsetPos;
        }

        auto curSize = static_cast< uint32_t >( MyMin( static_cast< uint64_t >( size ), mVolSize - altStream.pos ) );
        UInt32 realProcessed;
        RINOK( altStream.stream->Write( data, curSize, &realProcessed ) );
        //data = ( void* )( ( Byte* )data + realProcessed );
        //size -= realProcessed;
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
    if ( seekOrigin >= 3 ) {
        return STG_E_INVALIDFUNCTION;
    }
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            mAbsPos = offset;
            break;
        case STREAM_SEEK_CUR:
            mAbsPos += offset;
            break;
        case STREAM_SEEK_END:
            mAbsPos = mLength + offset;
            break;
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
            RINOK( altStream.stream->SetSize( newSize ) );
            altStream.realSize = newSize;
            break;
        }
        newSize -= altStream.realSize;
    }
    while ( i < mVolStreams.size() ) {
        {
            CAltStreamInfo& altStream = mVolStreams.back();
            altStream.stream.Release();
            NWindows::NFile::NDir::DeleteFileAlways( altStream.name.c_str() );
        }
        mVolStreams.pop_back();
    }
    mOffsetPos = mAbsPos;
    mStreamIndex = 0;
    mLength = newSize;
    return S_OK;
}
