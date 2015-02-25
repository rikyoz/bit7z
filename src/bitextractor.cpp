#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/opencallback.hpp"

using namespace bit7z;
using namespace NWindows;

using std::wstring;

BitExtractor::BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format ) : mLibrary( lib ), mFormat( format ),
    mPassword( L"" ) {}

BitInFormat BitExtractor::extractionFormat() {
    return mFormat;
}

void BitExtractor::setPassword( const wstring& password ) {
    mPassword = password;
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (the original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitExtractor::extract( const wstring& in_file, const wstring& out_dir ) const {
    CMyComPtr<IInArchive> inArchive;
    mLibrary.createArchiveObject( &mFormat.guid(), &IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    CInFileStream* fileStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> fileStream = fileStreamSpec;
    if ( !fileStreamSpec->Open( in_file.c_str() ) )
        throw BitException( L"Cannot open archive file '" + in_file + L"'" );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( fileStream, 0, openCallback ) != S_OK )
        throw BitException( L"Cannot open archive '" + in_file + L"'" );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( inArchive, out_dir );
    extractCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback ) != S_OK )
        throw BitException( extractCallbackSpec->getErrorMessage() );
}

void BitExtractor::extract( const wstring& in_file, vector<byte_t>& out_buffer, int index ) {
    CMyComPtr<IInArchive> inArchive;
    mLibrary.createArchiveObject( &mFormat.guid(), &IID_IInArchive, reinterpret_cast< void** >( &inArchive ) );

    CInFileStream* fileStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> fileStream = fileStreamSpec;
    if ( !fileStreamSpec->Open( in_file.c_str() ) )
        throw BitException( L"Cannot open archive file '" + in_file + L"'" );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( fileStream, 0, openCallback ) != S_OK )
        throw BitException( L"Cannot open archive '" + in_file + L"'" );

    NCOM::CPropVariant prop;
    inArchive->GetProperty( index, kpidSize, &prop );

    out_buffer.resize( prop.uintVal + 1 );

    MemExtractCallback* extractCallbackSpec = new MemExtractCallback( inArchive, mFormat, out_buffer );
    extractCallbackSpec->setPassword( mPassword );

    UInt32 indices[] = { index };

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( indices, 1, false, extractCallback ) != S_OK )
        throw BitException( extractCallbackSpec->getErrorMessage() );

    out_buffer[prop.uintVal] = 0;
}
