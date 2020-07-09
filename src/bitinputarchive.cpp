// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitinputarchive.hpp"

#include "../include/bitexception.hpp"
#include "../include/cbufferinstream.hpp"
#include "../include/cfileinstream.hpp"
#include "../include/opencallback.hpp"
#include "../include/extractcallback.hpp"

using namespace bit7z;
using namespace NWindows;
using namespace NArchive;

#ifdef BIT7Z_AUTO_FORMAT
namespace bit7z {
    namespace BitFormat {
        const BitInFormat& detectFormatFromExt( const tstring& in_file );

        const BitInFormat& detectFormatFromSig( IInStream* stream );
    }
}
#endif

CMyComPtr< IInArchive > initArchiveObject( const Bit7zLibrary& lib, const GUID* format_GUID ) {
    CMyComPtr< IInArchive > arc_object;
    lib.createArchiveObject( format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &arc_object ) );
    return arc_object;
}

IInArchive* BitInputArchive::openArchiveStream( const BitArchiveHandler& handler,
                                                const tstring& name,
                                                IInStream* in_stream ) {
#ifdef BIT7Z_AUTO_FORMAT
    bool detected_by_signature = false;
    if ( *mDetectedFormat == BitFormat::Auto ) {
        // Detecting format of the input file
        mDetectedFormat = &( BitFormat::detectFormatFromSig( in_stream ) );
        detected_by_signature = true;
    }
    GUID format_GUID = mDetectedFormat->guid();
#else
    GUID format_GUID = handler.format().guid();
#endif
    // NOTE: CMyComPtr is still needed: if an error occurs and an exception is thrown,
    // the IInArchive object is deleted automatically!
    CMyComPtr< IInArchive > in_archive = initArchiveObject( handler.library(), &format_GUID );

    // Creating open callback for the file
    CMyComPtr< IArchiveOpenCallback > open_callback = new OpenCallback( handler, name );

    // Trying to open the file with the detected format
    HRESULT res = in_archive->Open( in_stream, nullptr, open_callback );

#ifdef BIT7Z_AUTO_FORMAT
    if ( res != S_OK && handler.format() == BitFormat::Auto && !detected_by_signature ) {
        /* User wanted auto detection of format, an extension was detected but opening failed, so we try a more
         * precise detection by checking the signature.
         * NOTE: If user specified explicitly a format (i.e. not BitFormat::Auto), this check is not performed
         *       and an exception is thrown (next if)!
         * NOTE 2: If signature detection was already performed (detected_by_signature == false), it detected a
         *         a wrong format, no further check can be done and an exception must be thrown (next if)! */
        mDetectedFormat = &( BitFormat::detectFormatFromSig( in_stream ) );
        format_GUID = mDetectedFormat->guid();
        in_archive = initArchiveObject( handler.library(), &format_GUID );
        res = in_archive->Open( in_stream, nullptr, open_callback );
    }
#endif

    if ( res != S_OK ) {
        throw BitException( "Cannot open archive", make_hresult_code( res ), name );
    }

    return in_archive.Detach();
}

BitInputArchive::BitInputArchive( const BitArchiveHandler& handler, tstring in_file ) : mArchivePath{ std::move( in_file ) } {
    fs::path in_file_path = mArchivePath;
    CMyComPtr< CFileInStream > file_stream = new CFileInStream( in_file_path );
    if ( file_stream->fail() ) {
        throw BitException( "Cannot open archive file", std::make_error_code( std::errc::io_error ), mArchivePath );
    }
#ifdef BIT7Z_AUTO_FORMAT
    //if auto, detect format from signature here (and try later from content if this fails), otherwise try passed format
    mDetectedFormat = ( handler.format() == BitFormat::Auto ?
                        &BitFormat::detectFormatFromExt( mArchivePath ) : &handler.format() );
#endif
    mInArchive = openArchiveStream( handler, mArchivePath, file_stream );
}

BitInputArchive::BitInputArchive( const BitArchiveHandler& handler, const vector< byte_t >& in_buffer ) {
    CMyComPtr< IInStream > buf_stream = new CBufferInStream( in_buffer );
#ifdef BIT7Z_AUTO_FORMAT
    mDetectedFormat = &handler.format(); //if auto, detect format from content, otherwise try passed format
#endif
    mInArchive = openArchiveStream( handler, TSTRING( "." ), buf_stream );
}

BitInputArchive::BitInputArchive( const BitArchiveHandler& handler, std::istream& in_stream ) {
    CMyComPtr< IInStream > std_stream = new CStdInStream( in_stream );
#ifdef BIT7Z_AUTO_FORMAT
    mDetectedFormat = &handler.format(); //if auto, detect format from content, otherwise try passed format
#endif
    mInArchive = openArchiveStream( handler, TSTRING( "." ), std_stream );
}

BitPropVariant BitInputArchive::getArchiveProperty( BitProperty property ) const {
    BitPropVariant archive_property;
    HRESULT res = mInArchive->GetArchiveProperty( static_cast<PROPID>( property ), &archive_property );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve archive property", make_hresult_code( res ) );
    }
    return archive_property;
}

BitPropVariant BitInputArchive::getItemProperty( uint32_t index, BitProperty property ) const {
    BitPropVariant item_property;
    HRESULT res = mInArchive->GetProperty( index, static_cast<PROPID>( property ), &item_property );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve property for item at index " + std::to_string( index ),
                            make_hresult_code( res ) );
    }
    return item_property;
}

uint32_t BitInputArchive::itemsCount() const {
    uint32_t items_count;
    HRESULT res = mInArchive->GetNumberOfItems( &items_count );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve the number of items in the archive", make_hresult_code( res ) );
    }
    return items_count;
}

bool BitInputArchive::isItemFolder( uint32_t index ) const {
    BitPropVariant is_item_folder = getItemProperty( index, BitProperty::IsDir );
    return !is_item_folder.isEmpty() && is_item_folder.getBool();
}

bool BitInputArchive::isItemEncrypted( uint32_t index ) const {
    BitPropVariant is_item_encrypted = getItemProperty( index, BitProperty::Encrypted );
    return is_item_encrypted.isBool() && is_item_encrypted.getBool();
}

HRESULT BitInputArchive::initUpdatableArchive( IOutArchive** newArc ) const {
    return mInArchive->QueryInterface( ::IID_IOutArchive, reinterpret_cast< void** >( newArc ) );
}

void BitInputArchive::extract( const vector< uint32_t >& indices, ExtractCallback* extract_callback ) const {
    const uint32_t* item_indices = indices.empty() ? nullptr : indices.data();
    uint32_t num_items = indices.empty() ? static_cast< uint32_t >( -1 ) : static_cast< uint32_t >( indices.size() );

    HRESULT res = mInArchive->Extract( item_indices, num_items, NExtract::NAskMode::kExtract, extract_callback );
    if ( res != S_OK ) {
        extract_callback->throwException( res );
    }
}

void BitInputArchive::test( ExtractCallback* extract_callback ) const {
    HRESULT res = mInArchive->Extract( nullptr,
                                       static_cast< uint32_t >( -1 ),
                                       NExtract::NAskMode::kTest,
                                       extract_callback );
    if ( res != S_OK ) {
        extract_callback->throwException( res );
    }
}

#ifdef BIT7Z_AUTO_FORMAT
const BitInFormat& BitInputArchive::detectedFormat() const {
    // Defensive programming: for how the archive format is detected,
    // a correct BitInputArchive instance should have a non null mDetectedFormat!
    return mDetectedFormat == nullptr ? BitFormat::Auto : *mDetectedFormat;
}
#endif

const tstring& BitInputArchive::getArchivePath() const {
    return mArchivePath;
}

HRESULT BitInputArchive::close() const {
    return mInArchive->Close();
}

BitInputArchive::~BitInputArchive() {
    if ( mInArchive != nullptr ) {
        mInArchive->Release();
    }
}

BitInputArchive::const_iterator BitInputArchive::begin() const noexcept {
    return const_iterator{ 0, *this };
}

BitInputArchive::const_iterator BitInputArchive::end() const noexcept {
    //Note: we do not use itemsCount() since it can throw an exception and end() is marked as noexcept!
    uint32_t items_count = 0;
    mInArchive->GetNumberOfItems( &items_count );
    return const_iterator{ items_count, *this };
}

BitInputArchive::const_iterator BitInputArchive::cbegin() const noexcept {
    return begin();
}

BitInputArchive::const_iterator BitInputArchive::cend() const noexcept {
    return end();
}

BitInputArchive::const_iterator& BitInputArchive::const_iterator::operator++() {
    ++mItemOffset;
    return *this;
}

BitInputArchive::const_iterator BitInputArchive::const_iterator::operator++( int ) {
    const_iterator incremented = *this;
    ++( *this );
    return incremented;
}

bool BitInputArchive::const_iterator::operator==( const BitInputArchive::const_iterator& other ) const {
    return mItemOffset == other.mItemOffset;
}

bool BitInputArchive::const_iterator::operator!=( const BitInputArchive::const_iterator& other ) const {
    return !( *this == other );
}

BitInputArchive::const_iterator::reference BitInputArchive::const_iterator::operator*() {
    return mItemOffset;
}

BitInputArchive::const_iterator::pointer BitInputArchive::const_iterator::operator->() {
    return &mItemOffset;
}

BitInputArchive::const_iterator::const_iterator( uint32_t item_index, const BitInputArchive& item_archive )
    : mItemOffset( item_index, item_archive ) {}
