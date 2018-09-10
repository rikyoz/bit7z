// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitmemcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "Windows/COM.h"

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
void compressOut( const CMyComPtr< IOutArchive >& out_arc, CMyComPtr< T > out_stream,
                  const vector< byte_t >& in_buffer, const wstring& in_buffer_name, const BitArchiveCreator& creator ) {
    auto* update_callback_spec = new MemUpdateCallback( creator, in_buffer, in_buffer_name );

    CMyComPtr< IArchiveUpdateCallback > update_callback( update_callback_spec );
    HRESULT result = out_arc->UpdateItems( out_stream, 1, update_callback );
    update_callback_spec->Finilize();

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    }

    if ( result == E_FAIL && update_callback_spec->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    }

    if ( result != S_OK ) {
        throw BitException( update_callback_spec->getErrorMessage() );
    }
}

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer, const wstring& out_archive,
                                 wstring in_buffer_name ) const {
    CMyComPtr< IOutArchive > out_arc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    CMyComPtr< IOutStream > out_file_stream;
    if ( mVolumeSize > 0 ) {
        auto* out_multivol_stream_spec = new COutMultiVolStream( mVolumeSize, out_archive );
        out_file_stream = out_multivol_stream_spec;
    } else {
        auto* out_file_stream_spec = new COutFileStream();
        out_file_stream = out_file_stream_spec;
        if ( !out_file_stream_spec->Create( out_archive.c_str(), false ) ) {
            throw BitException( L"Can't create archive file '" + out_archive + L"'" );
        }
    }

    if ( in_buffer_name.empty() ) {
        in_buffer_name = fsutil::filename( out_archive );
    }

    compressOut( out_arc, out_file_stream, in_buffer, in_buffer_name, *this );
}

void BitMemCompressor::compress( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                                 const wstring& in_buffer_name ) const {
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    CMyComPtr< IOutArchive > out_arc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    auto* out_mem_stream_spec = new COutMemStream( out_buffer );
    CMyComPtr< ISequentialOutStream > out_mem_stream( out_mem_stream_spec );

    compressOut( out_arc, out_mem_stream, in_buffer, in_buffer_name, *this );
}
