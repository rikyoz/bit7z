#include "../include/bitmemcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/util.hpp"
#include "../include/bitexception.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/coutmultivolstream.hpp"
#include "../include/fsutil.hpp"
#include "../include/memupdatecallback.hpp"

using namespace bit7z;
using namespace bit7z::util;
using namespace NWindows;
using std::wstring;
using std::vector;

template< class T >
void compressOut( CMyComPtr< IOutArchive > outArc, CMyComPtr< T > outStream,
                  const vector< byte_t >& in_buffer, const wstring& in_buffer_name, const BitArchiveCreator& creator ) {
    MemUpdateCallback* updateCallbackSpec = new MemUpdateCallback( creator, in_buffer, in_buffer_name );

    CMyComPtr< IArchiveUpdateCallback > updateCallback( updateCallbackSpec );
    HRESULT result = outArc->UpdateItems( outStream, 1, updateCallback );
    updateCallbackSpec->Finilize();

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    } else if ( result == E_FAIL && updateCallbackSpec->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    } else if ( result != S_OK ) {
        throw BitException( updateCallbackSpec->getErrorMessage() );
    }

    wstring errorString = L"Error for files: ";
    for ( unsigned int i = 0; i < updateCallbackSpec->mFailedFiles.size(); i++ ) {
        errorString += updateCallbackSpec->mFailedFiles[ i ] + L" ";
    }

    if ( updateCallbackSpec->mFailedFiles.size() != 0 ) {
        throw BitException( errorString );
    }
}

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer, const wstring& out_archive,
                                 wstring in_buffer_name ) const {
    CMyComPtr< IOutArchive > outArc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    CMyComPtr< IOutStream > outFileStream;
    if ( mVolumeSize > 0 ) {
        COutMultiVolStream* outMultiVolStreamSpec = new COutMultiVolStream( mVolumeSize, out_archive );
        outFileStream = outMultiVolStreamSpec;
    } else {
        COutFileStream* outFileStreamSpec = new COutFileStream();
        outFileStream = outFileStreamSpec;
        if ( !outFileStreamSpec->Create( out_archive.c_str(), false ) ) {
            throw BitException( L"Can't create archive file '" + out_archive + L"'" );
        }
    }

    if ( in_buffer_name.empty() ) {
        fsutil::filename( out_archive, in_buffer_name );
    }

    compressOut( outArc, outFileStream, in_buffer, in_buffer_name, *this );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                                 wstring in_buffer_name ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    CMyComPtr< IOutArchive > outArc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    COutMemStream* outMemStreamSpec = new COutMemStream( out_buffer );
    CMyComPtr< ISequentialOutStream > outMemStream( outMemStreamSpec );

    compressOut( outArc, outMemStream, in_buffer, in_buffer_name, *this );
}
