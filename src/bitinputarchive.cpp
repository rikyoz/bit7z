#include "../include/bitinputarchive.hpp"

#include "../include/bitexception.hpp"
#include "../include/util.hpp"
#include "../include/opencallback.hpp"

#include "Common/MyCom.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Common/StreamObjects.h"

using namespace bit7z;
using namespace bit7z::util;
using namespace NWindows;
using namespace NArchive;

namespace bit7z {
    namespace BitFormat {
        const BitInFormat& detectFormatFromExt( const wstring& in_file );
        const BitInFormat& detectFormatFromSig( IInStream* stream );
    }
}

CMyComPtr< IInArchive > openArchiveStream( const BitArchiveHandler& handler,
        const BitInFormat& format,
        const wstring& name,
        IInStream* in_stream ) {
    bool detected_by_signature = false;
    GUID format_GUID;
    if ( format == BitFormat::Auto ) {  // Detecting format of the input file
        format_GUID = BitFormat::detectFormatFromSig( in_stream ).guid();
        detected_by_signature = true;
    } else {  // Format directly given by the user
        format_GUID = format.guid();
    }
    CMyComPtr< IInArchive > in_archive;
    handler.library().initInputArchive( &format_GUID, in_archive );

    // Creating open callback for the file
    auto* open_callback_spec = new OpenCallback( handler, name );
    CMyComPtr< IArchiveOpenCallback > open_callback( open_callback_spec );

    // Trying to open the file with the detected format
    HRESULT res = in_archive->Open( in_stream, nullptr, open_callback );
    if ( res == S_OK ) {
        return in_archive;
    }
    if ( format == BitFormat::Auto && !detected_by_signature ) {
        /* User wanted auto detection of format, an extension was detected but opening failed, so we try a more
         * precise detection by checking the signature.
         * NOTE: If user specified explicitly a format (i.e. not BitFormat::Auto), this check is not performed
         *       and an exception is thrown!
         * NOTE 2: If signature detection was already performed (detected_by_signature == false), it detected a
         *         a wrong format, no further check can be done and an exception must be thrown! */
        format_GUID = BitFormat::detectFormatFromSig( in_stream ).guid();
        handler.library().initInputArchive( &format_GUID, in_archive );
        res = in_archive->Open( in_stream, nullptr, open_callback );
        if ( res == S_OK ) {
            return in_archive;
        }
    }
    throw BitException( L"Cannot open archive '" + name + L"'" );
}

BitInputArchive::BitInputArchive( const BitArchiveHandler& handler, const wstring& in_file ) {
    auto* file_stream_spec = new CInFileStream;
    CMyComPtr< IInStream > file_stream = file_stream_spec;
    if ( !file_stream_spec->Open( in_file.c_str() ) ) {
        throw BitException( L"Cannot open archive file '" + in_file + L"'" );
    }
    auto& detectedFormat = ( handler.format() == BitFormat::Auto ? BitFormat::detectFormatFromExt( in_file ) : handler.format() );
    mInArchive = openArchiveStream( handler, detectedFormat, in_file, file_stream ).Detach();
}

BitInputArchive::BitInputArchive( const BitArchiveHandler& handler, const vector<byte_t>& in_buffer ) {
    auto* buf_stream_spec = new CBufInStream;
    CMyComPtr< IInStream > buf_stream = buf_stream_spec;
    buf_stream_spec->Init( in_buffer.data(), in_buffer.size() );
    mInArchive = openArchiveStream( handler, handler.format(), L".", buf_stream ).Detach();
}

BitPropVariant BitInputArchive::getArchiveProperty( BitProperty property ) const {
    BitPropVariant propvar;
    HRESULT res = mInArchive->GetArchiveProperty( static_cast<PROPID>( property ), &propvar );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve archive property" );
    }
    return propvar;
}

BitPropVariant BitInputArchive::getItemProperty( uint32_t index, BitProperty property ) const {
    BitPropVariant propvar;
    HRESULT res = mInArchive->GetProperty( index, static_cast<PROPID>( property ), &propvar );
    if ( res != S_OK ) {
        throw BitException( L"Could not retrieve property for item at index " + std::to_wstring( index ) );
    }
    return propvar;
}

uint32_t BitInputArchive::itemsCount() const {
    uint32_t items_count;
    HRESULT result = mInArchive->GetNumberOfItems( &items_count );
    if ( result != S_OK ) {
        throw BitException( "Could not retrieve the number of items in the archive" );
    }
    return items_count;
}

bool BitInputArchive::isItemFolder( uint32_t index ) const {
    BitPropVariant prop = getItemProperty( index, BitProperty::IsDir );
    return !prop.isEmpty() && prop.getBool();
}

HRESULT BitInputArchive::initUpdatableArchive( IOutArchive** newArc ) const {
    return mInArchive->QueryInterface( ::IID_IOutArchive, reinterpret_cast< void** >( newArc ) );
}

HRESULT BitInputArchive::extract( const vector<uint32_t>& indices, IArchiveExtractCallback* callback ) const {
    const uint32_t* item_indices = indices.empty() ? nullptr : indices.data();
    uint32_t num_items = indices.empty() ? static_cast< uint32_t >( -1 ) :
                         static_cast< uint32_t >( indices.size() );
    return mInArchive->Extract( item_indices, num_items, NExtract::NAskMode::kExtract, callback );
}

HRESULT BitInputArchive::test( IArchiveExtractCallback* callback ) const {
    return mInArchive->Extract( nullptr, static_cast< uint32_t >( -1 ), NExtract::NAskMode::kTest, callback );
}

HRESULT BitInputArchive::close() const {
    return mInArchive->Close();
}

BitInputArchive::~BitInputArchive() {
    if ( mInArchive ) {
        mInArchive->Release();
    }
}
