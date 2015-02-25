#include "../include/bitmemcompressor.hpp"

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"
#include "Windows/COM.h"
#include "Windows/PropVariant.h"

#include "../include/fsutil.hpp"
#include "../include/bitexception.hpp"
#include "../include/coutmemstream.hpp"
#include "../include/memupdatecallback.hpp"

using namespace bit7z;
using namespace NWindows;
using std::wstring;
using std::vector;

BitMemCompressor::BitMemCompressor( const Bit7zLibrary& lib, const BitOutFormat& format ) : mLibrary( lib ),
    mFormat( format ), mCryptHeaders( false ), mSolidMode( false ), mCompressionLevel( BitCompressionLevel::Normal ) {}

void BitMemCompressor::compress( const vector<byte_t>& in_buffer, const wstring& out_archive,
                                 wstring in_buffer_name ) const {
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

    COutFileStream* outFileStreamSpec = new COutFileStream();
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if ( !outFileStreamSpec->Create( out_archive.c_str(), false ) )
        throw BitException( L"Can't create archive file '" + out_archive + L"'" );

    if ( in_buffer_name.empty() )
        fsutil::filename( out_archive, in_buffer_name );

    MemUpdateCallback* updateCallbackSpec = new MemUpdateCallback( in_buffer, in_buffer_name /*, in_buffer_size*/ );
    updateCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveUpdateCallback> updateCallback( updateCallbackSpec );
    HRESULT result = outArchive->UpdateItems( outFileStream, 1, updateCallback );
    updateCallbackSpec->Finilize();

    if ( result != S_OK ) throw BitException( updateCallbackSpec->getErrorMessage() );

    wstring errorString = L"Error for files: ";
    for ( unsigned int i = 0; i < updateCallbackSpec->mFailedFiles.size(); i++ )
        errorString += updateCallbackSpec->mFailedFiles[i] + L" ";

    if ( updateCallbackSpec->mFailedFiles.size() != 0 )
        throw BitException( errorString );
}

void BitMemCompressor::compress( const vector<byte_t>& in_buffer, vector<byte_t>& out_buffer,
                                 wstring in_buffer_name ) const {
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

    MemUpdateCallback* updateCallbackSpec = new MemUpdateCallback( in_buffer, in_buffer_name );
    updateCallbackSpec->setPassword( mPassword );

    CMyComPtr<IArchiveUpdateCallback> updateCallback( updateCallbackSpec );
    HRESULT result = outArchive->UpdateItems( outMemStream, 1, updateCallback );
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
