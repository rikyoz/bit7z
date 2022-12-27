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

#include "bitarchivereader.hpp"

#include <algorithm>
#include <numeric>

#include <7zip/PropID.h>

using namespace bit7z;

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const tstring& in_file,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, in_file ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const vector< byte_t >& in_buffer,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, in_buffer ) {}

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    std::istream& in_stream,
                                    const BitInFormat& format,
                                    const tstring& password )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, in_stream ) {}

/*BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib,
                                    const BitArchiveReader& reader,
                                    const BitInFormat& format,
                                    const tstring& password,
                                    size_t index )
    : BitAbstractArchiveOpener( lib, format, password ), BitInputArchive( *this, reader ) {}*/


map< BitProperty, BitPropVariant > BitArchiveReader::archiveProperties() const {
    map< BitProperty, BitPropVariant > result;
    for ( uint32_t i = kpidNoProperty; i <= kpidCopyLink; ++i ) {
        // Yeah, I know, I cast property twice (here and in archiveProperty), but the code is easier to read!
        const auto property = static_cast< BitProperty >( i );
        const BitPropVariant property_value = archiveProperty( property );
        if ( !property_value.isEmpty() ) {
            result[ property ] = property_value;
        }
    }
    return result;
}

vector< BitArchiveItemInfo > BitArchiveReader::items() const {
    vector< BitArchiveItemInfo > result;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitArchiveItemInfo item( i );
        for ( uint32_t j = kpidNoProperty; j <= kpidCopyLink; ++j ) {
            // Yeah, I know, I cast property twice (here and in itemProperty), but the code is easier to read!
            const auto property = static_cast< BitProperty >( j );
            const auto property_value = itemProperty( i, property );
            if ( !property_value.isEmpty() ) {
                item.setProperty( property, property_value );
            }
        }
        result.push_back( item );
    }
    return result;
}

uint32_t BitArchiveReader::foldersCount() const {
    return std::count_if( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return item.isDir();
    } );
}

uint32_t BitArchiveReader::filesCount() const {
    return itemsCount() - foldersCount(); //I'm lazy :)
}

uint64_t BitArchiveReader::size() const {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.size();
    } );
}

uint64_t BitArchiveReader::packSize() const {
    return std::accumulate( cbegin(), cend(), 0ull, []( uint64_t accumulator, const BitArchiveItem& item ) {
        return item.isDir() ? accumulator : accumulator + item.packSize();
    } );
}

bool BitArchiveReader::hasEncryptedItems() const {
    /* Note: simple encryption (i.e., not including the archive headers) can be detected only reading
     *       the properties of the files in the archive, so we search for any encrypted file inside the archive! */
    return std::any_of( cbegin(), cend(), []( const BitArchiveItem& item ) {
        return !item.isDir() && item.isEncrypted();
    } );
}

bool BitArchiveReader::isMultiVolume() const {
    if ( extractionFormat() == BitFormat::Split ) {
        return true;
    }
    const BitPropVariant is_multi_volume = archiveProperty( BitProperty::IsVolume );
    return is_multi_volume.isBool() && is_multi_volume.getBool();
}

bool BitArchiveReader::isSolid() const {
    const BitPropVariant is_solid = archiveProperty( BitProperty::Solid );
    return is_solid.isBool() && is_solid.getBool();
}

uint32_t BitArchiveReader::volumesCount() const {
    const BitPropVariant volumes_count = archiveProperty( BitProperty::NumVolumes );
    return volumes_count.isEmpty() ? 1 : volumes_count.getUInt32();
}