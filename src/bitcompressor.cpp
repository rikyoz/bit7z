#include "../include/bitcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/updatecallback.hpp"
#include "../include/bitexception.hpp"

using namespace std;
using namespace bit7z;
using namespace NWindows;

BitCompressor::BitCompressor( const Bit7zLibrary& lib, BitOutFormat format ) : mLibrary( lib ), mFormat( format ),
    mCompressionLevel( BitCompressionLevel::Normal ), mPassword( L"" ), mCryptHeaders( false ), mSolidMode( false ) {}

BitOutFormat BitCompressor::compressionFormat() {
    return mFormat;
}

void BitCompressor::setPassword( const wstring& password, bool crypt_headers ) {
    mPassword = password;
    mCryptHeaders = ( password.length() > 0 ) && crypt_headers;//true only if a password is set and crypt_headers is true
}

void BitCompressor::setCompressionLevel( BitCompressionLevel compression_level ) {
    mCompressionLevel = compression_level;
}

void BitCompressor::setSolidMode( bool solid_mode ) {
    mSolidMode = solid_mode;
}

void BitCompressor::compress( const vector<wstring>& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && ( mFormat == BitOutFormat::BZip2 || mFormat == BitOutFormat::GZip
                                  || mFormat == BitOutFormat::Xz ) )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    for ( wstring filePath : in_paths ) {
        FSItem item( filePath );
        if ( ! item.exists() ) throw BitException( L"Item '" + item.name() + L"' does not exists" );
        if ( item.isDir() ) {
            FSIndexer indexer( filePath );
            indexer.listFilesInDirectory( dirItems );
        } else
            dirItems.push_back( item );
    }
    compressFS( dirItems, out_archive );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_archive ) const {
    vector<wstring> vfiles;
    vfiles.push_back( in_file );
    compressFiles( vfiles, out_archive );
}

void BitCompressor::compressFiles( const vector<wstring>& in_files, const wstring& out_archive ) const {
    if ( in_files.size() > 1 && ( mFormat == BitOutFormat::BZip2 || mFormat == BitOutFormat::GZip
                                  || mFormat == BitOutFormat::Xz ) )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    for ( wstring filePath : in_files ) {
        FSItem item( filePath );
        if ( item.exists() && !item.isDir() )
            dirItems.push_back( item );
    }
    compressFS( dirItems, out_archive );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_archive, const wstring& filter,
                                   bool recursive ) const {
    if ( mFormat == BitOutFormat::BZip2 || mFormat == BitOutFormat::GZip || mFormat == BitOutFormat::Xz )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    FSIndexer indexer( in_dir, filter );
    indexer.listFilesInDirectory( dirItems, recursive );
    compressFS( dirItems, out_archive );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_archive, bool recursive ) const {
    compressFiles( in_dir, out_archive, L"*", recursive );
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitCompressor::compressFS( const vector<FSItem>& in_items, const wstring& out_archive ) const {
    CMyComPtr<IOutArchive> outArchive;
    mLibrary.createArchiveObject( mFormat.guid(), &IID_IOutArchive, reinterpret_cast< void** >( &outArchive ) );

    vector< const wchar_t* > names;
    vector< NCOM::CPropVariant > values;
    if ( mCryptHeaders && ( mFormat == BitOutFormat::SevenZip || mFormat == BitOutFormat::Xz ) ) {
        names.push_back( L"he" );
        values.push_back( true );
    }
    if ( mFormat != BitOutFormat::Tar && mFormat != BitOutFormat::Wim ) {
        names.push_back( L"x" );
        values.push_back( static_cast< UInt32 >( mCompressionLevel ) );
    }
    if ( mSolidMode && mFormat == BitOutFormat::SevenZip ) {
        names.push_back( L"s" );
        values.push_back( mSolidMode );
    }

    if ( names.size() > 0 ) {
        CMyComPtr<ISetProperties> setProperties;
        if ( outArchive->QueryInterface( IID_ISetProperties, reinterpret_cast< void** >( &setProperties ) ) != S_OK )
            throw BitException( "ISetProperties unsupported" );
        if ( setProperties->SetProperties( &names[0], &values[0], static_cast<UInt32>( names.size() ) ) != S_OK )
            throw BitException( "Cannot set properties of the archive" );
    }

    COutFileStream* outFileStreamSpec = new COutFileStream();
    /* note: if you remove the following line (and you pass the outFileStreamSpec to UpdateItems method), you will not
     * have any problem... until you try to compress files with GZip format! In that case your program will crash!! */
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if ( !outFileStreamSpec->Create( out_archive.c_str(), false ) ) {
        delete outFileStreamSpec;
        throw BitException( L"Can't create archive file '" + out_archive + L"'" );
    }

    UpdateCallback* updateCallbackSpec = new UpdateCallback( in_items );
    updateCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveUpdateCallback2> updateCallback( updateCallbackSpec );
    HRESULT result = outArchive->UpdateItems( outFileStream, static_cast< UInt32 >( in_items.size() ), updateCallback );
    updateCallbackSpec->Finilize();

    if ( result != S_OK ) throw BitException( updateCallbackSpec->getErrorMessage() );

    wstring errorString = L"Error for files: ";
    for ( unsigned int i = 0; i < updateCallbackSpec->mFailedFiles.size(); i++ )
        errorString += updateCallbackSpec->mFailedFiles[i] + L" ";

    if ( updateCallbackSpec->mFailedFiles.size() != 0 )
        throw BitException( errorString );
}
