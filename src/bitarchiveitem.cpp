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

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

BitArchiveItem::BitArchiveItem( uint32_t item_index ) : mItemIndex( item_index ) {}

BitArchiveItem::~BitArchiveItem() {}

uint32_t BitArchiveItem::index() const {
    return mItemIndex;
}

bool BitArchiveItem::isDir() const {
    BitPropVariant propvar = getProperty( BitProperty::IsDir );
    return !propvar.isEmpty() && propvar.getBool();
}

wstring BitArchiveItem::name() const {
    BitPropVariant propvar = getProperty( BitProperty::Name );
    if ( propvar.isEmpty() ) {
        propvar = getProperty( BitProperty::Path );
        return propvar.isEmpty() ? L"" : fsutil::filename( propvar.getString(), true );
    }
    return propvar.getString();
}

wstring BitArchiveItem::extension() const {
    if ( isDir() ) {
        return L"";
    }
    BitPropVariant propvar = getProperty( BitProperty::Extension );
    return propvar.isEmpty() ? fsutil::extension( name() ) : propvar.getString();
}

wstring BitArchiveItem::path() const {
    BitPropVariant propvar = getProperty( BitProperty::Path );
    if ( propvar.isEmpty() ) {
        propvar = getProperty( BitProperty::Name );
        return propvar.isEmpty() ? L"" : propvar.getString();
    }
    return propvar.getString();
}

uint64_t BitArchiveItem::size() const {
    BitPropVariant propvar = getProperty( BitProperty::Size );
    return propvar.isEmpty() ? 0 : propvar.getUInt64();
}

uint64_t BitArchiveItem::packSize() const {
    BitPropVariant propvar = getProperty( BitProperty::PackSize );
    return propvar.isEmpty() ? 0 : propvar.getUInt64();
}

bool BitArchiveItem::isEncrypted() const {
    BitPropVariant propvar = getProperty( BitProperty::Encrypted );
    return propvar.isBool() && propvar.getBool();
}

BitPropVariant BitArchiveItem::getProperty( BitProperty property ) const {
    auto prop_it = mItemProperties.find( property );
    return ( prop_it != mItemProperties.end() ? ( *prop_it ).second : BitPropVariant() );
}

map< BitProperty, BitPropVariant > BitArchiveItem::itemProperties() const {
    return mItemProperties;
}

void BitArchiveItem::setProperty( BitProperty property, const BitPropVariant& value ) {
    mItemProperties[ property ] = value;
}
