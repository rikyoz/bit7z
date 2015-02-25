#include "../include/bitcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/coutmemstream.hpp"
#include "../include/updatecallback.hpp"
#include "../include/memupdatecallback.hpp"
#include "../include/bitexception.hpp"

using namespace std;
using namespace bit7z;
using namespace NWindows;

BitCompressor::BitCompressor( const Bit7zLibrary& lib, const BitOutFormat& format ) : mLibrary( lib ),
    mFormat( format ), mCompressionLevel( BitCompressionLevel::Normal ), mPassword( L"" ), mCryptHeaders( false ),
    mSolidMode( false ) {}

const BitOutFormat& BitCompressor::compressionFormat() {
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

/* from filesystem to filesystem */

void BitCompressor::compress( const vector<wstring>& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( FormatFeatures::MULTIPLE_FILES ) )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    FSIndexer::listFiles( in_paths, dirItems );
    compressToFileSystem( dirItems, out_archive );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_archive ) const {
    vector<wstring> vfiles;
    vfiles.push_back( in_file );
    compressFiles( vfiles, out_archive );
}

void BitCompressor::compressFiles( const vector<wstring>& in_files, const wstring& out_archive ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( FormatFeatures::MULTIPLE_FILES ) )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    FSIndexer::removeListedDirectories( in_files, dirItems );
    compressToFileSystem( dirItems, out_archive );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_archive, bool recursive ) const {
    compressFiles( in_dir, out_archive, L"*", recursive );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_archive, const wstring& filter,
                                   bool recursive ) const {
    if ( !mFormat.hasFeature( FormatFeatures::MULTIPLE_FILES ) )
        throw BitException( "Unsupported operation!" );
    vector<FSItem> dirItems;
    FSIndexer indexer( in_dir, filter );
    indexer.listFilesInDirectory( dirItems, recursive );
    compressToFileSystem( dirItems, out_archive );
}


/* from filesystem to memory buffer */

void BitCompressor::compressFile( const wstring& in_file, vector<byte_t>& out_buffer ) const {
    FSItem item( in_file );
    if ( item.isDir() )
        throw BitException( "Cannot compress a directory into a memory buffer!" );
    if ( !item.exists() )
        throw BitException( "The file does not exist!" );
    vector<FSItem> dirItems;
    dirItems.push_back( item );
    compressToMemory( dirItems, out_buffer );
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitCompressor::compressToFileSystem( const vector<FSItem>& in_items, const wstring& out_archive ) const {
    CMyComPtr<IOutArchive> outArchive;
    mLibrary.createArchiveObject( &( mFormat.guid() ), &IID_IOutArchive, reinterpret_cast< void** >( &outArchive ) );

    vector< const wchar_t* > names;
    vector< NCOM::CPropVariant > values;
    if ( mCryptHeaders && mFormat.hasFeature( FormatFeatures::HEADER_ENCRYPTION ) ) {
        names.push_back( L"he" );
        values.push_back( true );
    }
    if ( mFormat.hasFeature( FormatFeatures::COMPRESSION_LEVEL ) ) {
        names.push_back( L"x" );
        values.push_back( static_cast< UInt32 >( mCompressionLevel ) );
    }
    if ( mSolidMode && mFormat.hasFeature( FormatFeatures::SOLID_ARCHIVE ) ) {
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

//FS -> Memory
void BitCompressor::compressToMemory( const vector<FSItem>& in_items, vector<byte_t>& out_buffer ) const {
    if ( in_items.size() == 0 )
        throw BitException( "The list of files/directories cannot be empty!" );
    if ( !mFormat.hasFeature( FormatFeatures::INMEM_COMPRESSION ) )
        throw BitException( "Unsupported format for in-memory compression!" );

    CMyComPtr<IOutArchive> outArchive;
    mLibrary.createArchiveObject( &mFormat.guid(), &IID_IOutArchive, reinterpret_cast< void** >( &outArchive ) );

    vector< const wchar_t* > names;
    vector< NCOM::CPropVariant > values;
    if ( mCryptHeaders && mFormat.hasFeature( FormatFeatures::HEADER_ENCRYPTION ) ) {
        names.push_back( L"he" );
        values.push_back( true );
    }
    if ( mFormat.hasFeature( FormatFeatures::COMPRESSION_LEVEL ) ) {
        names.push_back( L"x" );
        values.push_back( static_cast< UInt32 >( mCompressionLevel ) );
    }
    if ( mSolidMode && mFormat.hasFeature( FormatFeatures::SOLID_ARCHIVE ) ) {
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

    COutMemStream* outMemStreamSpec = new COutMemStream( out_buffer );
    CMyComPtr<ISequentialOutStream> outMemStream( outMemStreamSpec );

    UpdateCallback* updateCallbackSpec = new UpdateCallback( in_items );
    updateCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveUpdateCallback2> updateCallback( updateCallbackSpec );
    HRESULT result = outArchive->UpdateItems( outMemStream, static_cast< UInt32 >( in_items.size() ), updateCallback );
    updateCallbackSpec->Finilize();

    if ( result == E_NOTIMPL )
        throw BitException( "Unsupported operation!" );
    else if ( result == E_FAIL && updateCallbackSpec->getErrorMessage().empty() )
        throw BitException( "Failed operation (unkwown error)!" );
    else if ( result != S_OK )
        throw BitException( updateCallbackSpec->getErrorMessage() );

    wstring errorString = L"Error for files: ";
    for ( unsigned int i = 0; i < updateCallbackSpec->mFailedFiles.size(); i++ )
        errorString += updateCallbackSpec->mFailedFiles[i] + L" ";

    if ( updateCallbackSpec->mFailedFiles.size() != 0 )
        throw BitException( errorString );
}
