// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitarchiveitem.hpp"

#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitArchiveItem::BitArchiveItem( uint32_t item_index ) : mItemIndex( item_index ) {}

uint32_t BitArchiveItem::index() const {
    return mItemIndex;
}

bool BitArchiveItem::isDir() const {
    BitPropVariant is_dir = getProperty( BitProperty::IsDir );
    return !is_dir.isEmpty() && is_dir.getBool();
}

tstring BitArchiveItem::name() const {
    BitPropVariant name = getProperty( BitProperty::Name );
    if ( name.isEmpty() ) {
        name = getProperty( BitProperty::Path );
        return name.isEmpty() ? TSTRING( "" ) : fsutil::filename( name.getString(), true );
    }
    return name.getString();
}

tstring BitArchiveItem::extension() const {
    if ( isDir() ) {
        return TSTRING( "" );
    }
    BitPropVariant extension = getProperty( BitProperty::Extension );
    return extension.isEmpty() ? fsutil::extension( name() ) : extension.getString();
}

tstring BitArchiveItem::path() const {
    BitPropVariant path = getProperty( BitProperty::Path );
    if ( path.isEmpty() ) {
        path = getProperty( BitProperty::Name );
        return path.isEmpty() ? TSTRING( "" ) : path.getString();
    }
    return path.getString();
}

uint64_t BitArchiveItem::size() const {
    BitPropVariant size = getProperty( BitProperty::Size );
    return size.isEmpty() ? 0 : size.getUInt64();
}

uint64_t BitArchiveItem::packSize() const {
    BitPropVariant pack_size = getProperty( BitProperty::PackSize );
    return pack_size.isEmpty() ? 0 : pack_size.getUInt64();
}

bool BitArchiveItem::isEncrypted() const {
    BitPropVariant is_encrypted = getProperty( BitProperty::Encrypted );
    return is_encrypted.isBool() && is_encrypted.getBool();
}
