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

#include "internal/dateutil.hpp"
#include "internal/fsutil.hpp"
#include "internal/renameditem.hpp"
#include "internal/util.hpp"

namespace bit7z {

RenamedItem::RenamedItem( const BitInputArchive& input_archive, uint32_t index, const tstring& new_path )
    : mInputArchive{ input_archive }, mIndex{ index }, mNewPath{ new_path } {}

auto RenamedItem::name() const -> tstring {
    return path_to_tstring( mNewPath.filename() );
}

auto RenamedItem::path() const -> tstring {
    return path_to_tstring( mNewPath );
}

auto RenamedItem::inArchivePath() const -> fs::path { return path(); }

auto RenamedItem::getStream( ISequentialInStream** /*inStream*/ ) const noexcept -> HRESULT {
    return S_OK;
}

auto RenamedItem::hasNewData() const noexcept -> bool {
    return false; //just a new property (i.e., path/name), no new data!
}

auto RenamedItem::isDir() const -> bool {
    return mInputArchive.itemProperty( mIndex, BitProperty::IsDir ).getBool();
}

auto RenamedItem::size() const -> uint64_t {
    return mInputArchive.itemProperty( mIndex, BitProperty::Size ).getUInt64();
}

auto RenamedItem::creationTime() const -> FILETIME {
    const BitPropVariant creation_time = mInputArchive.itemProperty( mIndex, BitProperty::CTime );
    return creation_time.isFileTime() ? creation_time.getFileTime() : currentFileTime();
}

auto RenamedItem::lastAccessTime() const -> FILETIME {
    const BitPropVariant access_time = mInputArchive.itemProperty( mIndex, BitProperty::ATime );
    return access_time.isFileTime() ? access_time.getFileTime() : currentFileTime();
}

auto RenamedItem::lastWriteTime() const -> FILETIME {
    const BitPropVariant write_time = mInputArchive.itemProperty( mIndex, BitProperty::MTime );
    return write_time.isFileTime() ? write_time.getFileTime() : currentFileTime();
}

auto RenamedItem::attributes() const -> uint32_t {
    return mInputArchive.itemProperty( mIndex, BitProperty::Attrib ).getUInt32();
}

} // namespace bit7z