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

#include "../include/bitarchiveinfo.hpp"

#include <algorithm>
#include <numeric>

#include <7zip/PropID.h>

using namespace bit7z;

BitArchiveInfo::BitArchiveInfo( const Bit7zLibrary& lib,
                                const tstring& in_file,
                                const BitInFormat& format,
                                const tstring& password )
    : BitArchiveOpener( lib, format, password ), BitInputArchive( *this, in_file ) {}

BitArchiveInfo::BitArchiveInfo( const Bit7zLibrary& lib,
                                const vector< byte_t >& in_buffer,
                                const BitInFormat& format,
                                const tstring& password )
    : BitArchiveOpener( lib, format, password ), BitInputArchive( *this, in_buffer ) {}

BitArchiveInfo::BitArchiveInfo( const Bit7zLibrary& lib,
                                std::istream& in_stream,
                                const BitInFormat& format,
                                const tstring& password )
    : BitArchiveOpener( lib, format, password ), BitInputArchive( *this, in_stream ) {}

map< BitProperty, BitPropVariant > BitArchiveInfo::archiveProperties() const {
    map< BitProperty, BitPropVariant > result;
    for ( uint32_t i = kpidNoProperty; i <= kpidCopyLink; ++i ) {
        // Yeah, I know, I double cast property (here and in getArchiveProperty), but the code is easier to read!
        auto property = static_cast< BitProperty >( i );
        BitPropVariant property_value = getArchiveProperty( property );
        if ( !property_value.isEmpty() ) {
            result[ property ] = property_value;
        }
    }
    return result;
}

vector< BitArchiveItemInfo > BitArchiveInfo::items() const {
    vector< BitArchiveItemInfo > result;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitArchiveItemInfo item( i );
        for ( uint32_t j = kpidNoProperty; j <= kpidCopyLink; ++j ) {
            // Yeah, I know, I double cast property (here and in getItemProperty), but the code is easier to read!
            auto property = static_cast<BitProperty>( j );
            BitPropVariant property_value = getItemProperty( i, property );
            if ( !property_value.isEmpty() ) {
                item.setProperty( property, property_value );
            }
        }
        result.push_back( item );
    }
    return result;
}

uint32_t BitArchiveInfo::foldersCount() const {
    return std::count_if( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return item.isDir();
    } );
}

uint32_t BitArchiveInfo::filesCount() const {
    return itemsCount() - foldersCount(); //I'm lazy :)
}

uint64_t BitArchiveInfo::size() const {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.size();
    } );
}

uint64_t BitArchiveInfo::packSize() const {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.packSize();
    } );
}

bool BitArchiveInfo::hasEncryptedItems() const {
    /* Note: simple encryption (i.e. not including the archive headers) can be detected only reading
     *       the properties of the files in the archive, so we search for any encrypted file inside the archive! */
    return std::any_of( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return !item.isDir() && item.isEncrypted();
    } );
}

bool BitArchiveInfo::isMultiVolume() const {
    if ( mFormat == BitFormat::Split ) {
        return true;
    }
    BitPropVariant is_multi_volume = getArchiveProperty( BitProperty::IsVolume );
    return is_multi_volume.isBool() && is_multi_volume.getBool();
}

bool BitArchiveInfo::isSolid() const {
    BitPropVariant is_solid = getArchiveProperty( BitProperty::Solid );
    return is_solid.isBool() && is_solid.getBool();
}

uint32_t BitArchiveInfo::volumesCount() const {
    BitPropVariant volumes_count = getArchiveProperty( BitProperty::NumVolumes );
    return volumes_count.isEmpty() ? 1 : volumes_count.getUInt32();
}
