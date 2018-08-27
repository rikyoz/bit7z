// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitmemextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/extractcallback.hpp"

using namespace bit7z;
using namespace std;
using namespace NWindows;
using namespace NArchive;

// NOTE: this function is not a method of BitMemExtractor because it would dirty the header with extra dependencies
CMyComPtr< IInArchive > openArchive( const Bit7zLibrary& lib, const BitInFormat& format,
                                     const vector< byte_t >& in_buffer, const BitArchiveOpener& opener ) {
    CMyComPtr< IInArchive > in_archive;
    const GUID format_GUID = format.guid();
    lib.createArchiveObject( &format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &in_archive ) );

    auto* buf_stream_spec = new CBufInStream;
    CMyComPtr< IInStream > buf_stream( buf_stream_spec );
    buf_stream_spec->Init( &in_buffer[0], in_buffer.size() );

    auto* open_callback_spec = new OpenCallback( opener );

    CMyComPtr< IArchiveOpenCallback > open_callback( open_callback_spec );
    if ( in_archive->Open( buf_stream, nullptr, open_callback ) != S_OK ) {
        throw BitException( L"Cannot open archive buffer" );
    }
    return in_archive;
}

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( mLibrary, mFormat, in_buffer, *this );

    auto* extract_callback_spec = new ExtractCallback( *this, in_archive, L"", out_dir );

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    if ( in_archive->Extract( nullptr, static_cast< uint32_t >( -1 ), NExtract::NAskMode::kExtract, extract_callback ) != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                               unsigned int index ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( mLibrary, mFormat, in_buffer, *this );

    NCOM::CPropVariant prop;
    in_archive->GetProperty( index, kpidSize, &prop );

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_buffer );

    const uint32_t indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    if ( in_archive->Extract( indices, 1, NExtract::NAskMode::kExtract, extract_callback ) != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}
