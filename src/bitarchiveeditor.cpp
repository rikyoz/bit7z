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

#include "bitarchiveeditor.hpp"

#include "bitexception.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsitem.hpp"
#include "internal/renameditem.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/updatecallback.hpp"

using bit7z::BitArchiveEditor;
using bit7z::BitException;
using bit7z::BitInFormat;
using bit7z::BitPropVariant;
using bit7z::UpdateCallback;

BitArchiveEditor::BitArchiveEditor( const Bit7zLibrary& lib,
                                    const tstring& in_file,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitArchiveWriter( lib, in_file, format, password ) {
    if ( mInputArchive != nullptr ) {
        return; // Input file was correctly read by base class BitOutputArchive constructor
    }

    /* Note: BitArchiveWriter doesn't require an input file, but BitArchiveEditor does! */
    if ( in_file.empty() ) {
        throw BitException( "Could not open archive", make_error_code( BitError::InvalidArchivePath ) );
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

void BitArchiveEditor::renameItem( uint32_t index, const tstring& new_path ) {
    checkIndex( index );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *mInputArchive, index, new_path );
}

void BitArchiveEditor::renameItem( const tstring& old_path, const tstring& new_path ) {
    auto index = findItem( old_path );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *mInputArchive, index, new_path );
}

void BitArchiveEditor::updateItem( uint32_t index, const tstring& in_file ) {
    checkIndex( index );
    auto item_name = mInputArchive->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< FSItem >( in_file, item_name.getString() );
}

void BitArchiveEditor::updateItem( uint32_t index, const std::vector< byte_t >& in_buffer ) {
    checkIndex( index );
    auto item_name = mInputArchive->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< BufferItem >( in_buffer, item_name.getString() );
}

void BitArchiveEditor::updateItem( uint32_t index, istream& in_stream ) {
    checkIndex( index );
    auto item_name = mInputArchive->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< StdInputItem >( in_stream, item_name.getString() );
}

void BitArchiveEditor::updateItem( const tstring& item_path, const tstring& in_file ) {
    mEditedItems[ findItem( item_path ) ] = std::make_unique< FSItem >( in_file, item_path );
}

void BitArchiveEditor::updateItem( const tstring& item_path, const std::vector< byte_t >& in_buffer ) {
    mEditedItems[ findItem( item_path ) ] = std::make_unique< BufferItem >( in_buffer, item_path );
}

void BitArchiveEditor::updateItem( const tstring& item_path, std::istream& in_stream ) {
    mEditedItems[ findItem( item_path ) ] = std::make_unique< StdInputItem >( in_stream, item_path );
}

void BitArchiveEditor::deleteItem( uint32_t index ) {
    if ( index >= mInputArchiveItemsCount ) {
        throw BitException( "Cannot delete item at index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    mEditedItems.erase( index );
    mDeletedItems.insert( index );
}

void BitArchiveEditor::deleteItem( const tstring& item_path ) {
    for ( const auto& archiveItem : *mInputArchive ) {
        if ( archiveItem.path() == item_path ) {
            mEditedItems.erase( archiveItem.index() );
            mDeletedItems.insert( archiveItem.index() );
            return;
        }
    }
    throw BitException( "Could not mark the item as deleted",
                        std::make_error_code( std::errc::no_such_file_or_directory ), item_path );
}

void BitArchiveEditor::setUpdateMode( UpdateMode update_mode ) {
    if ( update_mode == UpdateMode::None ) {
        throw BitException( "Cannot set update mode to UpdateMode::None",
                            make_error_code( BitError::UnsupportedOperation ) );
    }
    BitAbstractArchiveCreator::setUpdateMode( update_mode );
}

void BitArchiveEditor::applyChanges() {
    if ( mNewItemsVector.size() == 0 && mEditedItems.empty() && mDeletedItems.empty() ) {
        // Nothing to do here!
        return;
    }
    auto archive_path = mInputArchive->archivePath();
    compressTo( archive_path );
    mEditedItems.clear();
    mInputArchive = std::make_unique< BitInputArchive >( *this, archive_path );
}

uint32_t BitArchiveEditor::findItem( const tstring& item_path ) {
    auto archiveItem = mInputArchive->find( item_path );
    if ( archiveItem == mInputArchive->cend() ) {
        throw BitException( "Cannot find the file in the archive",
                            std::make_error_code( std::errc::no_such_file_or_directory ), item_path );
    }
    if ( mDeletedItems.find( archiveItem->index() ) != mDeletedItems.cend() ) {
        throw BitException( "Cannot edit item",
                            make_error_code( BitError::ItemMarkedAsDeleted ), item_path );
    }
    return archiveItem->index();
}

void BitArchiveEditor::checkIndex( uint32_t index ) {
    if ( index >= mInputArchiveItemsCount ) {
        throw BitException( "Cannot edit item at index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    if ( mDeletedItems.find( index ) != mDeletedItems.cend() ) {
        throw BitException( "Cannot edit item at index " + std::to_string( index ),
                            make_error_code( BitError::ItemMarkedAsDeleted ) );
    }
}

BitPropVariant BitArchiveEditor::itemProperty( input_index index, BitProperty propID ) const {
    const auto mapped_index = static_cast< uint32_t >( index );
    if ( mapped_index < mInputArchiveItemsCount ) {
        auto res = mEditedItems.find( mapped_index );
        if ( res != mEditedItems.end() ) {
            return res->second->itemProperty( propID );
        }
        return mInputArchive->itemProperty( mapped_index, static_cast< BitProperty >( propID ));
    }
    return BitOutputArchive::itemProperty( index, propID );
}

HRESULT BitArchiveEditor::itemStream( input_index index, ISequentialInStream** inStream ) const {
    const auto mapped_index = static_cast< uint32_t >( index );
    if ( mapped_index < mInputArchiveItemsCount ) { //old item in the archive
        auto res = mEditedItems.find( mapped_index );
        if ( res != mEditedItems.end() ) { //user wants to update the old item in the archive
            return res->second->getStream( inStream );
        }
        return S_OK;
    }
    return BitOutputArchive::itemStream( index, inStream );
}

bool BitArchiveEditor::hasNewData( uint32_t index ) const noexcept {
    const auto mapped_index = static_cast< uint32_t >( itemInputIndex( index ) );
    if ( mapped_index >= mInputArchiveItemsCount ) {
        return true; //new item
    }
    auto edited_item = mEditedItems.find( mapped_index );
    if ( edited_item != mEditedItems.end() ) {
        return edited_item->second->hasNewData(); //renamed item -> false (no new data), updated item -> true
    }
    return false;
}

bool BitArchiveEditor::hasNewProperties( uint32_t index ) const noexcept {
    const auto mapped_index = static_cast< uint32_t >( itemInputIndex( index ) );
    bool       isEditedItem = mEditedItems.find( mapped_index ) != mEditedItems.end();
    return mapped_index >= mInputArchiveItemsCount || isEditedItem;
}
