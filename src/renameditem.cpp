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

#include "renameditem.hpp"

#include "fsutil.hpp"

using bit7z::RenamedItem;
using bit7z::tstring;
using bit7z::BitPropVariant;
using namespace bit7z::filesystem;

RenamedItem::RenamedItem( const BitInputArchive& input_archive, uint32_t index, tstring new_path )
    : mInputArchive{ input_archive }, mIndex{ index }, mNewPath{ std::move( new_path ) } {}

tstring RenamedItem::name() const { return fsutil::filename( mNewPath, true ); }

fs::path RenamedItem::path() const { return mNewPath; }

fs::path RenamedItem::inArchivePath() const { return path(); }

BitPropVariant RenamedItem::getProperty( BitProperty propID ) const {
    if ( propID == bit7z::BitProperty::Path ) {
        return BitPropVariant{ inArchivePath().wstring() };
    }
    return mInputArchive.getItemProperty( mIndex, propID );
}

HRESULT RenamedItem::getStream( ISequentialInStream** ) const {
    return S_OK;
}

bool RenamedItem::hasNewData() const {
    return false; //just a new property (i.e., path/name), no new data!
}