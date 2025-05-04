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
#include "internal/rawdataextractcallback.hpp"
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

auto BitInputArchive::openArchiveStream( const fs::path& name,
                                         IInStream* inStream,
                                         ArchiveStartOffset startOffset ) -> IInArchive* {
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
    const auto openCallback = bit7z::make_com< OpenCallback >( mArchiveHandler, name );

    // Trying to open the file with the detected format
#ifndef BIT7Z_AUTO_FORMAT
    const
#endif
    HRESULT res = [&]() -> HRESULT {
        if ( startOffset == ArchiveStartOffset::FileStart ) {
            const UInt64 maxCheckStartPosition = 0;
            return inArchive->Open( inStream, &maxCheckStartPosition, openCallback );
        }
        return inArchive->Open( inStream, nullptr, openCallback );
    }();

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
#if defined( BIT7Z_AUTO_FORMAT ) && defined( BIT7Z_DETECT_FROM_EXTENSION )
    return ( ( format == BitFormat::Auto ) ? &detect_format_from_extension( arcPath ) : &format );
#else
    (void)arcPath; // unused when auto format detection is enabled!
    return &format;
#endif
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler,
                                  const tstring& inFile,
                                  ArchiveStartOffset startOffset )
    : BitInputArchive( handler, tstring_to_path( inFile ), startOffset ) {}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler,
                                  const fs::path& arcPath,
                                  ArchiveStartOffset startOffset )
    : mDetectedFormat{ detect_format( handler.format(), arcPath ) },
      mArchiveHandler{ handler },
      mArchivePath{ path_to_tstring( arcPath ) } {
    CMyComPtr< IInStream > fileStream;
    if ( *mDetectedFormat != BitFormat::Split && arcPath.extension() == ".001" ) {
        fileStream = bit7z::make_com< CMultiVolumeInStream, IInStream >( arcPath );
    } else {
        fileStream = bit7z::make_com< CFileInStream, IInStream >( arcPath );
    }
    mInArchive = openArchiveStream( arcPath, fileStream, startOffset );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler,
                                  const buffer_t& inBuffer,
                                  ArchiveStartOffset startOffset )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    const auto bufStream = bit7z::make_com< CBufferInStream, IInStream >( inBuffer );
    mInArchive = openArchiveStream( fs::path{}, bufStream, startOffset );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler,
                                  std::istream& inStream,
                                  ArchiveStartOffset startOffset )
    : mDetectedFormat{ &handler.format() }, // if auto, detect the format from content, otherwise try the passed format.
      mArchiveHandler{ handler } {
    const auto stdStream = bit7z::make_com< CStdInStream, IInStream >( inStream );
    mInArchive = openArchiveStream( fs::path{}, stdStream, startOffset );
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const BitArchiveItemOffset& nestedItem )
    : mDetectedFormat{ &handler.format() },
      mArchiveHandler{ handler },
      mArchivePath{ nestedItem.path() } {
    CMyComPtr< IInArchive > arc = mArchiveHandler.library().initInArchive( mArchiveHandler.format() );
    mInArchive = arc.Detach();
}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler, const BitInputArchive& parentArchive )
    : BitInputArchive{ handler, parentArchive, parentArchive.mainSubfileIndex() } {}

BitInputArchive::BitInputArchive( const BitAbstractArchiveHandler& handler,
                                  const BitInputArchive& parentArchive,
                                  std::uint32_t index )
    : mDetectedFormat{ &handler.format() },
      mArchiveHandler{ handler },
      mArchivePath{ parentArchive.itemAt( index ).path() } {
    const CMyComPtr< IInStream > subStream = parentArchive.getSubfileStream( index );
    mInArchive = openArchiveStream( fs::path{}, subStream, ArchiveStartOffset::FileStart );
}

auto BitInputArchive::archiveProperty( BitProperty property ) const -> BitPropVariant {
    BitPropVariant archiveProperty;
    const HRESULT res = mInArchive->GetArchiveProperty( static_cast< PROPID >( property ), &archiveProperty );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve archive's " + to_string( property ), make_hresult_code( res ) );
    }
    return archiveProperty;
}

auto BitInputArchive::itemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant {
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

auto BitInputArchive::itemHasProperty( std::uint32_t index, BitProperty property ) const -> bool {
    BitPropVariant itemProperty;
    return mInArchive->GetProperty( index, static_cast< PROPID >( property ), &itemProperty ) == S_OK;
}

auto BitInputArchive::itemsCount() const -> std::uint32_t {
    std::uint32_t itemsCount{};
    const HRESULT res = mInArchive->GetNumberOfItems( &itemsCount );
    if ( res != S_OK ) {
        throw BitException( "Could not retrieve the number of items in the archive", make_hresult_code( res ) );
    }
    return itemsCount;
}

auto BitInputArchive::isItemFolder( std::uint32_t index ) const -> bool {
    const BitPropVariant isItemFolder = itemProperty( index, BitProperty::IsDir );
    return !isItemFolder.isEmpty() && isItemFolder.getBool();
}

auto BitInputArchive::isItemEncrypted( std::uint32_t index ) const -> bool {
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

auto BitInputArchive::archiveHasPath() const noexcept -> bool {
    return !mArchivePath.empty();
}

auto BitInputArchive::handler() const noexcept -> const BitAbstractArchiveHandler& {
    return mArchiveHandler;
}

void BitInputArchive::useFormatProperty( const wchar_t* name, const BitPropVariant& property ) const {
    CMyComPtr< ISetProperties > setProperties;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    HRESULT res = mInArchive->QueryInterface( ::IID_ISetProperties, reinterpret_cast< void** >( &setProperties ) );
    if ( res != S_OK ) {
        throw BitException( "ISetProperties unsupported", make_hresult_code( res ) );
    }

    const auto propertyNames = { name };
    const auto propertyValues = { property };
    res = setProperties->SetProperties( propertyNames.begin(),
                                        propertyValues.begin(),
                                        static_cast< std::uint32_t >( propertyNames.size() ) );
    if ( res != S_OK ) {
        throw BitException( "Cannot use the archive format property", make_hresult_code( res ) );
    }
}

void BitInputArchive::extractTo( const tstring& outDir, BitIndicesView indices ) const {
    // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
    const auto invalidIndex = findInvalidIndex( indices );
    if ( invalidIndex != indices.cend() ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( *invalidIndex ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    const auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >( *this, outDir );
    extractArchive( callback, NAskMode::kExtract, indices );
}

namespace {
BIT7Z_ALWAYS_INLINE
auto shouldProcessItem( const BitArchiveItem& item, const tstring& itemFilter, bool extractMatchingItems ) -> bool {
    /* This condition is true only if the current item either:
     *  - matches the wildcard pattern, and we must include any matching item; or
     *  - doesn't match the wildcard pattern, and we must exclude those that match. */
    return filesystem::fsutil::wildcard_match( itemFilter, item.path() ) == extractMatchingItems;
}

BIT7Z_ALWAYS_INLINE
auto shouldProcessItem( const BitArchiveItem& item, const tregex& itemFilter, bool extractMatchingItems ) -> bool {
    /* This condition is true only if the current item either:
     *  - matches the regex pattern, and we must include any matching item; or
     *  - doesn't match the regex pattern, and we must exclude those that match. */
    return std::regex_match( item.path(), itemFilter ) == extractMatchingItems;
}
} // namespace

void BitInputArchive::extractMatchingTo( const tstring& outDir,
                                         const tstring& itemFilter,
                                         FilterPolicy policy ) const {
    if ( itemFilter.empty() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
    }

    const bool extractMatchingItems = policy == FilterPolicy::Include;
    extractTo( outDir, [ & ]( const BitArchiveItem& item ) -> FilterResult {
        return shouldProcessItem( item, itemFilter, extractMatchingItems ) ? FilterResult::Process : FilterResult::Skip;
    } );
}

#ifdef BIT7Z_REGEX_MATCHING

void BitInputArchive::extractMatchingRegexTo( const tstring& outDir,
                                              const tstring& regex,
                                              FilterPolicy policy = FilterPolicy::Include ) const {
    if ( regex.empty() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
    }

    const bool extractMatchingItems = policy == FilterPolicy::Include;
    const tregex regexFilter( regex, tregex::ECMAScript | tregex::optimize );
    extractTo( outDir, [ & ]( const BitArchiveItem& item ) -> FilterResult {
        return shouldProcessItem( item, regexFilter, extractMatchingItems )
            ? FilterResult::Process
            : FilterResult::Skip;
    } );
}

#endif

void BitInputArchive::extractTo( const tstring& outDir, FilterCallback filterCallback ) const {
    const auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >(
        *this,
        outDir,
        std::move( filterCallback )
    );
    extractArchive( callback, NAskMode::kExtract );
    if ( !callback->extractionAttempted() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::NoMatchingItems ) );
    }
}

void BitInputArchive::extractTo( const tstring& outDir, RenameCallback renameCallback ) const {
    const auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >(
        *this,
        outDir,
        FilterCallback{},
        std::move( renameCallback )
    );
    extractArchive( callback, NAskMode::kExtract );
}

namespace {
auto containsDotOrDotDot( const tstring& path ) -> bool {
    const std::size_t length = path.length();
    std::size_t pos = 0;

    // Search for the first occurrence of '.'
    do {
        pos = path.find( BIT7Z_STRING( '.' ), pos );
        // Exit the loop if no more dots are found.
        if ( pos == std::string::npos ) {
            return false;
        }

        // Check if we found a single "." or double dots ".." by looking at surrounding characters.

        // Case 1: Single dot "."
        if ( ( pos == 0 || isPathSeparator( path[ pos - 1 ] ) ) && // Start of string, or preceding char is a separator.
             ( pos + 1 == length || isPathSeparator( path[ pos + 1 ] ) ) ) { // End of string, or following char is a separator.
            return true;
        }

        // Case 2: Double dots ".."
        if ( ( pos + 1 < length && path[ pos + 1 ] == BIT7Z_STRING( '.' ) && // Two consecutive dots.
             ( pos == 0 || isPathSeparator( path[ pos - 1 ] ) ) ) && // Start of string, or preceding char is a separator.
             ( pos + 2 == length || isPathSeparator( path[ pos + 2 ] ) ) ) { // End of string, or following char is a separator.
            return true;
        }

        ++pos;
    } while( pos != std::string::npos );

    return false;
}

constexpr auto nativeDot = BIT7Z_NATIVE_STRING( "." );

BIT7Z_ALWAYS_INLINE
auto shouldFilterItem( const native_string& path, const BitArchiveItemOffset& item, FolderPathPolicy policy ) -> bool {
    constexpr auto nativeDotDot = BIT7Z_NATIVE_STRING( ".." );
    return ( starts_with( path, nativeDotDot ) ||
           ( ( path.empty() || path == nativeDot ) && ( policy == FolderPathPolicy::Strip || !item.isDir() ) ) );
}
} // namespace

void BitInputArchive::extractFolderTo( const tstring& outDir,
                                       const tstring& folderPath,
                                       FolderPathPolicy policy ) const {
    if ( folderPath.empty() || containsDotOrDotDot( folderPath ) ) {
        throw BitException( "Invalid folder path to be extracted from the archive",
                            std::make_error_code( std::errc::invalid_argument ) );
    }

    if ( isPathSeparator( folderPath.front() ) ) {
        throw BitException( "The folder path must be relative",
                            std::make_error_code( std::errc::invalid_argument ) );
    }

    std::uint32_t matchingCount = 0;
    const auto folderFsPath = tstring_to_path( folderPath );
    const auto folderName = isPathSeparator( folderPath.back() ) ?
        folderFsPath.parent_path().filename() : folderFsPath.filename();
    auto renameCallback = [ & ]( std::uint32_t index, const tstring& path ) -> tstring {
        // Note: we use the native item's path rather than the second parameter of the callback
        // to avoid unnecessary string conversions when creating the filesystem path object.
        const auto item = itemAt( index );
        const fs::path relativePath = fs::path{ item.nativePath() }.lexically_relative( folderFsPath );
        if ( shouldFilterItem( relativePath.native(), item, policy ) ) {
            return {}; // Skipping the item.
        }

        ++matchingCount;
        if ( policy == FolderPathPolicy::KeepPath ) {
            return path;
        }
        if ( policy == FolderPathPolicy::Strip ) {
            return path_to_tstring( relativePath );
        }
        if ( relativePath.native() == nativeDot ) {
            path_to_tstring( folderName );
        }
        return path_to_tstring( folderName / relativePath );
    };
    const auto callback = bit7z::make_com< FileExtractCallback, ExtractCallback >( *this,
                                                                                   outDir,
                                                                                   FilterCallback{},
                                                                                   std::move( renameCallback ) );
    extractArchive( callback, NAskMode::kExtract );
    if ( matchingCount == 0 ) {
        throw BitException( "No item inside the given folder path within the archive",
                            std::make_error_code( std::errc::invalid_argument ) );
    }
}

void BitInputArchive::extractTo( buffer_t& outBuffer, std::uint32_t index ) const {
    if ( isInvalidIndex( index ) ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    auto bufferCallback = [ &outBuffer ]( std::uint32_t, const tstring& ) -> buffer_t& {
        return outBuffer;
    };
    const auto extractCallback = bit7z::make_com< BufferExtractCallback, ExtractCallback >(
        *this,
        std::move( bufferCallback )
    );
    try {
        extractArchive( extractCallback, NAskMode::kExtract, index );
    } catch ( const BitException& ) {
        outBuffer.clear();
        throw;
    }
}

void BitInputArchive::extractMatchingTo( buffer_t& outBuffer, const tstring& itemFilter, FilterPolicy policy ) const {
    if ( itemFilter.empty() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
    }

    const bool extractMatchingItems = policy == FilterPolicy::Include;
    extractTo(  outBuffer, [ & ]( const BitArchiveItem& item ) -> FilterResult {
        return shouldProcessItem( item, itemFilter, extractMatchingItems ) ? FilterResult::Process : FilterResult::Skip;
    } );
}

#ifdef BIT7Z_REGEX_MATCHING

void BitInputArchive::extractMatchingRegexTo( buffer_t& outBuffer,
                                              const tstring& regex,
                                              FilterPolicy policy = FilterPolicy::Include ) const {
    if ( regex.empty() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
    }

    const bool extractMatchingItems = policy == FilterPolicy::Include;
    const tregex regexFilter( regex, tregex::ECMAScript | tregex::optimize );
    extractTo( outBuffer, [ & ]( const BitArchiveItem& item ) -> FilterResult {
        return shouldProcessItem( item, regexFilter, extractMatchingItems )
            ? FilterResult::Process
            : FilterResult::Skip;
    } );
}

#endif

void BitInputArchive::extractTo( buffer_t& outBuffer, FilterCallback filterCallback ) const {
    auto bufferCallback = [ &outBuffer ]( std::uint32_t, const tstring& ) -> buffer_t& {
        return outBuffer;
    };
    const auto callback = bit7z::make_com< BufferExtractCallback, ExtractCallback >(
        *this,
        std::move( bufferCallback ),
        std::move( filterCallback )
    );
    extractArchive( callback, NAskMode::kExtract );
    if ( !callback->extractionAttempted() ) {
        throw BitException( "Cannot extract items", make_error_code( BitError::NoMatchingFile ) );
    }
}

void BitInputArchive::extractTo( std::ostream& outStream, std::uint32_t index ) const {
    if ( isInvalidIndex( index ) ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    const auto extractCallback = bit7z::make_com< StreamExtractCallback, ExtractCallback >( *this, outStream );
    extractArchive( extractCallback, NAskMode::kExtract, index );
}

void BitInputArchive::extractTo( byte_t* outBuffer, std::size_t size, std::uint32_t index ) const {
    if ( outBuffer == nullptr ) {
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::NullOutputBuffer ) );
    }

    if ( isInvalidIndex( index ) ) {
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::InvalidIndex ) );
    }

    if ( isItemFolder( index ) ) { // Consider only files, not folders
        throw BitException( "Cannot extract the item at the index " + std::to_string( index ) + " to the buffer",
                            make_error_code( BitError::ItemIsAFolder ) );
    }

    const auto itemSize = itemProperty( index, BitProperty::Size ).getUInt64();
    if ( size != itemSize ) {
        throw BitException( "Cannot extract archive to pre-allocated buffer",
                            make_error_code( BitError::InvalidOutputBufferSize ) );
    }

    const auto extractCallback = bit7z::make_com< FixedBufferExtractCallback, ExtractCallback >( *this, outBuffer, size );
    extractArchive( extractCallback, NAskMode::kExtract, index );
}

void BitInputArchive::extractTo( std::map< tstring, buffer_t >& outMap ) const {
    const std::uint32_t numberItems = itemsCount();
    IndicesVector filesIndices;
    for ( std::uint32_t i = 0; i < numberItems; ++i ) {
        if ( !isItemFolder( i ) ) { // Consider only files, not folders
            filesIndices.push_back( i );
        }
    }

    auto bufferCallback = [&outMap]( std::uint32_t, const tstring& path ) -> buffer_t& {
        // Note: the [] operator creates the buffer if it does not already exist.
        return outMap[ path ];
    };
    extractTo( std::move( bufferCallback ), filesIndices );
}

void BitInputArchive::extractTo( BufferCallback callback, BitIndicesView indices ) const {
    const auto extractCallback = bit7z::make_com< BufferExtractCallback, ExtractCallback >(
        *this,
        std::move( callback )
    );
    extractArchive( extractCallback, NAskMode::kExtract, indices );
}

void BitInputArchive::extractTo( RawDataCallback callback, BitIndicesView indices ) const {
    // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
    const auto invalidIndex = findInvalidIndex( indices );
    if ( invalidIndex != indices.cend() ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( *invalidIndex ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    const auto extractCallback = bit7z::make_com< RawDataExtractCallback, ExtractCallback >(
        *this,
        std::move( callback )
    );
    extractArchive( extractCallback, NAskMode::kExtract, indices );
}

void BitInputArchive::test( BitIndicesView indices ) const {
    // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
    const auto invalidIndex = findInvalidIndex( indices );
    if ( invalidIndex != indices.cend() ) {
        throw BitException( "Cannot extract item at the index " + std::to_string( *invalidIndex ),
                            make_error_code( BitError::InvalidIndex ) );
    }

    testArchive( indices );
}

void BitInputArchive::testItem( std::uint32_t index ) const {
    test( index );
}

void BitInputArchive::testArchive( BitIndicesView indices ) const {
    byte_t dummyBuffer{};
    const auto extractCallback = bit7z::make_com< FixedBufferExtractCallback, ExtractCallback >(
        *this,
        &dummyBuffer,
        1u
    );
    extractArchive( extractCallback, NAskMode::kTest, indices );
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

auto BitInputArchive::itemAt( std::uint32_t index ) const -> BitArchiveItemOffset {
    if ( isInvalidIndex( index ) ) {
        throw BitException( "Cannot get the item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    return { *this, index };
}

auto BitInputArchive::mainSubfileIndex() const -> std::uint32_t {
    const BitPropVariant prop = archiveProperty( BitProperty::MainSubfile );
    if ( !prop.isUInt32() ) {
        throw BitException{ "Could not retrieve the index of the main subfile", make_hresult_code( E_FAIL ) };
    }

    const std::uint32_t mainSubfileIndex = prop.getUInt32();
    if ( mainSubfileIndex >= itemsCount() ) {
        throw BitException{ "Could not retrieve the index of the main subfile",
                            make_error_code( BitError::InvalidIndex ) };
    }
    return mainSubfileIndex;
}

auto BitInputArchive::findInvalidIndex( BitIndicesView indices ) const -> BitIndicesView::const_iterator {
    if ( indices.data() == nullptr ) {
        return indices.cend();
    }

    const auto count = itemsCount();
    return std::find_if( indices.cbegin(), indices.cend(), [ &count ]( std::uint32_t index ) noexcept -> bool {
        return index >= count;
    } );
}

auto BitInputArchive::isInvalidIndex( std::uint32_t index ) const -> bool {
    return index >= itemsCount();
}

void BitInputArchive::openArchiveSeqStream( ISequentialInStream* inStream ) const {
    // Trying to open the archive as a sequential stream.
    CMyComPtr< IArchiveOpenSeq > inSeqArchive{};

    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    HRESULT res = mInArchive->QueryInterface( IID_IArchiveOpenSeq, reinterpret_cast< void** >( &inSeqArchive ) );
    if ( res != S_OK ) { // TODO: Improve error message when format doesn't support sequentially opening of archives.
        throw BitException( "Could not open the archive sequentially", make_hresult_code( res ) );
    }

    res = inSeqArchive->OpenSeq( inStream );
    if ( res != S_OK ) {
        throw BitException( "Could not open the archive sequentially", make_hresult_code( res ) );
    }
}

void BitInputArchive::extractSequentially( BufferQueue& queue, std::uint32_t index ) const {
    const auto extractCallback = bit7z::make_com< SequentialExtractCallback, ExtractCallback >( *this, queue );
    extractArchive( extractCallback, NAskMode::kExtract, index );
}

void BitInputArchive::extractArchive( ExtractCallback* callback, std::int32_t mode, BitIndicesView indices ) const {
    const auto numItems = indices.empty() ? std::numeric_limits< std::uint32_t >::max() : indices.size();
    const HRESULT res = mInArchive->Extract( indices.data(), numItems, mode, callback );
    if ( res == S_OK ) {
        return;
    }

    const auto& errorException = callback->errorException();
    if ( !errorException ) {
        throw BitException( mode == NAskMode::kTest ? "Could not test the archive" : "Could not extract the archive",
                            make_hresult_code( res ) );
    }
    std::rethrow_exception( errorException );
}

auto BitInputArchive::getSubfileStream( std::uint32_t index ) const -> CMyComPtr< IInStream > {
    CMyComPtr< IInArchiveGetStream > getStream;

    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    HRESULT res = mInArchive->QueryInterface( IID_IInArchiveGetStream, reinterpret_cast< void** >( &getStream ) );
    if ( res != S_OK ) {
        throw BitException{ "The archive format doesn't support subfile streams", make_hresult_code( res ) };
    }

    CMyComPtr< ISequentialInStream > subSequentialInStream;
    res = getStream->GetStream( index, &subSequentialInStream );
    if ( res != S_OK ) {
        throw BitException{ "Could not get the subfile sequential stream", make_hresult_code( res ) };
    }

    CMyComPtr< IInStream > subInStream;
    res = subSequentialInStream.QueryInterface( IID_IInStream, &subInStream );
    if ( res != S_OK ) {
        throw BitException{ "Could not get the subfile stream", make_hresult_code( res ) };
    }
    return subInStream;
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

auto BitInputArchive::ConstIterator::operator==( const BitInputArchive::ConstIterator& other ) const noexcept -> bool {
    return mItemOffset == other.mItemOffset;
}

auto BitInputArchive::ConstIterator::operator!=( const BitInputArchive::ConstIterator& other ) const noexcept -> bool {
    return !( *this == other );
}

auto BitInputArchive::ConstIterator::operator*() const noexcept -> BitInputArchive::ConstIterator::reference {
    return mItemOffset;
}

auto BitInputArchive::ConstIterator::operator->() const noexcept -> BitInputArchive::ConstIterator::pointer {
    return &mItemOffset;
}

BitInputArchive::ConstIterator::ConstIterator( std::uint32_t itemIndex, const BitInputArchive& itemArchive ) noexcept
    : mItemOffset( itemArchive, itemIndex ) {}

} // namespace bit7z