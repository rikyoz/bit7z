// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitoutputarchive.hpp"

#include "../include/bitexception.hpp"
#include "../include/fs.hpp"
#include "../include/fsitem.hpp"
#include "../include/fsindexer.hpp"
#include "../include/bufferitem.hpp"
#include "../include/streamitem.hpp"
#include "../include/updatecallback.hpp"
#include "../include/cmultivoloutstream.hpp"
#include "../include/cbufferoutstream.hpp"

using bit7z::BitException;
using bit7z::BitOutputArchive;
using bit7z::BitArchiveCreator;
using bit7z::filesystem::FSItem;
using bit7z::BufferItem;
using bit7z::StreamItem;
using bit7z::filesystem::FSIndexer;
using bit7z::UpdateCallback;
using bit7z::byte_t;
using bit7z::tstring;

BitOutputArchive::BitOutputArchive( const BitArchiveCreator& creator, tstring in_file )
    : mArchiveCreator{ creator } {
    std::error_code ec;
    if ( !in_file.empty() && fs::exists( in_file, ec ) ) {
        if ( !mArchiveCreator.updateMode() ) {
            throw BitException( "Archive creator cannot update archives",
                                std::make_error_code( std::errc::invalid_argument ) );
        }
        if ( !mArchiveCreator.compressionFormat().hasFeature( FormatFeatures::MULTIPLE_FILES ) ) {
            //Update mode is set but format does not support adding more files
            throw BitException( "Format does not support updating existing archive files",
                                std::make_error_code( std::errc::invalid_argument ) );
        }
        mInputArchive = std::make_unique< BitInputArchive >( creator, in_file );
    }
}

BitOutputArchive::BitOutputArchive( const BitArchiveCreator& creator,
                                    const std::vector< bit7z::byte_t >& in_buffer )
    : mArchiveCreator{ creator } {
    if ( !in_buffer.empty() ) {
        mInputArchive = std::make_unique< BitInputArchive >( creator, in_buffer );
    }
}

BitOutputArchive::BitOutputArchive( const BitArchiveCreator& creator, std::istream& in_stream )
    : mArchiveCreator{ creator } {
    mInputArchive = std::make_unique< BitInputArchive >( creator, in_stream );
}

void BitOutputArchive::addItems( const std::vector< tstring >& in_paths ) {
    mNewItemsIndex.indexPaths( in_paths );
}

void BitOutputArchive::addItems( const std::map< tstring, tstring >& in_paths ) {
    mNewItemsIndex.indexPathsMap( in_paths );
}

void BitOutputArchive::addFile( const tstring& in_file ) {
    mNewItemsIndex.indexFile( in_file );
}

void BitOutputArchive::addFile( const std::vector< byte_t >& in_buffer, const tstring& name ) {
    mNewItemsIndex.indexBuffer( in_buffer, name );
}

void BitOutputArchive::addFile( std::istream& in_stream, const tstring& name ) {
    mNewItemsIndex.indexStream( in_stream, name );
}

void BitOutputArchive::addFiles( const std::vector< tstring >& in_files ) {
    mNewItemsIndex.indexPaths( in_files, true );
}


void BitOutputArchive::addFiles( const tstring& in_dir, bool recursive, const tstring& filter ) {
    mNewItemsIndex.indexDirectory( in_dir, filter, recursive );
}

void BitOutputArchive::addDirectory( const tstring& in_dir ) {
    mNewItemsIndex.indexDirectory( in_dir );
}

void BitOutputArchive::compressTo( const tstring& out_file ) const {
    CMyComPtr< UpdateCallback > update_callback = initUpdateCallback();
    compressToFile( out_file, update_callback );
}

CMyComPtr< IOutArchive > BitOutputArchive::initOutArchive() const {
    CMyComPtr< IOutArchive > new_arc;
    if ( mInputArchive == nullptr ) {
        const GUID format_GUID = mArchiveCreator.format().guid();
        mArchiveCreator.library()
                       .createArchiveObject( &format_GUID, &::IID_IOutArchive, reinterpret_cast< void** >( &new_arc ) );
    } else {
        mInputArchive->initUpdatableArchive( &new_arc );
    }
    setArchiveProperties( new_arc );
    return new_arc;
}

CMyComPtr< IOutStream > BitOutputArchive::initOutFileStream( const tstring& out_archive,
                                                             bool updating_archive ) const {
    if ( mArchiveCreator.volumeSize() > 0 ) {
        return new CMultiVolOutStream( mArchiveCreator.volumeSize(), out_archive );
    }

    fs::path out_path = out_archive;
    if ( updating_archive ) {
        out_path += ".tmp";
    }

    auto* file_out_stream = new CFileOutStream( out_path, updating_archive );
    CMyComPtr< IOutStream > out_stream = file_out_stream;
    if ( file_out_stream->fail() ) {
        //Unknown error!
        throw BitException( "Cannot create output archive file", last_error_code(), out_path.native() );
    }
    return out_stream;
}

void BitOutputArchive::compressOut( IOutArchive* out_arc,
                                    IOutStream* out_stream,
                                    UpdateCallback* update_callback ) const {
    HRESULT result = out_arc->UpdateItems( out_stream, update_callback->itemsCount(), update_callback );

    if ( result == E_NOTIMPL ) {
        throw BitException( kUnsupportedOperation, make_hresult_code( result ) );
    }

    if ( result != S_OK ) {
        update_callback->throwException( result );
    }
}

void BitOutputArchive::compressToFile( const tstring& out_file, UpdateCallback* update_callback ) const {
    // Note: if old_arc != nullptr, new_arc will actually point to the same IInArchive object used by old_arc
    // (see initUpdatableArchive function of BitInputArchive)!
    bool updating_archive = mInputArchive != nullptr && mInputArchive->getArchivePath() == out_file;
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_stream = initOutFileStream( out_file, updating_archive );
    compressOut( new_arc, out_stream, update_callback );

    if ( updating_archive ) { //we updated the input archive
        mInputArchive->close();
        /* NOTE: In the following instruction, we use the . (dot) operator, not the -> (arrow) operator:
         *       in fact, both CMyComPtr and IOutStream have a Release() method, and we need to call only
         *       the one of CMyComPtr (which in turns calls the one of IOutStream)! */
        out_stream.Release(); //Releasing the output stream so that we can rename it as the original file

#if defined( __MINGW32__ ) && defined( USE_STANDARD_FILESYSTEM )
        /* MinGW seems to not follow the standard since filesystem::rename does not overwrite an already
         * existing destination file (as it should). So we explicitly remove it before! */
        std::error_code ec;
        fs::remove( out_file, ec );
        if ( ec ) {
            throw BitException( "Cannot remove old archive file", ec, out_file );
        }
#endif

        //remove old file and rename tmp file (move file with overwriting)
        std::error_code error;
        fs::rename( out_file + TSTRING( ".tmp" ), out_file, error );
        if ( error ) {
            throw BitException( "Cannot rename temp archive file", error, out_file );
        }
    }
}

void BitOutputArchive::compressTo( std::vector< byte_t >& out_buffer ) const {
    if ( !out_buffer.empty() ) {
        throw BitException( kCannotOverwriteBuffer, std::make_error_code( std::errc::invalid_argument ) );
    }

    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_mem_stream = new CBufferOutStream( out_buffer );
    CMyComPtr< UpdateCallback > update_callback = initUpdateCallback();
    compressOut( new_arc, out_mem_stream, update_callback );
}

void BitOutputArchive::compressTo( std::ostream& out_stream ) const {
    CMyComPtr< IOutArchive > new_arc = initOutArchive();
    CMyComPtr< IOutStream > out_std_stream = new CStdOutStream( out_stream );
    CMyComPtr< UpdateCallback > update_callback = initUpdateCallback();
    compressOut( new_arc, out_std_stream, update_callback );
}

void BitOutputArchive::setArchiveProperties( IOutArchive* out_archive ) const {
    ArchiveProperties properties = mArchiveCreator.getArchiveProperties();
    if ( !properties.names.empty() ) {
        CMyComPtr< ISetProperties > set_properties;
        if ( out_archive->QueryInterface( ::IID_ISetProperties,
                                          reinterpret_cast< void** >( &set_properties ) ) != S_OK ) {
            throw BitException( "ISetProperties unsupported", std::make_error_code( std::errc::not_supported ) );
        }
        if ( set_properties->SetProperties( properties.names.data(), properties.values.data(),
                                            static_cast< uint32_t >( properties.names.size() ) ) != S_OK ) {
            throw BitException( "Cannot set properties of the archive",
                                std::make_error_code( std::errc::invalid_argument ) );
        }
    }
}

CMyComPtr< UpdateCallback > BitOutputArchive::initUpdateCallback() const {
    CMyComPtr< UpdateCallback > update_callback = new UpdateCallback( mArchiveCreator, mNewItemsIndex );
    update_callback->setOldArc( mInputArchive.get() );
    return update_callback;
}
