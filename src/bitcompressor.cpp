#include "../include/bitcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/fsitem.hpp"
#include "../include/util.hpp"
#include "../include/bitexception.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/coutmultivolstream.hpp"
#include "../include/memupdatecallback.hpp"
#include "../include/updatecallback.hpp"

using namespace std;
using namespace bit7z;
using namespace bit7z::util;
using namespace NWindows;

template< class T >
void compressOut( CMyComPtr< IOutArchive > outArc, CMyComPtr< T > outStream,
                  const vector< FSItem >& in_items, const BitArchiveCreator& creator ) {
    UpdateCallback* updateCallbackSpec = new UpdateCallback( creator, in_items );

    CMyComPtr< IArchiveUpdateCallback2 > updateCallback( updateCallbackSpec );
    HRESULT result = outArc->UpdateItems( outStream, static_cast< UInt32 >( in_items.size() ), updateCallback );
    updateCallbackSpec->Finilize();

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation!" );
    } else if ( result == E_FAIL && updateCallbackSpec->getErrorMessage().empty() ) {
        throw BitException( "Failed operation (unkwown error)!" );
    } else if ( result != S_OK ) {
        throw BitException( updateCallbackSpec->getErrorMessage() );
    }

    wstring errorString = L"Error for files: ";
    for ( unsigned int i = 0; i < updateCallbackSpec->mFailedFiles.size(); i++ ) {
        errorString += updateCallbackSpec->mFailedFiles[ i ] + L" ";
    }

    if ( updateCallbackSpec->mFailedFiles.size() != 0 ) {
        throw BitException( errorString );
    }
}

BitCompressor::BitCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format )
    : BitArchiveCreator( lib, format ) {}

/* from filesystem to filesystem */

void BitCompressor::compress( const vector< wstring >& in_paths, const wstring& out_archive ) const {
    if ( in_paths.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > dirItems;
    FSIndexer::listFiles( in_paths, dirItems );
    compressToFileSystem( dirItems, out_archive );
}

void BitCompressor::compressFile( const wstring& in_file, const wstring& out_archive ) const {
    vector< wstring > vfiles;
    vfiles.push_back( in_file );
    compressFiles( vfiles, out_archive );
}

void BitCompressor::compressFiles( const vector< wstring >& in_files, const wstring& out_archive ) const {
    if ( in_files.size() > 1 && !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > dirItems;
    FSIndexer::removeListedDirectories( in_files, dirItems );
    compressToFileSystem( dirItems, out_archive );
}

void BitCompressor::compressDirectory( const wstring& in_dir, const wstring& out_archive, bool recursive ) const {
    compressFiles( in_dir, out_archive, L"*", recursive );
}

void BitCompressor::compressFiles( const wstring& in_dir, const wstring& out_archive, const wstring& filter,
                                   bool recursive ) const {
    if ( !mFormat.hasFeature( MULTIPLE_FILES ) ) {
        throw BitException( "Unsupported operation!" );
    }
    vector< FSItem > dirItems;
    FSIndexer indexer( in_dir, filter );
    indexer.listFilesInDirectory( dirItems, recursive );
    compressToFileSystem( dirItems, out_archive );
}

/* from filesystem to memory buffer */

void BitCompressor::compressFile( const wstring& in_file, vector< byte_t >& out_buffer ) const {
    FSItem item( in_file );
    if ( item.isDir() ) {
        throw BitException( "Cannot compress a directory into a memory buffer!" );
    }
    if ( !item.exists() ) {
        throw BitException( "The file does not exist!" );
    }
    vector< FSItem > dirItems;
    dirItems.push_back( item );
    compressToMemory( dirItems, out_buffer );
}

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitCompressor::compressToFileSystem( const vector< FSItem >& in_items, const wstring& out_archive ) const {
    CMyComPtr< IOutArchive > outArc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    CMyComPtr< IOutStream > outFileStream;
    if ( mVolumeSize > 0 ) {
        COutMultiVolStream* outMultiVolStreamSpec = new COutMultiVolStream( mVolumeSize, out_archive );
        outFileStream = outMultiVolStreamSpec;
    } else {
        COutFileStream* outFileStreamSpec = new COutFileStream();
        /* note: if you remove the following line (and you pass the outFileStreamSpec to UpdateItems method), you will not
         * have any problem... until you try to compress files with GZip format! In that case your program will crash!! */
        outFileStream = outFileStreamSpec;
        if ( !outFileStreamSpec->Create( out_archive.c_str(), false ) ) {
            throw BitException( L"Can't create archive file '" + out_archive + L"'" );
        }
    }

    compressOut( outArc, outFileStream, in_items, *this );
}

// FS -> Memory
void BitCompressor::compressToMemory( const vector< FSItem >& in_items, vector< byte_t >& out_buffer ) const {
    if ( in_items.size() == 0 ) {
        throw BitException( "The list of files/directories cannot be empty!" );
    }
    if ( !mFormat.hasFeature( INMEM_COMPRESSION ) ) {
        throw BitException( "Unsupported format for in-memory compression!" );
    }

    CMyComPtr< IOutArchive > outArc = initOutArchive( mLibrary, mFormat, mCompressionLevel, mCryptHeaders, mSolidMode );

    COutMemStream* outMemStreamSpec = new COutMemStream( out_buffer );
    CMyComPtr< ISequentialOutStream > outMemStream( outMemStreamSpec );

    compressOut( outArc, outMemStream, in_items, *this );
}
