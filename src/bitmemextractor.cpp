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

BitMemExtractor::BitMemExtractor( const Bit7zLibrary& lib, BitInFormat format ) : mLibrary( lib ), mFormat( format ),
    mPassword( L"" ) {}

void BitMemExtractor::extract(const vector<byte_t>& in_buffer, const wstring& out_dir ) const {
    CMyComPtr<IInArchive> inArchive;
    mLibrary.createArchiveObject( &mFormat.guid(), &IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    CBufInStream* bufStreamSpec = new CBufInStream;
    CMyComPtr<IInStream> bufStream( bufStreamSpec );
    bufStreamSpec->Init( &in_buffer[0], in_buffer.size() );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( bufStream, 0, openCallback ) != S_OK )
        throw BitException( L"Cannot open archive buffer" );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( inArchive, out_dir );
    extractCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback ) != S_OK )
        throw BitException( extractCallbackSpec->getErrorMessage() );
}

void BitMemExtractor::extract( const vector<byte_t>& in_buffer, vector<byte_t>& out_buffer, int index ) const {
    CMyComPtr<IInArchive> inArchive;
    mLibrary.createArchiveObject( &mFormat.guid(), &IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    CBufInStream* bufStreamSpec = new CBufInStream;
    CMyComPtr<IInStream> bufStream( bufStreamSpec );
    bufStreamSpec->Init( &in_buffer[0], in_buffer.size() );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( bufStream, 0, openCallback ) != S_OK )
        throw BitException( L"Cannot open archive buffer" );

    NCOM::CPropVariant prop;
    inArchive->GetProperty( index, kpidSize, &prop );

    out_buffer.resize( prop.uintVal + 1 );

    MemExtractCallback* extractCallbackSpec = new MemExtractCallback( inArchive, mFormat, out_buffer/**out_buffer, prop.uintVal + 1 */);
    extractCallbackSpec->setPassword( mPassword );

    UInt32 indices[] = { index };

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( indices, 1, false, extractCallback ) != S_OK ) {
        throw BitException( extractCallbackSpec->getErrorMessage() );
    }

    out_buffer[prop.uintVal] = 0;
}
