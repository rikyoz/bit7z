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

#include "bitarchiveeditor.hpp"

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/bufferitem.hpp"
#include "internal/fsitem.hpp"
#include "internal/renameditem.hpp"
#include "internal/stdinputitem.hpp"
#include "internal/stringutil.hpp"

namespace bit7z {

using std::istream;

BitArchiveEditor::BitArchiveEditor( const Bit7zLibrary& lib,
                                    const tstring& inFile,
                                    const BitInOutFormat& format,
                                    const tstring& password )
    : BitArchiveWriter( lib, inFile, format, password ) {
    if ( inputArchive() != nullptr ) {
        return; // Input file was correctly read by base class BitOutputArchive constructor
    }

    /* Note: BitArchiveWriter doesn't require an input file, but BitArchiveEditor does! */
    if ( inFile.empty() ) {
        throw BitException( "Could not open archive", make_error_code( BitError::InvalidArchivePath ) );
    }

    /* Note: if we are here, a non-empty inFile was specified, but the BitOutputArchive constructor
     *       left a nullptr mInputArchive.
     *       This means that inFile doesn't exist (see BitOutputArchive's constructor).
     *       There's no need to check again for its existence (e.g., using fs::exists). */
    throw BitException( "Could not open archive",
                        std::make_error_code( std::errc::no_such_file_or_directory ),
                        inFile );
}

BitArchiveEditor::~BitArchiveEditor() = default;

void BitArchiveEditor::renameItem( uint32_t index, const tstring& newPath ) {
    checkIndex( index );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *inputArchive(), index, newPath ); //-V108
}

void BitArchiveEditor::renameItem( const tstring& oldPath, const tstring& newPath ) {
    auto index = findItem( oldPath );
    mEditedItems[ index ] = std::make_unique< RenamedItem >( *inputArchive(), index, newPath ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, const tstring& inFile ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< FilesystemItem >( tstring_to_path( inFile ), itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, const std::vector< byte_t >& inBuffer ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< BufferItem >( inBuffer, itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( uint32_t index, std::istream& inStream ) {
    checkIndex( index );
    auto itemName = inputArchive()->itemProperty( index, BitProperty::Path );
    mEditedItems[ index ] = std::make_unique< StdInputItem >( inStream, itemName.getNativeString() ); //-V108
}

void BitArchiveEditor::updateItem( const tstring& itemPath, const tstring& inFile ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< FilesystemItem >( tstring_to_path( inFile ), //-V108
                                                                               tstring_to_path( itemPath ) );
}

void BitArchiveEditor::updateItem( const tstring& itemPath, const std::vector< byte_t >& inBuffer ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< BufferItem >( inBuffer, itemPath ); //-V108
}

void BitArchiveEditor::updateItem( const tstring& itemPath, std::istream& inStream ) {
    mEditedItems[ findItem( itemPath ) ] = std::make_unique< StdInputItem >( inStream, itemPath ); //-V108
}

void BitArchiveEditor::deleteItem( uint32_t index ) {
    if ( index >= inputArchiveItemsCount() ) {
        throw BitException( "Cannot delete item at index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    mEditedItems.erase( index );
    setDeletedIndex( index );
}

void BitArchiveEditor::deleteItem( const tstring& itemPath ) {
    auto res = std::find_if( inputArchive()->cbegin(), inputArchive()->cend(),
                             // Note: we don't use auto for the parameter type to support compilation with GCC 4.9
                             [ &, this ]( const BitArchiveItemOffset& archiveItem ) {
                                 if ( archiveItem.path() == itemPath ) {
                                     mEditedItems.erase( archiveItem.index() );
                                     setDeletedIndex( archiveItem.index() );
                                     return true;
                                 }
                                 return false;
                             } );
    if ( res == inputArchive()->cend() ) {
        throw BitException( "Could not mark the item as deleted",
                            std::make_error_code( std::errc::no_such_file_or_directory ), itemPath );
    }
}

void BitArchiveEditor::setUpdateMode( UpdateMode mode ) {
    if ( mode == UpdateMode::None ) {
        throw BitException( "Cannot set update mode to UpdateMode::None",
                            std::make_error_code( std::errc::invalid_argument ) );
    }
    BitAbstractArchiveCreator::setUpdateMode( mode );
}

void BitArchiveEditor::applyChanges() {
    if ( !hasNewItems() && mEditedItems.empty() && !hasDeletedIndexes() ) {
        // Nothing to do here!
        return;
    }
    auto archivePath = inputArchive()->archivePath();
    compressTo( archivePath );
    mEditedItems.clear();
    setInputArchive( std::make_unique< BitInputArchive >( *this, archivePath ) );
}

auto BitArchiveEditor::findItem( const tstring& itemPath ) -> uint32_t {
    auto archiveItem = inputArchive()->find( itemPath );
    if ( archiveItem == inputArchive()->cend() ) {
        throw BitException( "Could not find the file in the archive",
                            std::make_error_code( std::errc::no_such_file_or_directory ), itemPath );
    }
    if ( isDeletedIndex( archiveItem->index() ) ) {
        throw BitException( "Could not find item",
                            make_error_code( BitError::ItemMarkedAsDeleted ), itemPath );
    }
    return archiveItem->index();
}

void BitArchiveEditor::checkIndex( uint32_t index ) {
    if ( index >= inputArchiveItemsCount() ) {
        throw BitException( "Cannot edit item at the index " + std::to_string( index ),
                            make_error_code( BitError::InvalidIndex ) );
    }
    if ( isDeletedIndex( index ) ) {
        throw BitException( "Cannot edit item at the index " + std::to_string( index ),
                            make_error_code( BitError::ItemMarkedAsDeleted ) );
    }
}

auto BitArchiveEditor::itemProperty( InputIndex index, BitProperty property ) const -> BitPropVariant {
    const auto mappedIndex = static_cast< uint32_t >( index );
    if ( mappedIndex < inputArchiveItemsCount() ) {
        auto res = mEditedItems.find( mappedIndex );
        if ( res != mEditedItems.end() ) {
            return res->second->itemProperty( property );
        }
        return inputArchive()->itemProperty( mappedIndex, property );
    }
    return BitOutputArchive::itemProperty( index, property );
}

auto BitArchiveEditor::itemStream( InputIndex index, ISequentialInStream** inStream ) const -> HRESULT {
    const auto mappedIndex = static_cast< uint32_t >( index );
    if ( mappedIndex < inputArchiveItemsCount() ) { //old item in the archive
        auto res = mEditedItems.find( mappedIndex );
        if ( res != mEditedItems.end() ) { //user wants to update the old item in the archive
            return res->second->getStream( inStream );
        }
        return S_OK;
    }
    return BitOutputArchive::itemStream( index, inStream );
}

auto BitArchiveEditor::hasNewData( uint32_t index ) const noexcept -> bool {
    const auto mappedIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    if ( mappedIndex >= inputArchiveItemsCount() ) {
        return true; //new item
    }
    auto editedItem = mEditedItems.find( mappedIndex );
    if ( editedItem != mEditedItems.end() ) {
        return editedItem->second->hasNewData(); //renamed item -> false (no new data), updated item -> true
    }
    return false;
}

auto BitArchiveEditor::hasNewProperties( uint32_t index ) const noexcept -> bool {
    const auto mappedIndex = static_cast< uint32_t >( itemInputIndex( index ) );
    const bool isEditedItem = mEditedItems.find( mappedIndex ) != mEditedItems.end();
    return mappedIndex >= inputArchiveItemsCount() || isEditedItem;
}

} // namespace bit7z
