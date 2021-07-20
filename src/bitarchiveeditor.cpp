/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#include "bitexception.hpp"
#include "bitarchiveeditor.hpp"
#include "updatecallback.hpp"
#include "genericitem.hpp"
#include "fsitem.hpp"
#include "bufferitem.hpp"
#include "streamitem.hpp"
#include "util.hpp"

using bit7z::BitArchiveEditor;
using bit7z::BitException;
using bit7z::BitInFormat;
using bit7z::BitPropVariant;
using bit7z::UpdateCallback;

BitArchiveEditor::BitArchiveEditor( const Bit7zLibrary& lib,
                                    const tstring& in_file,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitArchiveCreator( lib, format, password, UpdateMode::APPEND ),
      BitOutputArchive( *this, in_file ) {
    if ( mInputArchive != nullptr ) {
        return; // Input file was correctly read by base class BitOutputArchive constructor
    }

    /* Note: BitOutputArchive doesn't require an input file, but BitArchiveEditor does! */
    if ( in_file.empty() ) {
        throw BitException( "Invalid archive path", std::make_error_code( std::errc::invalid_argument ) );
    }

    /* Note: if we are here, a non-empty in_file was specified, but BitOutputArchive constructor
     *       left a nullptr mInputArchive.
     *       This means that in_file doesn't exist (see BitOutputArchive's constructor).
     *       There's no need to check again for its existence (e.g., using fs::exists). */
    throw BitException( "Could not open archive",
                        std::make_error_code( std::errc::no_such_file_or_directory ),
                        in_file );
}

BitArchiveEditor::~BitArchiveEditor() = default;

void BitArchiveEditor::renameItem( unsigned index, const tstring& new_path ) {
    checkIndex( index );
    mRenamedItems[ index ] = new_path;
}

void BitArchiveEditor::renameItem( const tstring& old_path, const tstring& new_path ) {
    mRenamedItems[ findItem( old_path ) ] = new_path;
}

void BitArchiveEditor::updateItem( unsigned int index, const tstring& in_file ) {
    checkIndex( index );
    auto item_name = mInputArchive->getItemProperty( index, BitProperty::Path );
    mUpdatedItems[ index ] = std::make_unique< FSItem >( in_file, item_name.getString() );
}

void BitArchiveEditor::updateItem( unsigned int index, const std::vector< byte_t >& in_buffer ) {
    checkIndex( index );
    auto item_name = mInputArchive->getItemProperty( index, BitProperty::Path );
    mUpdatedItems[ index ] = std::make_unique< BufferItem >( in_buffer, item_name.getString() );
}

void BitArchiveEditor::updateItem( unsigned int index, istream& in_stream ) {
    checkIndex( index );
    auto item_name = mInputArchive->getItemProperty( index, BitProperty::Path );
    mUpdatedItems[ index ] = std::make_unique< StreamItem >( in_stream, item_name.getString() );
}

void BitArchiveEditor::updateItem( const tstring& item_path, const tstring& in_file ) {
    mUpdatedItems[ findItem( item_path ) ] = std::make_unique< FSItem >( in_file, item_path );
}

void BitArchiveEditor::updateItem( const tstring& item_path, const std::vector< byte_t >& in_buffer ) {
    mUpdatedItems[ findItem( item_path ) ] = std::make_unique< BufferItem >( in_buffer, item_path );
}

void BitArchiveEditor::updateItem( const tstring& item_path, std::istream& in_stream ) {
    mUpdatedItems[ findItem( item_path ) ] = std::make_unique< StreamItem >( in_stream, item_path );
}

void BitArchiveEditor::deleteItem( unsigned int index ) {
    if ( index >= mInputArchiveItemsCount ) {
        throw BitException( "Invalid index " + std::to_string( index ),
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    mRenamedItems.erase( index );
    mUpdatedItems.erase( index );
    mDeletedItems.insert( index );
}

void BitArchiveEditor::deleteItem( const tstring& item_path ) {
    for ( const auto& archiveItem : *mInputArchive ) {
        if ( archiveItem.path() == item_path ) {
            mRenamedItems.erase( archiveItem.index() );
            mUpdatedItems.erase( archiveItem.index() );
            mDeletedItems.insert( archiveItem.index() );
            return;
        }
    }
    throw BitException( "Could not find the file in the archive",
                        std::make_error_code( std::errc::no_such_file_or_directory ), item_path );
}

void BitArchiveEditor::setUpdateMode( UpdateMode update_mode ) {
    if ( update_mode == UpdateMode::NONE ) {
        throw BitException( "BitArchiveEditor doesn't support setting update mode to UpdateMode::NONE",
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    BitArchiveCreator::setUpdateMode( update_mode );
}

void BitArchiveEditor::applyChanges() {
    if ( mNewItemsVector.size() == 0 && mRenamedItems.empty() && mUpdatedItems.empty() && mDeletedItems.empty() ) {
        // Nothing to do here!
        return;
    }
    auto archive_path = mInputArchive->getArchivePath();
    compressTo( archive_path );
    mRenamedItems.clear();
    mInputArchive = std::make_unique< BitInputArchive >( *this, archive_path );
}

uint32_t BitArchiveEditor::findItem( const tstring& item_path ) {
    auto archiveItem = mInputArchive->find( item_path );
    if ( archiveItem == mInputArchive->cend() ) {
        throw BitException( "Could not find the file in the archive",
                            std::make_error_code( std::errc::no_such_file_or_directory ), item_path );
    }
    if ( mDeletedItems.find( archiveItem->index() ) != mDeletedItems.end() ) {
        throw BitException( "Cannot edit deleted item",
                            std::make_error_code( std::errc::invalid_argument ), item_path );
    }
    return archiveItem->index();
}

void BitArchiveEditor::checkIndex( uint32_t index ) {
    if ( index >= mInputArchiveItemsCount ) {
        throw BitException( "Invalid index " + std::to_string( index ),
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    if ( mDeletedItems.find( index ) != mDeletedItems.end() ) {
        throw BitException( "Cannot edit deleted item at index " + std::to_string( index ),
                            std::make_error_code( std::errc::invalid_argument ) );
    }
}

BitPropVariant BitArchiveEditor::getItemProperty( input_index index, PROPID propID ) const {
    auto mapped_index = static_cast< uint32_t >( index );
    if ( mapped_index < mInputArchiveItemsCount ) {
        if ( propID == kpidPath ) { // Renamed by the user
            auto res = mRenamedItems.find( mapped_index );
            if ( res != mRenamedItems.end() ) {
                return BitPropVariant{ WIDEN( res->second ) };
            }
        }
        auto res = mUpdatedItems.find( mapped_index );
        if ( res != mUpdatedItems.end() ) {
            return res->second->getProperty( propID );
        }
        return mInputArchive->getItemProperty( mapped_index, static_cast< BitProperty >( propID ) );
    }
    return BitOutputArchive::getItemProperty( index, propID );
}

HRESULT BitArchiveEditor::getItemStream( input_index index, ISequentialInStream** inStream ) const {
    auto mapped_index = static_cast< uint32_t >( index );
    if ( mapped_index < mInputArchiveItemsCount ) { //old item in the archive
        auto res = mUpdatedItems.find( mapped_index );
        if ( res != mUpdatedItems.end() ) { //user wants to update the old item in the archive
            return res->second->getStream( inStream );
        }
        return S_OK;
    }
    return BitOutputArchive::getItemStream( index, inStream );
}

bool BitArchiveEditor::hasNewData( uint32_t index ) const {
    auto mapped_index = static_cast< uint32_t >( getItemInputIndex( index ) );
    return mapped_index >= mInputArchiveItemsCount || mUpdatedItems.find( mapped_index ) != mUpdatedItems.end();
}

bool BitArchiveEditor::hasNewProperties( uint32_t index ) const {
    auto mapped_index = static_cast< uint32_t >( getItemInputIndex( index ) );
    bool isRenamedItem = mRenamedItems.find( mapped_index ) != mRenamedItems.end();
    bool isUpdatedItem = mUpdatedItems.find( mapped_index ) != mUpdatedItems.end();
    return mapped_index >= mInputArchiveItemsCount || isRenamedItem || isUpdatedItem;
}
