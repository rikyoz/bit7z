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

#include "bitarchiveitem.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitArchiveItem::BitArchiveItem( uint32_t item_index ) noexcept
    : mItemIndex( item_index ) {}

auto BitArchiveItem::index() const noexcept -> uint32_t {
    return mItemIndex;
}

auto BitArchiveItem::isDir() const -> bool {
    const BitPropVariant is_dir = itemProperty( BitProperty::IsDir );
    return !is_dir.isEmpty() && is_dir.getBool();
}

BIT7Z_NODISCARD
inline auto filename( const fs::path& path ) -> tstring {
    return path_to_tstring( path.filename() );
}

auto BitArchiveItem::name() const -> tstring {
    BitPropVariant name = itemProperty( BitProperty::Name );
    if ( name.isEmpty() ) {
        name = itemProperty( BitProperty::Path );
        return name.isEmpty() ? tstring{} : filename( name.getNativeString() );
    }
    return name.getString();
}

auto BitArchiveItem::extension() const -> tstring {
    if ( isDir() ) {
        return tstring{};
    }
    const BitPropVariant extension = itemProperty( BitProperty::Extension );
    return extension.isEmpty() ? fsutil::extension( name() ) : extension.getString();
}

auto BitArchiveItem::path() const -> tstring {
    BitPropVariant path = itemProperty( BitProperty::Path );
    if ( path.isEmpty() ) {
        path = itemProperty( BitProperty::Name );
        return path.isEmpty() ? tstring{} : path.getString();
    }
    return path.getString();
}

auto BitArchiveItem::size() const -> uint64_t {
    const BitPropVariant size = itemProperty( BitProperty::Size );
    return size.isEmpty() ? 0 : size.getUInt64();
}

auto BitArchiveItem::packSize() const -> uint64_t {
    const BitPropVariant pack_size = itemProperty( BitProperty::PackSize );
    return pack_size.isEmpty() ? 0 : pack_size.getUInt64();
}

auto BitArchiveItem::isEncrypted() const -> bool {
    const BitPropVariant is_encrypted = itemProperty( BitProperty::Encrypted );
    return is_encrypted.isBool() && is_encrypted.getBool();
}

auto BitArchiveItem::creationTime() const -> time_type {
    const BitPropVariant creation_time = itemProperty( BitProperty::CTime );
    return creation_time.isFileTime() ? creation_time.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::lastAccessTime() const -> time_type {
    const BitPropVariant access_time = itemProperty( BitProperty::ATime );
    return access_time.isFileTime() ? access_time.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::lastWriteTime() const -> time_type {
    const BitPropVariant write_time = itemProperty( BitProperty::MTime );
    return write_time.isFileTime() ? write_time.getTimePoint() : time_type::clock::now();
}

auto BitArchiveItem::attributes() const -> uint32_t {
    const BitPropVariant attrib = itemProperty( BitProperty::Attrib );
    return attrib.isUInt32() ? attrib.getUInt32() : 0;
}

auto BitArchiveItem::crc() const -> uint32_t {
    const BitPropVariant crc = itemProperty( BitProperty::CRC );
    return crc.isUInt32() ? crc.getUInt32() : 0;
}
