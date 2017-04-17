#include "include/coutmultivolstream.hpp"

#include "Common/IntToString.h"
#include "Windows/FileDir.h"

/* This class is a modified version of COutMultiVolStream you can find in 7zSDK/CPP/7zip/UI/Common/Update.cpp
 * Main changes made:
 *  + Volumes have all the same size, except the last one (original code allowed volumes of differents sizes)
 *  + Use of vector instead of CObjectVector
 *  + Use of wstring instead of FString
 *  + Use of uint64_t instead of UInt64
 *  + The work performed originally by the Init method is now performed by the class constructor */

COutMultiVolStream::COutMultiVolStream(uint64_t size , const wstring &archiveName ) {
    mStreamIndex = 0;
    mOffsetPos = 0;
    mAbsPos = 0;
    mLength = 0;
    mVolSize = size;
    mVolPrefix = archiveName;
    mVolPrefix += L"."; //we don't use the + operator since it doesn't compile with MSVC 2010
}

COutMultiVolStream::~COutMultiVolStream() {

}

HRESULT COutMultiVolStream::Close() {
    HRESULT res = S_OK;
    for ( auto it = mVolStreams.cbegin(); it != mVolStreams.cend(); ++it ) {
        COutFileStream* s = (*it).streamSpec;
        if ( s ) {
            HRESULT res2 = s->Close();
            if ( res2 != S_OK )
                res = res2;
        }
    }
    return res;
}

UInt64 COutMultiVolStream::GetSize() const { return mLength; }

bool COutMultiVolStream::SetMTime( const FILETIME* mTime ) {
    bool res = true;
    for ( auto it = mVolStreams.cbegin(); it != mVolStreams.cend(); ++it ) {
        COutFileStream* s = (*it).streamSpec;
        if ( s )
            if ( !s->SetMTime( mTime ) )
                res = false;
    }
    return res;
}

STDMETHODIMP COutMultiVolStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != NULL )
        *processedSize = 0;
    while ( size > 0 ) {
        if ( mStreamIndex >= mVolStreams.size() ) {
            CAltStreamInfo altStream;

            FChar temp[16];
            ConvertUInt32ToString( mStreamIndex + 1, temp );
            wstring name = temp;
            while ( name.length() < 3 )
                name.insert( 0, L"0" );
            name.insert( 0, mVolPrefix );
            altStream.streamSpec = new COutFileStream;
            altStream.stream = altStream.streamSpec;
            if ( !altStream.streamSpec->Create( name.c_str(), false ) )
                return ::GetLastError();

            altStream.pos = 0;
            altStream.realSize = 0;
            altStream.name = name;
            mVolStreams.push_back( altStream );
            continue;
        }
        CAltStreamInfo &altStream = mVolStreams[mStreamIndex];

        if ( mOffsetPos >= mVolSize ) {
            mOffsetPos -= mVolSize;
            mStreamIndex++;
            continue;
        }
        if ( mOffsetPos != altStream.pos ) {
            RINOK( altStream.stream->Seek( mOffsetPos, STREAM_SEEK_SET, NULL ) );
            altStream.pos = mOffsetPos;
        }

        UInt32 curSize = ( UInt32 )MyMin( ( UInt64 )size, mVolSize - altStream.pos );
        UInt32 realProcessed;
        RINOK( altStream.stream->Write( data, curSize, &realProcessed ) );
        data = ( void* )( ( Byte* )data + realProcessed );
        size -= realProcessed;
        altStream.pos += realProcessed;
        mOffsetPos += realProcessed;
        mAbsPos += realProcessed;
        if ( mAbsPos > mLength )
            mLength = mAbsPos;
        if ( mOffsetPos > altStream.realSize )
            altStream.realSize = mOffsetPos;
        if ( processedSize != NULL )
            *processedSize += realProcessed;
        if ( altStream.pos == mVolSize ) {
            mStreamIndex++;
            mOffsetPos = 0;
        }
        if ( realProcessed == 0 && curSize != 0 )
            return E_FAIL;
        break;
    }
    return S_OK;
}

STDMETHODIMP COutMultiVolStream::Seek( Int64 offset, UInt32 seekOrigin, UInt64* newPosition ) {
    if ( seekOrigin >= 3 )
        return STG_E_INVALIDFUNCTION;
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
    if ( newPosition != NULL )
        *newPosition = mAbsPos;
    mStreamIndex = 0;
    return S_OK;
}

STDMETHODIMP COutMultiVolStream::SetSize( UInt64 newSize ) {
    unsigned i = 0;
    while ( i < mVolStreams.size() ) {
        CAltStreamInfo &altStream = mVolStreams[i++];
        if ( ( UInt64 )newSize < altStream.realSize ) {
            RINOK( altStream.stream->SetSize( newSize ) );
            altStream.realSize = newSize;
            break;
        }
        newSize -= altStream.realSize;
    }
    while ( i < mVolStreams.size() ) {
        {
            CAltStreamInfo &altStream = mVolStreams.back();
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
