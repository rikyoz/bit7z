// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/renameditem.hpp"

#include "internal/dateutil.hpp"
#include "internal/fsutil.hpp"

namespace bit7z {
RenamedItem::RenamedItem( const BitInputArchive& input_archive, uint32_t index, const tstring& new_path )
    : mInputArchive{ input_archive }, mIndex{ index }, mNewPath{ new_path } {}

tstring RenamedItem::name() const {
    return mNewPath.filename().string< tchar >();
}

tstring RenamedItem::path() const {
    return mNewPath.string< tchar >();
}

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
    BitPropVariant creation_time = mInputArchive.itemProperty( mIndex, BitProperty::CTime );
    return creation_time.isFileTime() ? creation_time.getFileTime() : currentFileTime();
}

FILETIME RenamedItem::lastAccessTime() const {
    BitPropVariant access_time = mInputArchive.itemProperty( mIndex, BitProperty::ATime );
    return access_time.isFileTime() ? access_time.getFileTime() : currentFileTime();
}

FILETIME RenamedItem::lastWriteTime() const {
    BitPropVariant write_time = mInputArchive.itemProperty( mIndex, BitProperty::MTime );
    return write_time.isFileTime() ? write_time.getFileTime() : currentFileTime();
}

uint32_t bit7z::RenamedItem::attributes() const {
    return mInputArchive.itemProperty( mIndex, BitProperty::Attrib ).getUInt32();
}
}