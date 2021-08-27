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

#include "internal/renameditem.hpp"

#include "internal/fsutil.hpp"

using bit7z::BitInputArchive;
using bit7z::BitPropVariant;
using bit7z::RenamedItem;
using bit7z::tstring;
using namespace bit7z::filesystem;

RenamedItem::RenamedItem( const BitInputArchive& input_archive, uint32_t index, tstring new_path )
    : mInputArchive{ input_archive }, mIndex{ index }, mNewPath{ std::move( new_path ) } {}

tstring RenamedItem::name() const { return mNewPath.filename(); }

tstring RenamedItem::path() const { return mNewPath; }

fs::path RenamedItem::inArchivePath() const { return path(); }

HRESULT RenamedItem::getStream( ISequentialInStream** ) const noexcept {
    return S_OK;
}

bool RenamedItem::hasNewData() const noexcept {
    return false; //just a new property (i.e., path/name), no new data!
}

bool RenamedItem::isDir() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::IsDir ).getBool();
}

uint64_t RenamedItem::size() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::Size ).getUInt64();
}

FILETIME RenamedItem::creationTime() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::CTime ).getFileTime();
}

FILETIME RenamedItem::lastAccessTime() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::ATime ).getFileTime();
}

FILETIME RenamedItem::lastWriteTime() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::MTime ).getFileTime();
}

uint32_t bit7z::RenamedItem::attributes() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::Attrib ).getUInt32();
}
