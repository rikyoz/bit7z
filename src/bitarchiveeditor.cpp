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

#include "../include/bitexception.hpp"
#include "../include/bitarchiveeditor.hpp"

#include "../include/fileupdatecallback.hpp"

using bit7z::BitArchiveEditor;
using bit7z::BitException;
using bit7z::BitInFormat;

BitArchiveEditor::BitArchiveEditor( const Bit7zLibrary& lib,
                                    const tstring& in_file,
                                    const BitInOutFormat& format,
                                    const tstring& password )
                                    : BitArchiveCreator( lib, format, password ),
                                      mInputArchive( std::make_unique<BitInputArchive>( *this, in_file ) ) {
    mUpdateMode = true;
}

void BitArchiveEditor::renameItem( unsigned index, const tstring& new_name ) {
    if ( index >= mInputArchive->itemsCount() ) {
        throw BitException( "Invalid index " + std::to_string(index), std::make_error_code( std::errc::invalid_argument ) );
    }
    mRenameMap[ index ] = new_name;
}

void BitArchiveEditor::renameItem( const tstring& old_name, const tstring& new_name ) {
    for ( const auto& archiveItem : *mInputArchive ) {
        if ( archiveItem.name() == old_name ) {
            mRenameMap[ archiveItem.index() ] = new_name;
            return;
        }
    }
    throw BitException("Could not find the file in the archive",
                       std::make_error_code( std::errc::no_such_file_or_directory ), { old_name } );
}

void BitArchiveEditor::applyChanges() {
    auto archive_path = mInputArchive->getArchivePath();
    mInputArchive.reset();
    CMyComPtr< UpdateCallback > update_callback = new FileUpdateCallback( *this, {} );
    update_callback->setRenamedItems( mRenameMap );
    BitArchiveCreator::compressToFile( archive_path, update_callback );
    mRenameMap.clear();
    mInputArchive = std::make_unique<BitInputArchive>( *this, archive_path );
}
