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

#include "bitinputarchive.hpp"

#include "bitarchiveitemoffset.hpp"
#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitpropvariant.hpp"
#include "bittypes.hpp"
#include "bitformat.hpp"
#include "internal/bufferextractcallback.hpp"
#include "internal/cbufferinstream.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/cmultivolumeinstream.hpp"
#include "internal/cstdinstream.hpp"
#include "internal/extractcallback.hpp"
#include "internal/fileextractcallback.hpp"
#include "internal/fixedbufferextractcallback.hpp"
#include "internal/opencallback.hpp"
#include "internal/operationresult.hpp"
#include "internal/sequentialextractcallback.hpp"
#include "internal/streamextractcallback.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

#ifdef BIT7Z_AUTO_FORMAT
#include "internal/formatdetect.hpp"
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <istream>
#include <limits>
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

using namespace NWindows;
using namespace NArchive;

namespace bit7z {

auto BitInputArchive::openArchiveStream( const fs::path& name, IInStream* inStream ) -> IInArchive* {
#ifdef BIT7Z_AUTO_FORMAT
    bool detectedBySignature = false;
    if ( *mDetectedFormat == BitFormat::Auto ) {
        // Detecting the format of the input file
        mDetectedFormat = &( detect_format_from_signature( inStream ) );
        detectedBySignature = true;
    }
    CMyComPtr< IInArchive > inArchive = mArchiveHandler.library().initInArchive( *mDetectedFormat );
#else
    CMyComPtr< IInArchive > inArchive = mArchiveHandler.library().initInArchive( mArchiveHandler.format() );
#endif
    // NOTE: CMyComPtr is still needed: if an error occurs, and an exception is thrown,
    // the IInArchive object is deleted automatically.

    // Creating open callback for the file
    auto openCallback = bit7z::make_com< OpenCallback >( mArchiveHandler, name );

    // Trying to open the file with the detected format
#ifndef BIT7Z_AUTO_FORMAT
    const
#endif
    HRESULT res = inArchive->Open( inStream, nullptr, openCallback );

#ifdef BIT7Z_AUTO_FORMAT
    if ( res != S_OK && mArchiveHandler.format() == BitFormat::Auto && !detectedBySignature ) {
        /* User wanted auto-detection of the format, an extension was detected but opening failed, so we try a more
         * precise detection by checking the signature.
         * NOTE: If user specified explicitly a format (i.e., not BitFormat::Auto), this check is not performed,
         *       and an exception is thrown (next if).
         * NOTE 2: If signature detection was already performed (detectedBySignature == false), it detected
         *         a wrong format, no further check can be done, and an exception must be thrown (next if). */

        /* Opening the file might have changed the current file pointer, so we reset it to the beginning of the file
         * to correctly read the file signature. */
        inStream->Seek( 0, STREAM_SEEK_SET, nullptr );
        mDetectedFormat = &( detect_format_from_signature( inStream ) );
        inArchive = mArchiveHandler.library().initInArchive( *mDetectedFormat );
        res = inArchive->Open( inStream, nullptr, openCallback );
    }
#endif

    if ( res != S_OK ) {
        const auto error = openCallback->passwordWasAsked() ?
                           make_error_code( OperationResult::OpenErrorEncrypted ) : make_hresult_code( res );
        throw BitException( "Could not open the archive", error, path_to_tstring( name ) );
    }

    return inArchive.Detach();
}

inline auto detect_format( const BitInFormat& format, const fs::path& arcPath ) -> const BitInFormat* {
#ifdef BIT7Z_AUTO_FORMAT
    return ( ( format == BitFormat::Auto ) ? &detect_format_from_extension( arcPath ) : &format );
#else
    (void)arcPath; // unused when auto format detection is enabled!
    return &format;
#endif
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const tstring& inFile )
    : BitInputArchive( handler, tstring_to_path( inFile ) ) {}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const fs::path& arcPath )
    : mDetectedFormat{ detect_format( handler.format(), arcPath ) },
      mArchiveHandler{ handler },
      mArchivePath{ path_to_tstring( arcPath ) } {
    CMyComPtr< IInStream > fileStream;
    if ( *mDetectedFormat != BitFormat::Split && arcPath.extension() == ".001" ) {
        fileStream = bit7z::make_com< CMultiVolumeInStream, IInStream >( arcPath );
    } else {
        fileStream = bit7z::make_com< CFileInStream, IInStream >( arcPath );
    }
    mInArchive = openArchiveStream( arcPath, fileStream );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const buffer_t& inBuffer )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    auto bufStream = bit7z::make_com< CBufferInStream, IInStream >( inBuffer );
    mInArchive = openArchiveStream( fs::path{}, bufStream );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, std::istream& inStream )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    auto stdStream = bit7z::make_com< CStdInStream, IInStream >( inStream );
    mInArchive = openArchiveStream( fs::path{}, stdStream );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler )
    : mDetectedFormat{ &handler.format() },
      mArchiveHandler{ handler } {
    CMyComPtr< IInArchive > arc = mArchiveHandler.library().initInArchive( mArchiveHandler.format() );
    mInArchive = arc.Detach();
}

auto BitInputArchive::archiveProperty( BitProperty property ) const -> BitPropVariant {
    BitPropVariant archiveProperty;
    const HRESULT res = mInArchive->GetArchiveProperty( static_cast< PROPID >( property ), &archiveProperty );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve archive's " + to_string( property ), make_hresult_code( res ) );
    }
    return archiveProperty;
}

auto BitInputArchive::itemProperty( uint32_t index, BitProperty property ) const -> BitPropVariant {
    BitPropVariant itemProperty;
    const HRESULT res = mInArchive->GetProperty( index, static_cast< PROPID >( property ), &itemProperty );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve " + to_string( property ) +
                            " of the item at the index " + std::to_string( index ),
                            make_hresult_code( res ) );
    }
    if ( property == BitProperty::Path && itemProperty.isEmpty() && itemsCount() == 1 ) {
        if ( mArchivePath.empty() ) {
            itemProperty = kEmptyFileWideAlias;
        } else {
            auto itemPath = tstring_to_path( mArchivePath );
            if ( *mDetectedFormat != BitFormat::Split && itemPath.extension() == ".001" ) {
                itemPath = itemPath.stem();
            }
            itemProperty = path_to_wide_string( itemPath.stem() );
        }
    }
    return itemProperty;
}

auto BitInputArchive::itemHasProperty( uint32_t index, BitProperty property ) const -> bool {
    BitPropVariant itemProperty;
    return mInArchive->GetProperty( index, static_cast< PROPID >( property ), &itemProperty ) == S_OK;
}

auto BitInputArchive::itemsCount() const -> uint32_t {
    uint32_t itemsCount{};
    const HRESULT res = mInArchive->GetNumberOfItems( &itemsCount );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve the number of items in the archive", make_hresult_code( res ) );
    }
    return itemsCount;
}

auto BitInputArchive::isItemFolder( uint32_t index ) const -> bool {
    const BitPropVariant isItemFolder = itemProperty( index, BitProperty::IsDir );
    return !isItemFolder.isEmpty() && isItemFolder.getBool();
}

auto BitInputArchive::isItemEncrypted( uint32_t index ) const -> bool {
    const BitPropVariant isItemEncrypted = itemProperty( index, BitProperty::Encrypted );
    return isItemEncrypted.isBool() && isItemEncrypted.getBool();
}

auto BitInputArchive::initUpdatableArchive( IOutArchive** newArc ) const -> HRESULT {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return mInArchive->QueryInterface( ::IID_IOutArchive, reinterpret_cast< void** >( newArc ) );
}

auto BitInputArchive::detectedFormat() const noexcept -> const BitInFormat& {
#ifdef BIT7Z_AUTO_FORMAT
    // Defensive programming: for how the archive format is detected,
    // a correct BitInputArchive instance should have a non-null mDetectedFormat.
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

void BitInputArchive::extractTo( const tstring& outDir ) const {
    auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >( *this, outDir );
    extractArchive( {}, callback, NAskMode::kExtract );
}

inline auto findInvalidIndex( const std::vector< uint32_t >& indices,
                              uint32_t itemsCount ) noexcept -> std::vector< uint32_t >::const_iterator {
    return std::find_if( indices.cbegin(), indices.cend(), [&itemsCount]( uint32_t index ) noexcept -> bool {
        return index >= itemsCount;
    });
}

void BitInputArchive::extractTo( const tstring& outDir, const std::vector< uint32_t >& indices ) const {
    // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
    const auto invalidIndex = findInvalidIndex( indices, itemsCount() );
    if ( invalidIndex != indices.cend() ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( *invalidIndex ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >( *this, outDir );
    extractArchive( indices, callback, NAskMode::kExtract );
}

void BitInputArchive::extractTo( buffer_t& outBuffer, uint32_t index ) const {
    const uint32_t numberItems = itemsCount();
    if ( index >= numberItems ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    std::map< tstring, buffer_t > buffersMap;
    auto extractCallback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, buffersMap );
    extractArchive( { index }, extractCallback, NAskMode::kExtract );
    outBuffer = std::move( buffersMap.begin()->second );
}

void BitInputArchive::extractTo( std::ostream& outStream, uint32_t index ) const {
    const uint32_t numberItems = itemsCount();
    if ( index >= numberItems ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    auto extractCallback = bit7z::make_com< StreamExtractCallback, ExtractCallback >( *this, outStream );
    extractArchive( { index }, extractCallback, NAskMode::kExtract );
}

void BitInputArchive::extractTo( byte_t* buffer, std::size_t size, uint32_t index ) const {
    if ( buffer == nullptr ) {
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::NullOutputBuffer ) );
    }

    const uint32_t numberItems = itemsCount();
    if ( index >= numberItems ) {
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    auto itemSize = itemProperty( index, BitProperty::Size ).getUInt64();
    if ( size != itemSize ) {
        throw BitException( "Cannot extract archive to pre-allocated buffer",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }

    auto extractCallback = bit7z::make_com< FixedBufferExtractCallback, ExtractCallback >( *this, buffer, size );
    extractArchive( { index }, extractCallback, NAskMode::kExtract );
}

void BitInputArchive::extractTo( std::map< tstring, buffer_t >& outMap ) const {
    const uint32_t numberItems = itemsCount();
    std::vector< uint32_t > filesIndices;
    for ( uint32_t i = 0; i < numberItems; ++i ) {
        if ( !isItemFolder( i ) ) { // Consider only files, not folders
            filesIndices.push_back( i );
        }
    }

    auto extractCallback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, outMap );
    extractArchive( filesIndices, extractCallback, NAskMode::kExtract );
}

void BitInputArchive::test() const {
    testArchive( {} );
}

void BitInputArchive::test( const std::vector< uint32_t >& indices ) const {
    // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
    const auto invalidIndex = findInvalidIndex( indices, itemsCount() );
    if ( invalidIndex != indices.cend() ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( *invalidIndex ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    testArchive( indices );
}

void BitInputArchive::testItem( uint32_t index ) const {
    const uint32_t numberItems = itemsCount();
    if ( index >= numberItems ) {
        throw BitException( "Cannot test item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    testArchive( { index } );
}

void BitInputArchive::testArchive( const std::vector< uint32_t >& indices ) const {
    std::map< tstring, buffer_t > dummyMap; // output map (not used since we are testing).
    auto extractCallback = bit7z::make_com< BufferExtractCallback, ExtractCallback >( *this, dummyMap );
    extractArchive( indices, extractCallback, NAskMode::kTest );
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

auto BitInputArchive::begin() const noexcept -> BitInputArchive::ConstIterator {
    return ConstIterator{ 0, *this };
}

auto BitInputArchive::end() const noexcept -> BitInputArchive::ConstIterator try {
    return ConstIterator{ itemsCount(), *this }; //-V509
} catch ( const BitException& ) {
    return begin();
}

auto BitInputArchive::cbegin() const noexcept -> BitInputArchive::ConstIterator {
    return begin();
}

auto BitInputArchive::cend() const noexcept -> BitInputArchive::ConstIterator {
    return end();
}

auto BitInputArchive::find( const tstring& path ) const noexcept -> BitInputArchive::ConstIterator {
    /* Windows supports both '/' and '\' characters as path separators,
     * but 7-Zip reports item paths with the \ path separator.
     * To support both path separators in the user-supplied path,
     * we use the fs::path's element-wise path equality operator.
     *
     * Other operating systems usually only support the '/' path separator,
     * and 7-Zip uses this in the item paths, so we only need to compare the path strings. */
#ifdef _WIN32
    const auto pathToFind = tstring_to_path( path );
    return std::find_if( begin(), end(), [ &pathToFind ]( const BitArchiveItemOffset& oldItem ) -> bool {
        return oldItem.nativePath() == pathToFind;
    } );
#else
    return std::find_if( begin(), end(), [ &path ]( const BitArchiveItemOffset& oldItem ) -> bool {
        return oldItem.path() == path;
    } );
#endif
}

auto BitInputArchive::contains( const tstring& path ) const noexcept -> bool {
    return find( path ) != end();
}

auto BitInputArchive::itemAt( uint32_t index ) const -> BitArchiveItemOffset {
    const uint32_t numberItems = itemsCount();
    if ( index >= numberItems ) {
        throw BitException( "Cannot get the item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    return { *this, index };
}

void BitInputArchive::openArchiveSeqStream( ISequentialInStream* inStream ) const {
    // Trying to open the archive as a sequential stream.
    CMyComPtr< IArchiveOpenSeq > inSeqArchive{};

    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    HRESULT res = mInArchive->QueryInterface( IID_IArchiveOpenSeq, reinterpret_cast< void** >( &inSeqArchive ) );
    if ( res != S_OK ) {
        throw BitException( "Could not open the archive sequentially", make_hresult_code( res ) );
    }

    res = inSeqArchive->OpenSeq( inStream );
    if ( res != S_OK ) {
        throw BitException( "Could not open the archive sequentially", make_hresult_code( res ) );
    }
}

void BitInputArchive::extractSequentially( BufferQueue& queue, uint32_t index ) const {
    auto extractCallback = bit7z::make_com< SequentialExtractCallback, ExtractCallback >( *this, queue );
    extractArchive( { index }, extractCallback, NAskMode::kExtract );
}

void BitInputArchive::extractArchive( const std::vector< uint32_t >& indices,
                                      ExtractCallback* callback,
                                      int32_t mode ) const {
    const uint32_t* itemIndices = indices.empty() ? nullptr : indices.data();
    const uint32_t numItems = indices.empty() ?
                              std::numeric_limits< uint32_t >::max() : static_cast< uint32_t >( indices.size() );

    const HRESULT res = mInArchive->Extract( itemIndices, numItems, mode, callback );
    if ( res != S_OK ) {
        const auto& errorException = callback->errorException();
        if ( errorException ) {
            std::rethrow_exception( errorException );
        }
        throw BitException( mode == NAskMode::kTest ? "Could not test the archive" : "Could not extract the archive",
                            make_hresult_code( res ) );
    }
}

auto BitInputArchive::ConstIterator::operator++() noexcept -> BitInputArchive::ConstIterator& {
    ++mItemOffset;
    return *this;
}

// NOLINTNEXTLINE(cert-dcl21-cpp)
auto BitInputArchive::ConstIterator::operator++( int ) noexcept -> BitInputArchive::ConstIterator {
    ConstIterator incremented = *this;
    ++( *this );
    return incremented;
}

auto
BitInputArchive::ConstIterator::operator==( const BitInputArchive::ConstIterator& other ) const noexcept -> bool {
    return mItemOffset == other.mItemOffset;
}

auto
BitInputArchive::ConstIterator::operator!=( const BitInputArchive::ConstIterator& other ) const noexcept -> bool {
    return !( *this == other );
}

auto BitInputArchive::ConstIterator::operator*() const noexcept -> BitInputArchive::ConstIterator::reference {
    return mItemOffset;
}

auto BitInputArchive::ConstIterator::operator->() const noexcept -> BitInputArchive::ConstIterator::pointer {
    return &mItemOffset;
}

BitInputArchive::ConstIterator::ConstIterator( uint32_t itemIndex, const BitInputArchive& itemArchive ) noexcept
    : mItemOffset( itemArchive, itemIndex ) {}

} // namespace bit7z