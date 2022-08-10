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

#include "bitarchiveitem.hpp"

#include "internal/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitArchiveItem::BitArchiveItem( uint32_t item_index ) noexcept
    : mItemIndex( item_index ) {}

uint32_t BitArchiveItem::index() const noexcept {
    return mItemIndex;
}

bool BitArchiveItem::isDir() const {
    BitPropVariant is_dir = itemProperty( BitProperty::IsDir );
    return !is_dir.isEmpty() && is_dir.getBool();
}

tstring BitArchiveItem::name() const {
    BitPropVariant name = itemProperty( BitProperty::Name );
    if ( name.isEmpty() ) {
        name = itemProperty( BitProperty::Path );
        return name.isEmpty() ? tstring{} : fsutil::filename( name.getString(), true );
    }
    return name.getString();
}

tstring BitArchiveItem::extension() const {
    if ( isDir() ) {
        return tstring{};
    }
    BitPropVariant extension = itemProperty( BitProperty::Extension );
    return extension.isEmpty() ? fsutil::extension( name() ) : extension.getString();
}

tstring BitArchiveItem::path() const {
    BitPropVariant path = itemProperty( BitProperty::Path );
    if ( path.isEmpty() ) {
        path = itemProperty( BitProperty::Name );
        return path.isEmpty() ? tstring{} : path.getString();
    }
    return path.getString();
}

uint64_t BitArchiveItem::size() const {
    BitPropVariant size = itemProperty( BitProperty::Size );
    return size.isEmpty() ? 0 : size.getUInt64();
}

uint64_t BitArchiveItem::packSize() const {
    BitPropVariant pack_size = itemProperty( BitProperty::PackSize );
    return pack_size.isEmpty() ? 0 : pack_size.getUInt64();
}

bool BitArchiveItem::isEncrypted() const {
    BitPropVariant is_encrypted = itemProperty( BitProperty::Encrypted );
    return is_encrypted.isBool() && is_encrypted.getBool();
}

time_type BitArchiveItem::creationTime() const {
    BitPropVariant creation_time = itemProperty( BitProperty::CTime );
    return creation_time.isFileTime() ? creation_time.getTimePoint() : time_type::clock::now();
}

time_type BitArchiveItem::lastAccessTime() const {
    BitPropVariant access_time = itemProperty( BitProperty::ATime );
    return access_time.isFileTime() ? access_time.getTimePoint() : time_type::clock::now();
}

time_type BitArchiveItem::lastWriteTime() const {
    BitPropVariant write_time = itemProperty( BitProperty::MTime );
    return write_time.isFileTime() ? write_time.getTimePoint() : time_type::clock::now();
}

uint32_t BitArchiveItem::attributes() const {
    BitPropVariant attrib = itemProperty( BitProperty::Attrib );
    return attrib.isUInt32() ? attrib.getUInt32() : 0;
}
