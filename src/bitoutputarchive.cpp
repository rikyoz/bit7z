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

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/archiveproperties.hpp"
#include "internal/cbufferoutstream.hpp"
#include "internal/cmultivolumeoutstream.hpp"
#include "internal/fsutil.hpp"
#include "internal/genericinputitem.hpp"
#include "internal/updatecallback.hpp"
#include "internal/util.hpp"

namespace bit7z {

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator, const tstring& in_file )
    : BitOutputArchive( creator, fs::path{ in_file } ) {}

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator, fs::path in_arc )
#else

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator, const fs::path& in_arc )
#endif
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( mArchiveCreator.overwriteMode() != OverwriteMode::None ) {
        return;
    }

    if ( in_arc.empty() ) { // No input file specified, so we are creating a totally new archive!
        return;
    }

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
    if ( filesystem::fsutil::should_format_long_path( in_arc ) ) {
        in_arc = filesystem::fsutil::format_long_path( in_arc );
    }
#endif

    std::error_code error;
    if ( !fs::exists( in_arc, error ) ) { // An input file was specified, but it doesn't exist, so we ignore it.
        return;
    }

    if ( mArchiveCreator.updateMode() == UpdateMode::None ) {
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::WrongUpdateMode ) );
    }

    if ( !mArchiveCreator.compressionFormat().hasFeature( FormatFeatures::MultipleFiles ) ) {
        //Update mode is set, but the format does not support adding more files.
        throw BitException( "Cannot update the existing archive",
                            make_error_code( BitError::FormatFeatureNotSupported ) );
    }

    mInputArchive = std::make_unique< BitInputArchive >( creator, in_arc );
    mInputArchiveItemsCount = mInputArchive->itemsCount();
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator,
                                    const std::vector< bit7z::byte_t >& in_buffer )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( !in_buffer.empty() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, in_buffer );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

BitOutputArchive::BitOutputArchive( const BitAbstractArchiveCreator& creator, std::istream& in_stream )
    : mArchiveCreator{ creator }, mInputArchiveItemsCount{ 0 } {
    if ( in_stream.good() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, in_stream );
        mInputArchiveItemsCount = mInputArchive->itemsCount();
    }
}

void BitOutputArchive::addItems( const std::vector< tstring >& in_paths ) {
    IndexingOptions options{};
    options.retain_folder_structure = mArchiveCreator.retainDirectories();
    mNewItemsVector.indexPaths( in_paths, options );
}

void BitOutputArchive::addItems( const std::map< tstring, tstring >& in_paths ) {
    mNewItemsVector.indexPathsMap( in_paths );
}

void BitOutputArchive::addFile( const tstring& in_file, const tstring& name ) {
    mNewItemsVector.indexFile( in_file, mArchiveCreator.retainDirectories() ? in_file : name );
}

void BitOutputArchive::addFile( const std::vector< byte_t >& in_buffer, const tstring& name ) {
    mNewItemsVector.indexBuffer( in_buffer, name );
}

void BitOutputArchive::addFile( std::istream& in_stream, const tstring& name ) {
    mNewItemsVector.indexStream( in_stream, name );
}

void BitOutputArchive::addFiles( const std::vector< tstring >& in_files ) {
    IndexingOptions options{};
    options.recursive = false;
    options.retain_folder_structure = mArchiveCreator.retainDirectories();
    options.only_files = true;
    mNewItemsVector.indexPaths( in_files, options );
}

void BitOutputArchive::addFiles( const tstring& in_dir, const tstring& filter, bool recursive ) {
    IndexingOptions options{};
    options.recursive = recursive;
    options.retain_folder_structure = mArchiveCreator.retainDirectories();
    options.only_files = true;
    mNewItemsVector.indexDirectory( in_dir, filter, options );
}

void BitOutputArchive::addDirectory( const tstring& in_dir ) {
    IndexingOptions options{};
    options.retain_folder_structure = mArchiveCreator.retainDirectories();
    mNewItemsVector.indexDirectory( in_dir, BIT7Z_STRING( "" ), options );
}

auto BitOutputArchive::initOutArchive() const -> CMyComPtr< IOutArchive > {
    CMyComPtr< IOutArchive > new_arc;
    if ( mInputArchive == nullptr ) {
        const GUID format_GUID = formatGUID( mArchiveCreator.format() );
        mArchiveCreator.library() // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                       .createArchiveObject( &format_GUID, &::IID_IOutArchive, reinterpret_cast< void** >( &new_arc ) );
    } else {
        mInputArchive->initUpdatableArchive( &new_arc );
    }
    setArchiveProperties( new_arc );
    return new_arc;
}

auto BitOutputArchive::initOutFileStream( const fs::path& out_archive,
                                          bool updating_archive ) const -> CMyComPtr< IOutStream > {
    if ( mArchiveCreator.volumeSize() > 0 ) {
        return bit7z::make_com< CMultiVolumeOutStream, IOutStream >( mArchiveCreator.volumeSize(), out_archive );
    }

    fs::path out_path = out_archive;
    if ( updating_archive ) {
        out_path += ".tmp";
    }

    return bit7z::make_com< CFileOutStream, IOutStream >( out_path, updating_archive );
}

void BitOutputArchive::compressOut( IOutArchive* out_arc,
                                    IOutStream* out_stream,
                                    UpdateCallback* update_callback ) {
    if ( mInputArchive != nullptr && mArchiveCreator.updateMode() == UpdateMode::Update ) {
        for ( const auto& new_item : mNewItemsVector ) {
            auto updated_item = mInputArchive->find( new_item->inArchivePath().string< tchar >() );
            if ( updated_item != mInputArchive->cend() ) {
                setDeletedIndex( updated_item->index() );
            }
        }
    }
    updateInputIndices();

    const HRESULT result = out_arc->UpdateItems( out_stream, itemsCount(), update_callback );

    if ( result == E_NOTIMPL ) {
        throw BitException( bit7z::kUnsupportedOperation, bit7z::make_hresult_code( result ) );
    }

    if ( result != S_OK ) {
        throw BitException( "Error while compressing files", make_hresult_code( result ), std::move( mFailedFiles ) );
    }
}

void BitOutputArchive::compressToFile( const fs::path& out_file, UpdateCallback* update_callback ) {
    // Note: if mInputArchive != nullptr, new_arc will actually point to the same IInArchive object used by the old_arc
    // (see initUpdatableArchive function of BitInputArchive)!
    const bool updating_archive = mInputArchive != nullptr && mInputArchive->archivePath() == out_file;
    const CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_stream = initOutFileStream( out_file, updating_archive );
    compressOut( new_arc, out_stream, update_callback );

    if ( updating_archive ) { //we updated the input archive
        auto close_result = mInputArchive->close();
        if ( close_result != S_OK ) {
            throw BitException( "Failed to close the archive", make_hresult_code( close_result ),
                                mInputArchive->archivePath() );
        }
        /* NOTE: In the following instruction, we use the (dot) operator, not the -> (arrow) operator:
         *       in fact, both CMyComPtr and IOutStream have a Release() method, and we need to call only
         *       the one of CMyComPtr (which in turns calls the one of IOutStream)! */
        out_stream.Release(); //Releasing the output stream so that we can rename it as the original file.

        std::error_code error;
#if defined( __MINGW32__ ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
        /* MinGW seems to not follow the standard since filesystem::rename does not overwrite an already
         * existing destination file (as it should). So we explicitly remove it before! */
        if ( !fs::remove( out_file, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, out_file.string< tchar >() );
        }
#endif

        //remove the old file and rename the temporary file (move file with overwriting)
        fs::path tmp_file = out_file;
        tmp_file += ".tmp";
        fs::rename( tmp_file, out_file, error );
        if ( error ) {
            throw BitException( "Failed to overwrite the old archive file", error, out_file.string< tchar >() );
        }
    }
}

void BitOutputArchive::compressTo( const tstring& out_file ) {
    using namespace bit7z::filesystem;
    const fs::path out_path = FORMAT_LONG_PATH( out_file );
    std::error_code error;
    if ( fs::exists( out_path, error ) ) {
        const OverwriteMode overwrite_mode = mArchiveCreator.overwriteMode();
        if ( overwrite_mode == OverwriteMode::Skip ) { // Skipping if the output file already exists
            return;
        }
        if ( overwrite_mode == OverwriteMode::Overwrite && !fs::remove( out_path, error ) ) {
            throw BitException( "Failed to delete the old archive file", error, out_file );
        }
        // Note: if overwrite_mode is OverwriteMode::None, an exception will be thrown by the CFileOutStream constructor
        // called by the initOutFileStream function.
    }

    auto update_callback = bit7z::make_com< UpdateCallback >( *this );
    compressToFile( out_path, update_callback );
}

void BitOutputArchive::compressTo( std::vector< byte_t >& out_buffer ) {
    if ( !out_buffer.empty() ) {
        const OverwriteMode overwrite_mode = mArchiveCreator.overwriteMode();
        if ( overwrite_mode == OverwriteMode::Skip ) {
            return;
        }
        if ( overwrite_mode == OverwriteMode::Overwrite ) {
            out_buffer.clear();
        } else {
            throw BitException( "Cannot compress to buffer", make_error_code( BitError::NonEmptyOutputBuffer ) );
        }
    }

    const CMyComPtr< IOutArchive > new_arc = initOutArchive();
    auto out_mem_stream = bit7z::make_com< CBufferOutStream, IOutStream >( out_buffer );
    auto update_callback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( new_arc, out_mem_stream, update_callback );
}

void BitOutputArchive::compressTo( std::ostream& out_stream ) {
    const CMyComPtr< IOutArchive > new_arc = initOutArchive();
    auto out_std_stream = bit7z::make_com< CStdOutStream, IOutStream >( out_stream );
    auto update_callback = bit7z::make_com< UpdateCallback >( *this );
    compressOut( new_arc, out_std_stream, update_callback );
}

void BitOutputArchive::setArchiveProperties( IOutArchive* out_archive ) const {
    const ArchiveProperties properties = mArchiveCreator.archiveProperties();
    if ( properties.empty() ) {
        return;
    }

    CMyComPtr< ISetProperties > set_properties;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    HRESULT res = out_archive->QueryInterface( ::IID_ISetProperties, reinterpret_cast< void** >( &set_properties ) );
    if ( res != S_OK ) {
        throw BitException( "ISetProperties unsupported", make_hresult_code( res ) );
    }
    res = set_properties->SetProperties( properties.names(),
                                         properties.values(),
                                         static_cast< uint32_t >( properties.size() ) );
    if ( res != S_OK ) {
        throw BitException( "Cannot set properties of the archive", make_hresult_code( res ) );
    }
}

void BitOutputArchive::updateInputIndices() {
    if ( mDeletedItems.empty() ) {
        return;
    }

    uint32_t offset = 0;
    for ( uint32_t new_index = 0; new_index < itemsCount(); ++new_index ) {
        for ( auto it = mDeletedItems.find( new_index + offset );
              it != mDeletedItems.end() && *it == new_index + offset;
              ++it ) {
            ++offset;
        }
        mInputIndices.push_back( static_cast< input_index >( new_index + offset ) );
    }
}

auto BitOutputArchive::itemsCount() const -> uint32_t {
    auto result = static_cast< uint32_t >( mNewItemsVector.size() );
    if ( mInputArchive != nullptr ) {
        result += mInputArchive->itemsCount() - static_cast< uint32_t >( mDeletedItems.size() );
    }
    return result;
}

auto BitOutputArchive::itemProperty( input_index index, BitProperty propID ) const -> BitPropVariant {
    const auto new_item_index = static_cast< size_t >( index ) - static_cast< size_t >( mInputArchiveItemsCount );
    const GenericInputItem& new_item = mNewItemsVector[ new_item_index ];
    return new_item.itemProperty( propID );
}

auto BitOutputArchive::itemStream( input_index index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto new_item_index = static_cast< size_t >( index ) - static_cast< size_t >( mInputArchiveItemsCount );
    const GenericInputItem& new_item = mNewItemsVector[ new_item_index ];

    const HRESULT res = new_item.getStream( inStream );
    if ( FAILED( res ) ) {
        auto path = new_item.path();
        std::error_code error;
        if ( fs::exists( path, error ) ) {
            error = std::make_error_code( std::errc::file_exists );
        }
        mFailedFiles.emplace_back( path, error );
    }
    return res;
}

auto BitOutputArchive::hasNewData( uint32_t index ) const noexcept -> bool {
    const auto original_index = static_cast< uint32_t >( itemInputIndex( index ) );
    return original_index >= mInputArchiveItemsCount;
}

auto BitOutputArchive::hasNewProperties( uint32_t index ) const noexcept -> bool {
    /* Note: in BitOutputArchive, you can only add new items or overwrite (delete + add) existing ones.
     * So if we have new data, we also have new properties! This is not true for BitArchiveEditor! */
    return hasNewData( index );
}

auto BitOutputArchive::itemInputIndex( uint32_t new_index ) const noexcept -> input_index {
    const auto index = static_cast< decltype( mInputIndices )::size_type >( new_index );
    if ( index < mInputIndices.size() ) {
        return mInputIndices[ index ];
    }
    // if we are here, the user didn't delete any item, so the input_index is essentially equivalent to the new_index
    return static_cast< input_index >( new_index );
}

auto BitOutputArchive::outputItemProperty( uint32_t index, BitProperty propID ) const -> BitPropVariant {
    const auto mapped_index = itemInputIndex( index );
    return itemProperty( mapped_index, propID );
}

auto BitOutputArchive::outputItemStream( uint32_t index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto mapped_index = itemInputIndex( index );
    return itemStream( mapped_index, inStream );
}

auto BitOutputArchive::indexInArchive( uint32_t index ) const noexcept -> uint32_t {
    const auto original_index = static_cast< uint32_t >( itemInputIndex( index ) );
    return original_index < mInputArchiveItemsCount ? original_index : static_cast< uint32_t >( -1 );
}

auto BitOutputArchive::handler() const noexcept -> const BitAbstractArchiveHandler& {
    return mArchiveCreator;
}

} // namespace bit7z