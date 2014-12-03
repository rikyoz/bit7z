#include "../include/bitextractor.hpp"

#include "7zip/Archive/IArchive.h"
#include "Windows/COM.h"

#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/opencallback.hpp"

using namespace Bit7z;

BitExtractor::BitExtractor( const Bit7zLibrary& lib, BitFormat format ) : mLibrary( lib ),
    mFormat( format ) {}

void BitExtractor::extract( const std::wstring& in_file, const std::wstring& out_dir,
                            const std::wstring& password ) {
    CMyComPtr<IInArchive> archive = mLibrary.inputArchiveObject( mFormat );
    CInFileStream* fileStream = new CInFileStream;
    if ( !fileStream->Open( in_file.c_str() ) )
        throw BitException( "Cannot open archive file" );

    OpenCallback* openCallbackSpec = new OpenCallback();
    if ( password.size() > 0 )
        openCallbackSpec->setPassword( password.c_str() );

    CMyComPtr<IArchiveOpenCallback> openCallback( openCallbackSpec );
    if ( archive->Open( fileStream, 0, openCallback ) != S_OK )
        throw BitException( "Cannot open archive" );

    ExtractCallback* extractCallbackSpec = new ExtractCallback( archive, out_dir.c_str() );
    if ( password.size() > 0 )
        extractCallbackSpec->setPassword( password.c_str() );

    CMyComPtr<IArchiveExtractCallback> extractCallback( extractCallbackSpec );
    if ( archive->Extract( NULL, ( UInt32 )( Int32 )( -1 ), false, extractCallback ) != S_OK )
        throw BitException( "Extract Error" );
}
