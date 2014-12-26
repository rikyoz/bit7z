#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/opencallback.hpp"

using namespace bit7z;

BitExtractor::BitExtractor( const Bit7zLibrary& lib, BitInFormat format ) : mLibrary( lib ), mFormat( format ) {}

BitInFormat BitExtractor::extractionFormat() {
    return mFormat;
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (the original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitExtractor::extract( const std::wstring& in_file, const std::wstring& out_dir, const std::wstring& pass ) const {
    CMyComPtr<IInArchive> inArchive;
    mLibrary.createArchiveObject( mFormat.guid(),
                                  &IID_IInArchive,
                                  reinterpret_cast< void** >( &inArchive ) );

    CInFileStream* fileStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> fileStream = fileStreamSpec;
    if ( !fileStreamSpec->Open( in_file.c_str() ) )
        throw BitException( L"Cannot open archive file '" + in_file + L"'" );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( pass );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( fileStream, 0, openCallback ) != S_OK )
        throw BitException( L"Cannot open archive '" + in_file + L"'" );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( inArchive, out_dir );
    extractCallbackSpec->setPassword( pass );

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback ) != S_OK )
        throw BitException( extractCallbackSpec->getErrorMessage() );
}
