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

#include "bitarchiveitemoffset.hpp"

#include "bitinputarchive.hpp"

using namespace bit7z;

BitArchiveItemOffset::BitArchiveItemOffset( uint32_t item_index, const BitInputArchive& item_arc ) noexcept
    : BitArchiveItem( item_index ), mArc( &item_arc ) {}

BitArchiveItemOffset& BitArchiveItemOffset::operator++() noexcept {
    ++mItemIndex;
    return *this;
}

BitArchiveItemOffset BitArchiveItemOffset::operator++( int ) noexcept {
    BitArchiveItemOffset old_value = *this;
    ++( *this );
    return old_value;
}

bool BitArchiveItemOffset::operator==( const BitArchiveItemOffset& other ) const noexcept {
    return mItemIndex == other.mItemIndex;
}

bool BitArchiveItemOffset::operator!=( const BitArchiveItemOffset& other ) const noexcept {
    return !( *this == other );
}

BitPropVariant BitArchiveItemOffset::itemProperty( BitProperty property ) const {
    return mArc != nullptr ? mArc->itemProperty( mItemIndex, property ) : BitPropVariant();
}
