#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/opencallback.hpp"

using namespace Bit7z;

BitExtractor::BitExtractor( const Bit7zLibrary& lib,
                            Bit7z::BitInFormat format ) : mLibrary( lib ), mFormat( format ) {}

void BitExtractor::extract( const std::wstring& in_file, const std::wstring& out_dir,
                            const std::wstring& password ) const {
    CMyComPtr<IInArchive> inArchive = mLibrary.inputArchiveObject( mFormat );

    CInFileStream* fileStream = new CInFileStream;
    if ( !fileStream->Open( in_file.c_str() ) )
        throw BitException( "Cannot open archive file" );

    OpenCallback* openCallbackSpec = new OpenCallback();
    openCallbackSpec->setPassword( password );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( inArchive->Open( fileStream, 0, openCallback ) != S_OK )
        throw BitException( "Cannot open archive" );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( inArchive, out_dir );
    extractCallbackSpec->setPassword( password );

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( inArchive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback ) != S_OK )
        throw BitException( extractCallbackSpec->getErrorMessage() );
}
