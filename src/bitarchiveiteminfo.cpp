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

#include "bitarchiveiteminfo.hpp"
#include "bitarchiveitem.hpp"
#include "bitpropvariant.hpp"

#include <cstdint>
#include <map>

namespace bit7z {

BitArchiveItemInfo::BitArchiveItemInfo( uint32_t itemIndex ) : BitArchiveItem( itemIndex ) {}

auto BitArchiveItemInfo::itemProperty( BitProperty property ) const -> BitPropVariant {
    const auto propIt = mItemProperties.find( property );
    return ( propIt != mItemProperties.end() ? ( *propIt ).second : BitPropVariant() );
}

auto BitArchiveItemInfo::itemProperties() const -> std::map< BitProperty, BitPropVariant > {
    return mItemProperties;
}

void BitArchiveItemInfo::setProperty( BitProperty property, const BitPropVariant& value ) {
    mItemProperties[ property ] = value;
}

} // namespace bit7z