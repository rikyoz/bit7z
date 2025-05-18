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

#include "bitoutputarchive.hpp"

#include "bitabstractarchivecreator.hpp"
#include "bitabstractarchivehandler.hpp"
#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitformat.hpp"
#include "bitinputarchive.hpp"
#include "bititemsvector.hpp"
#include "bitpropvariant.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"
#include "internal/archiveproperties.hpp"
#include "internal/cbufferoutstream.hpp"
#include "internal/cfileoutstream.hpp"
#include "internal/cmultivolumeoutstream.hpp"
#include "internal/cstdoutstream.hpp"
#include "internal/fsutil.hpp"
#include "internal/updatecallback.hpp"
#include "internal/util.hpp"

#include <cstddef>
#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <utility>
#include <system_error>
#include <vector>

namespace bit7z {

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const tstring& inFile,
                                    ArchiveStartOffset startOffset )
    : BitOutputArchive( creator, tstring_to_path( inFile ), startOffset ) {}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const fs::path& inArc,
                                    ArchiveStartOffset archiveStart )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( mArchiveCreator.overwriteMode() != OverwriteMode::None ) {
        return;
    }

    if ( inArc.empty() ) { // No input file specified, so we are creating a totally new archive.
        return;
    }

    std::error_code error;
    if ( !fs::exists( inArc, error ) ) { // An input file was specified, but it doesn't exist, so we ignore it.
        return;
    }

    if ( mArchiveCreator.updateMode() == UpdateMode::None ) {
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::WrongUpdateMode ) );
    }

    // TODO: Add support for updating single-file bz2, gz, and xz archives when updated archive still contains one file.
    if ( !mArchiveCreator.compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        //Update mode is set, but the format does not support adding more files.
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::FormatFeatureNotSupported ) );
    }

    mInputArchive = std::make_unique< BitInputArchive >( creator, inArc, archiveStart );
    mInputArchiveItemsCount = mInputArchive->itemsCount();
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const buffer_t& inBuffer,
                                    ArchiveStartOffset startOffset )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( !inBuffer.empty() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, inBuffer, startOffset );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    std::istream& inStream,
                                    ArchiveStartOffset startOffset )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( inStream.good() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, inStream, startOffset );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

BitOutputArchive::~BitOutputArchive() = default;

void BitOutputArchive::addItems( const std::vector< tstring >& inPaths ) {
    IndexingOptions options{};
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexPaths( mNewItems, inPaths, options );
}

void BitOutputArchive::addItems( const std::map< tstring, tstring >& inPaths ) {
    IndexingOptions options{};
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexPathsMap( mNewItems, inPaths, options );
}

void BitOutputArchive::addFile( const tstring& inFile, const tstring& name ) {
    const auto policy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexFile( mNewItems, inFile, mArchiveCreator.retainDirectories() ? inFile : name, policy );
}

void BitOutputArchive::addFile( const buffer_t& inBuffer, const tstring& name ) {
    indexBuffer( mNewItems, inBuffer, name );
}

void BitOutputArchive::addFile( std::istream& inStream, const tstring& name ) {
    indexStream( mNewItems, inStream, name );
}

void BitOutputArchive::addFiles( const std::vector< tstring >& inFiles ) {
    IndexingOptions options{};
    options.recursive = false;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.onlyFiles = true;
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexPaths( mNewItems, inFiles, options );
}

void BitOutputArchive::addFiles( const tstring& inDir, const tstring& filter, bool recursive ) {
    addFiles( inDir, filter, FilterPolicy::Include, recursive );
}

void BitOutputArchive::addFiles( const tstring& inDir, const tstring& filter, FilterPolicy policy, bool recursive ) {
    IndexingOptions options{};
    options.filterPolicy = policy;
    options.recursive = recursive;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.onlyFiles = true;
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexDirectory( mNewItems, tstring_to_path( inDir ), filter, options );
}

void BitOutputArchive::addDirectory( const tstring& inDir ) {
    IndexingOptions options{};
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    indexDirectory( mNewItems, tstring_to_path( inDir ), {}, options );
}

void BitOutputArchive::addDirectoryContents( const tstring& inDir, const tstring& filter, bool recursive ) {
    addDirectoryContents( inDir, filter, FilterPolicy::Include, recursive );
}

void BitOutputArchive::addDirectoryContents( const tstring& inDir,
                                             const tstring& filter,
                                             FilterPolicy policy,
                                             bool recursive ) {
    IndexingOptions options{};
    options.filterPolicy = policy;
    options.recursive = recursive;
    options.onlyFiles = !recursive;
    options.retainFolderStructure = mArchiveCreator.retainDirectories();
    options.symlinkPolicy = !mArchiveCreator.storeSymbolicLinks() ? SymlinkPolicy::Follow : SymlinkPolicy::DoNotFollow;
    std::error_code error;
    indexDirectory( mNewItems, fs::absolute( tstring_to_path( inDir ), error ), filter, options );
}

auto BitOutputArchive::initOutArchive() const -> CMyComPtr< IOutArchive > {
    CMyComPtr< IOutArchive > newArc;
    if ( mInputArchive == nullptr ) {
        newArc = mArchiveCreator.library().initOutArchive( mArchiveCreator.compressionFormat() );
    } else {
        const auto res = mInputArchive->initUpdatableArchive( &newArc );
        if ( res != S_OK ) {
            throw BitException{ "Could not make the input archive object updatable", make_hresult_code( res ) };
        }
    }
    setArchiveProperties( newArc );
    return newArc;
}

auto BitOutputArchive::initOutFileStream( const fs::path& outArchive,
                                          bool updatingArchive ) const -> CMyComPtr< IOutStream > {
    if ( mArchiveCreator.volumeSize() > 0 ) {
        return bit7z::make_com< CMultiVolumeOutStream, IOutStream >( mArchiveCreator.volumeSize(), outArchive );
    }

    if ( !updatingArchive ) { // No need to create a copy of the output archive path.
        return bit7z::make_com< CFileOutStream, IOutStream >( outArchive, updatingArchive );
    }

    fs::path tmpArchive = outArchive;
    tmpArchive += ".tmp";
    return bit7z::make_com< CFileOutStream, IOutStream >( tmpArchive, updatingArchive );
}

void BitOutputArchive::compressOut( IOutArchive* outArc,
                                    IOutStream* outStream,
                                    UpdateCallback* updateCallback ) {
    if ( mInputArchive != nullptr && mArchiveCreator.updateMode() == UpdateMode::Update ) {
        for ( const auto& newItem : mNewItems ) {
            auto newItemPath = path_to_tstring( newItem.inArchivePath() );
            auto updatedItem = mInputArchive->find( newItemPath );
            if ( updatedItem != mInputArchive->cend() ) {
                setDeletedIndex( updatedItem->index() );
            }
        }
    }
    updateInputIndices();

    const HRESULT result = outArc->UpdateItems( outStream, itemsCount(), updateCallback );

    if ( result == E_NOTIMPL ) {
        throw BitException( "Unsupported operation", bit7z::make_hresult_code( result ) );
    }

    if ( result != S_OK ) {
        throw BitException( "Error while compressing files", make_hresult_code( result ), std::move( mFailedFiles ) );
    }
}

void BitOutputArchive::compressToFile( const fs::path& outFile, UpdateCallback* updateCallback ) {
    // Note: if mInputArchive is not nullptr, newArc will actually point to the same IInArchive object
    // used by the old_arc (see initUpdatableArchive function of BitInputArchive).
    const bool updatingArchive = mInputArchive != nullptr && tstring_to_path( mInputArchive->archivePath() ) == outFile;
    const CMyComPtr< IOutArchive > newArc = initOutArchive();
    CMyComPtr< IOutStream > outStream = initOutFileStream( outFile, updatingArchive );
    compressOut( newArc, outStream, updateCallback );

    if ( updatingArchive ) { //we updated the input archive
        const auto closeResult = mInputArchive->close();
        if ( closeResult != S_OK ) {
            throw BitException( "Failed to close the archive", make_hresult_code( closeResult ),
                                mInputArchive->archivePath() );
        }
        /* NOTE: In the following instruction, we use the (dot) operator, not the -> (arrow) operator:
         *       in fact, both CMyComPtr and IOutStream have a Release() method, and we need to call only
         *       the one of CMyComPtr (which in turns calls the one of IOutStream)! */
        outStream.Release(); //Releasing the output stream so that we can rename it as the original file.

        std::error_code error;
#if defined( __MINGW32__ ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
        /* MinGW seems to not follow the standard since filesystem::rename does not overwrite an already
         * existing destination file (as it should). So we explicitly remove it before! */
        if ( !fs::remove( outFile, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, path_to_tstring( outFile ) );
        }
#endif

        //remove the old file and rename the temporary file (move file with overwriting)
        fs::path tmpFile = outFile;
        tmpFile += ".tmp";
        fs::rename( tmpFile, outFile, error );
        if ( error ) {
            throw BitException( "Failed to overwrite the old archive file", error, path_to_tstring( outFile ) );
        }
    }
}

void BitOutputArchive::compressTo( const tstring& outFile ) {
    const fs::path outPath = tstring_to_path( outFile );
    std::error_code error;
    if ( fs::exists( outPath, error ) ) {
        const OverwriteMode overwriteMode = mArchiveCreator.overwriteMode();
        if ( overwriteMode == OverwriteMode::Skip ) { // Skipping if the output file already exists
            return;
        }
        if ( overwriteMode == OverwriteMode::Overwrite && !fs::remove( outPath, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, outFile );
        }
        // Note: if overwriteMode is OverwriteMode::None, an exception will be thrown by the CFileOutStream constructor
        // called by the initOutFileStream function.
    }

    const auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressToFile( outPath, updateCallback );
}

void BitOutputArchive::compressTo( buffer_t& outBuffer ) {
    if ( !outBuffer.empty() ) {
        const OverwriteMode overwriteMode = mArchiveCreator.overwriteMode();
        if ( overwriteMode == OverwriteMode::Skip ) {
            return;
        }
        if ( overwriteMode == OverwriteMode::Overwrite ) {
            outBuffer.clear();
        } else {
            throw BitException( "Cannot compress to buffer", make_error_code( BitError::NonEmptyOutputBuffer ) );
        }
    }

    const auto newArc = initOutArchive();
    const auto outMemStream = bit7z::make_com< CBufferOutStream, IOutStream >( outBuffer );
    const auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( newArc, outMemStream, updateCallback );
}

void BitOutputArchive::compressTo( std::ostream& outStream ) {
    const auto newArc = initOutArchive();
    const auto outStdStream = bit7z::make_com< CStdOutStream, IOutStream >( outStream );
    const auto updateCallback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( newArc, outStdStream, updateCallback );
}

void BitOutputArchive::setArchiveProperties( IOutArchive* outArchive ) const {
    const ArchiveProperties properties = mArchiveCreator.archiveProperties();
    if ( properties.empty() ) {
        return;
    }

    CMyComPtr< ISetProperties > setProperties;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    HRESULT res = outArchive->QueryInterface( ::IID_ISetProperties, reinterpret_cast< void** >( &setProperties ) );
    if ( res != S_OK ) {
        throw BitException( "ISetProperties unsupported", make_hresult_code( res ) );
    }
    res = setProperties->SetProperties( properties.names(),
                                        properties.values(),
                                        static_cast< std::uint32_t >( properties.size() ) );
    if ( res != S_OK ) {
        throw BitException( "Cannot set properties of the archive", make_hresult_code( res ) );
    }
}

void BitOutputArchive::updateInputIndices() {
    if ( mDeletedItems.empty() ) {
        return;
    }

    std::uint32_t offset = 0;
    for ( std::uint32_t newIndex = 0; newIndex < itemsCount(); ++newIndex ) {
        for ( auto it = mDeletedItems.find( newIndex + offset );
              it != mDeletedItems.end() && *it == newIndex + offset;
              ++it ) {
            ++offset;
        }
        mInputIndices.push_back( static_cast< InputIndex >( newIndex + offset ) );
    }
}

auto BitOutputArchive::itemsCount() const -> std::uint32_t {
    auto result = static_cast< std::uint32_t >( mNewItems.size() );
    if ( mInputArchive != nullptr ) {
        result += mInputArchive->itemsCount() - static_cast< std::uint32_t >( mDeletedItems.size() );
    }
    return result;
}

auto BitOutputArchive::itemProperty( InputIndex index, BitProperty property ) const -> BitPropVariant {
    const auto newItemIndex = static_cast< std::size_t >( index ) - static_cast< std::size_t >( mInputArchiveItemsCount );
    const auto& newItem = mNewItems[ newItemIndex ];
    return newItem.itemProperty( property );
}

auto BitOutputArchive::itemStream( InputIndex index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto newItemIndex = static_cast< std::size_t >( index ) - static_cast< std::size_t >( mInputArchiveItemsCount );
    const auto& newItem = mNewItems[ newItemIndex ];

    const HRESULT res = newItem.getStream( inStream );
    if ( FAILED( res ) ) {
        mFailedFiles.emplace_back( to_tstring( newItem.path() ), make_hresult_code( res ) );
    }
    return res;
}

auto BitOutputArchive::hasNewData( std::uint32_t index ) const noexcept -> bool {
    const auto originalIndex = static_cast< std::uint32_t >( itemInputIndex( index ) );
    return originalIndex >= mInputArchiveItemsCount;
}

auto BitOutputArchive::hasNewProperties( std::uint32_t index ) const noexcept -> bool {
    /* Note: in BitOutputArchive, you can only add new items or overwrite (delete + add) existing ones.
     * So if we have new data, we also have new properties; this is not true for BitArchiveEditor. */
    return hasNewData( index );
}

auto BitOutputArchive::itemInputIndex( std::uint32_t newIndex ) const noexcept -> InputIndex {
    const auto index = static_cast< decltype( mInputIndices )::size_type >( newIndex );
    if ( index < mInputIndices.size() ) {
        return mInputIndices[ index ];
    }
    // if we are here, the user didn't delete any item, so the InputIndex is essentially equivalent to the newIndex
    return static_cast< InputIndex >( newIndex );
}

auto BitOutputArchive::outputItemProperty( std::uint32_t index, BitProperty property ) const -> BitPropVariant {
    const auto mappedIndex = itemInputIndex( index );
    return itemProperty( mappedIndex, property );
}

auto BitOutputArchive::outputItemStream( std::uint32_t index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto mappedIndex = itemInputIndex( index );
    return itemStream( mappedIndex, inStream );
}

auto BitOutputArchive::indexInArchive( std::uint32_t index ) const noexcept -> std::uint32_t {
    const auto originalIndex = static_cast< std::uint32_t >( itemInputIndex( index ) );
    return originalIndex < mInputArchiveItemsCount ? originalIndex : static_cast< std::uint32_t >( -1 );
}

auto BitOutputArchive::handler() const noexcept -> const BitAbstractArchiveHandler& {
    return mArchiveCreator;
}

auto BitOutputArchive::creator() const noexcept -> const BitAbstractArchiveCreator& {
    return mArchiveCreator;
}

} // namespace bit7z