// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "bitinputarchive.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/bufferextractcallback.hpp"
#include "internal/cbufferinstream.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/fileextractcallback.hpp"
#include "internal/fixedbufferextractcallback.hpp"
#include "internal/streamextractcallback.hpp"
#include "internal/opencallback.hpp"
#include "internal/util.hpp"
#include "internal/cmultivolumeinstream.hpp"

#ifdef BIT7Z_AUTO_FORMAT
#include "internal/formatdetect.hpp"
#endif

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
#include "internal/fsutil.hpp"
#endif

using namespace bit7z;
using namespace NWindows;
using namespace NArchive;

auto initArchiveObject( const Bit7zLibrary& lib, const GUID* format_GUID ) -> CMyComPtr< IInArchive > {
    CMyComPtr< IInArchive > arc_object;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    lib.createArchiveObject( format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &arc_object ) );
    return arc_object;
}

void extractArc( IInArchive* in_archive, const vector< uint32_t >& indices, ExtractCallback* extract_callback ) {
    const uint32_t* item_indices = indices.empty() ? nullptr : indices.data();
    const uint32_t num_items = indices.empty() ?
                               std::numeric_limits< uint32_t >::max() : static_cast< uint32_t >( indices.size() );

    const HRESULT res = in_archive->Extract( item_indices, num_items, NExtract::NAskMode::kExtract, extract_callback );
    if ( res != S_OK ) {
        const auto& errorException = extract_callback->errorException();
        if ( errorException ) {
            std::rethrow_exception( errorException );
        } else {
            throw BitException( "Could not extract the archive", make_hresult_code( res ) );
        }
    }
}

void testArc( IInArchive* in_archive, ExtractCallback* extract_callback ) {
    const HRESULT res = in_archive->Extract( nullptr,
                                             static_cast< uint32_t >( -1 ),
                                             NExtract::NAskMode::kTest,
                                             extract_callback );
    if ( res != S_OK ) {
        const auto& errorException = extract_callback->errorException();
        if ( errorException ) {
            std::rethrow_exception( errorException );
        } else {
            throw BitException( "Could not test the archive", make_hresult_code( res ) );
        }
    }
}

auto BitInputArchive::openArchiveStream( const fs::path& name, IInStream* in_stream ) -> IInArchive* {
#ifdef BIT7Z_AUTO_FORMAT
    bool detected_by_signature = false;
    if ( *mDetectedFormat == BitFormat::Auto ) {
        // Detecting the format of the input file
        mDetectedFormat = &( detectFormatFromSig( in_stream ) );
        detected_by_signature = true;
    }
    GUID format_GUID = formatGUID( *mDetectedFormat );
#else
    const GUID format_GUID = formatGUID( mArchiveHandler.format() );
#endif
    // NOTE: CMyComPtr is still needed: if an error occurs, and an exception is thrown,
    // the IInArchive object is deleted automatically!
    CMyComPtr< IInArchive > in_archive = initArchiveObject( mArchiveHandler.library(), &format_GUID );

    // Creating open callback for the file
    auto open_callback = bit7z::make_com< OpenCallback, IArchiveOpenCallback >( mArchiveHandler, name );

    // Trying to open the file with the detected format
#ifndef BIT7Z_AUTO_FORMAT
    const
#endif
    HRESULT res = in_archive->Open( in_stream, nullptr, open_callback );

#ifdef BIT7Z_AUTO_FORMAT
    if ( res != S_OK && mArchiveHandler.format() == BitFormat::Auto && !detected_by_signature ) {
        /* User wanted auto-detection of the format, an extension was detected but opening failed, so we try a more
         * precise detection by checking the signature.
         * NOTE: If user specified explicitly a format (i.e., not BitFormat::Auto), this check is not performed,
         *       and an exception is thrown (next if)!
         * NOTE 2: If signature detection was already performed (detected_by_signature == false), it detected
         *         a wrong format, no further check can be done, and an exception must be thrown (next if)! */

        /* Opening the file might have changed the current file pointer, so we reset it to the beginning of the file
         * to correctly read the file signature. */
        in_stream->Seek( 0, STREAM_SEEK_SET, nullptr );
        mDetectedFormat = &( detectFormatFromSig( in_stream ) );
        format_GUID = formatGUID( *mDetectedFormat );
        in_archive = initArchiveObject( mArchiveHandler.library(), &format_GUID );
        res = in_archive->Open( in_stream, nullptr, open_callback );
    }
#endif

    if ( res != S_OK ) {
        throw BitException( "Failed to open the archive", make_hresult_code( res ), name.string< tchar >() );
    }

    return in_archive.Detach();
}

#ifdef BIT7Z_AUTO_FORMAT
#   define DETECT_FORMAT( format, arc_path ) \
        ( (format) == BitFormat::Auto ? &detectFormatFromExt( arc_path ) : &(format) )
#else
#   define DETECT_FORMAT( format, arc_path ) &format
#endif

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const tstring& in_file )
    : BitInputArchive( handler, fs::path{ in_file } ) {}

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, fs::path arc_path )
    : mDetectedFormat{ nullptr },
#else
BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const fs::path& arc_path )
    : mDetectedFormat{ DETECT_FORMAT( handler.format(), arc_path ) },
#endif
      mArchiveHandler{ handler },
      mArchivePath{ arc_path.string< tchar >() } {
#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    if ( filesystem::fsutil::should_format_long_path( arc_path ) ) {
        arc_path = filesystem::fsutil::format_long_path( arc_path );
    }
    mDetectedFormat = DETECT_FORMAT( handler.format(), arc_path );
#endif

    CMyComPtr< IInStream > file_stream;
    if ( *mDetectedFormat != BitFormat::Split && arc_path.extension() == ".001" ) {
        file_stream = bit7z::make_com< CMultiVolumeInStream, IInStream >( arc_path );
    } else {
        file_stream = bit7z::make_com< CFileInStream, IInStream >( arc_path );
    }
    mInArchive = openArchiveStream( arc_path, file_stream );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const std::vector< byte_t >& in_buffer )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    auto buf_stream = bit7z::make_com< CBufferInStream, IInStream >( in_buffer );
    mInArchive = openArchiveStream( BIT7Z_STRING( "." ), buf_stream );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, std::istream& in_stream )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    auto std_stream = bit7z::make_com< CStdInStream, IInStream >( in_stream );
    mInArchive = openArchiveStream( BIT7Z_STRING( "." ), std_stream );
}

auto BitInputArchive::archiveProperty( BitProperty property ) const -> BitPropVariant {
    BitPropVariant archive_property;
    const HRESULT res = mInArchive->GetArchiveProperty( static_cast<PROPID>( property ), &archive_property );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve archive property", make_hresult_code( res ) );
    }
    return archive_property;
}

auto BitInputArchive::itemProperty( uint32_t index, BitProperty property ) const -> BitPropVariant {
    BitPropVariant item_property;
    const HRESULT res = mInArchive->GetProperty( index, static_cast<PROPID>( property ), &item_property );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve property for item at the index " + std::to_string( index ),
                            make_hresult_code( res ) );
    }
    return item_property;
}

auto BitInputArchive::itemsCount() const -> uint32_t {
    uint32_t items_count{};
    const HRESULT res = mInArchive->GetNumberOfItems( &items_count );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve the number of items in the archive", make_hresult_code( res ) );
    }
    return items_count;
}

auto BitInputArchive::isItemFolder( uint32_t index ) const -> bool {
    const BitPropVariant is_item_folder = itemProperty( index, BitProperty::IsDir );
    return !is_item_folder.isEmpty() && is_item_folder.getBool();
}

auto BitInputArchive::isItemEncrypted( uint32_t index ) const -> bool {
    const BitPropVariant is_item_encrypted = itemProperty( index, BitProperty::Encrypted );
    return is_item_encrypted.isBool() && is_item_encrypted.getBool();
}

auto BitInputArchive::initUpdatableArchive( IOutArchive** newArc ) const -> HRESULT {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return mInArchive->QueryInterface( ::IID_IOutArchive, reinterpret_cast< void** >( newArc ) );
}

auto BitInputArchive::detectedFormat() const noexcept -> const BitInFormat& {
#ifdef BIT7Z_AUTO_FORMAT
    // Defensive programming: for how the archive format is detected,
    // a correct BitInputArchive instance should have a non-null mDetectedFormat!
    return mDetectedFormat == nullptr ? BitFormat::Auto : *mDetectedFormat;
#else
    return *mDetectedFormat;
#endif
}

auto BitInputArchive::archivePath() const noexcept -> const tstring& {
    return mArchivePath;
}

auto BitInputArchive::handler() const noexcept -> const BitAbstractArchiveHandler& {
    return mArchiveHandler;
}

void BitInputArchive::extractTo( const tstring& out_dir, const std::vector< uint32_t >& indices ) const {
    auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >( *this, out_dir );
    extractArc( mInArchive, indices, callback );
}

void BitInputArchive::extractTo( std::vector< byte_t >& out_buffer, uint32_t index ) const {
    const uint32_t number_items = itemsCount();
    if ( index >= number_items ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    const vector< uint32_t > indices( 1, index );
    map< tstring, vector< byte_t > > buffers_map;
    auto extract_callback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, buffers_map );
    extractArc( mInArchive, indices, extract_callback );
    out_buffer = std::move( buffers_map.begin()->second );
}

void BitInputArchive::extractTo( std::ostream& out_stream, uint32_t index ) const {
    const uint32_t number_items = itemsCount();
    if ( index >= number_items ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    const vector< uint32_t > indices( 1, index );
    auto extract_callback = bit7z::make_com< StreamExtractCallback, ExtractCallback >( *this, out_stream );
    extractArc( mInArchive, indices, extract_callback );
}

void BitInputArchive::extractTo( byte_t* buffer, std::size_t size, uint32_t index ) const {
    const uint32_t number_items = itemsCount();
    if ( index >= number_items ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    auto item_size = itemProperty( index, BitProperty::Size ).getUInt64();
    if ( size != item_size ) {
        throw BitException( "Cannot extract archive to pre-allocated buffer",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }

    const vector< uint32_t > indices( 1, index );
    auto extract_callback = bit7z::make_com< FixedBufferExtractCallback, ExtractCallback >( *this, buffer, size );
    extractArc( mInArchive, indices, extract_callback );
}

void BitInputArchive::extractTo( std::map< tstring, std::vector< byte_t > >& out_map ) const {
    const uint32_t number_items = itemsCount();
    vector< uint32_t > files_indices;
    for ( uint32_t i = 0; i < number_items; ++i ) {
        if ( !isItemFolder( i ) ) { //Consider only files, not folders
            files_indices.push_back( i );
        }
    }

    auto extract_callback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, out_map );
    extractArc( mInArchive, files_indices, extract_callback );
}

void BitInputArchive::test() const {
    map< tstring, vector< byte_t > > dummy_map; //output map (not used since we are testing!)
    auto extract_callback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, dummy_map );
    testArc( mInArchive, extract_callback );
}

auto BitInputArchive::close() const noexcept -> HRESULT {
    return mInArchive->Close();
}

BitInputArchive::~BitInputArchive() {
    if ( mInArchive != nullptr ) {
        mInArchive->Close();
        mInArchive->Release();
    }
}

auto BitInputArchive::begin() const noexcept -> BitInputArchive::const_iterator {
    return const_iterator{ 0, *this };
}

auto BitInputArchive::end() const noexcept -> BitInputArchive::const_iterator {
    //Note: we do not use itemsCount() since it can throw an exception and end() is marked as noexcept!
    uint32_t items_count = 0;
    mInArchive->GetNumberOfItems( &items_count );
    return const_iterator{ items_count, *this };
}

auto BitInputArchive::cbegin() const noexcept -> BitInputArchive::const_iterator {
    return begin();
}

auto BitInputArchive::cend() const noexcept -> BitInputArchive::const_iterator {
    return end();
}

auto BitInputArchive::find( const tstring& path ) const noexcept -> BitInputArchive::const_iterator {
    return std::find_if( begin(), end(), [ &path ]( auto& old_item ) {
        return old_item.path() == path;
    } );
}

auto BitInputArchive::contains( const tstring& path ) const noexcept -> bool {
    return find( path ) != end();
}

auto BitInputArchive::const_iterator::operator++() noexcept -> BitInputArchive::const_iterator& {
    ++mItemOffset;
    return *this;
}

// NOLINTNEXTLINE(cert-dcl21-cpp)
auto BitInputArchive::const_iterator::operator++( int ) noexcept -> BitInputArchive::const_iterator {
    const_iterator incremented = *this;
    ++( *this );
    return incremented;
}

auto BitInputArchive::const_iterator::operator==( const BitInputArchive::const_iterator& other ) const noexcept -> bool {
    return mItemOffset == other.mItemOffset;
}

auto BitInputArchive::const_iterator::operator!=( const BitInputArchive::const_iterator& other ) const noexcept -> bool {
    return !( *this == other );
}

auto BitInputArchive::const_iterator::operator*() noexcept -> BitInputArchive::const_iterator::reference {
    return mItemOffset;
}

auto BitInputArchive::const_iterator::operator->() noexcept -> BitInputArchive::const_iterator::pointer {
    return &mItemOffset;
}

BitInputArchive::const_iterator::const_iterator( uint32_t item_index, const BitInputArchive& item_archive ) noexcept
    : mItemOffset( item_index, item_archive ) {}
