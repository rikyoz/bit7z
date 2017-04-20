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

// NOTE: this function is not a method of BitMemExtractor because it would dirty the header with extra dependencies
CMyComPtr< IInArchive > openArchive( const Bit7zLibrary& lib, const BitInFormat& format,
                                     const vector< byte_t >& in_buffer, const BitArchiveOpener& opener ) {
    CMyComPtr< IInArchive > inArchive;
    const GUID formatGUID = format.guid();
    lib.createArchiveObject( &formatGUID, &::IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    CBufInStream* bufStreamSpec = new CBufInStream;
    CMyComPtr< IInStream > bufStream( bufStreamSpec );
    bufStreamSpec->Init( &in_buffer[0], in_buffer.size() );

    OpenCallback* openCallbackSpec = new OpenCallback( opener );

    CMyComPtr< IArchiveOpenCallback > openCallback( openCallbackSpec );
    if ( inArchive->Open( bufStream, 0, openCallback ) != S_OK ) {
        throw BitException( L"Cannot open archive buffer" );
    }
    return inArchive;
}

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_buffer, *this );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( *this, inArchive, out_dir );

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( NULL, static_cast< UInt32 >( -1 ), false, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}

void BitMemExtractor::extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                               unsigned int index ) const {
    CMyComPtr< IInArchive > inArchive = openArchive( mLibrary, mFormat, in_buffer, *this );

    NCOM::CPropVariant prop;
    inArchive->GetProperty( index, kpidSize, &prop );

    MemExtractCallback* extractCallbackSpec = new MemExtractCallback( *this, inArchive, out_buffer );

    const UInt32 indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( indices, 1, false, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }
}
