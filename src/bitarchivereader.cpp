// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitarchivereader.hpp"

#include "7zip/PropID.h"

#include "../include/bitexception.hpp"
#include "../include/util.hpp"

using namespace bit7z;
using namespace bit7z::util;

BitArchiveReader::BitArchiveReader( const Bit7zLibrary& lib, const wstring& in_file, const BitInFormat& format )
    : BitArchiveOpener( lib, format ) {
    mInArchive = openArchive( mLibrary, mFormat, in_file, *this ).Detach();
}

BitArchiveReader::~BitArchiveReader() {
    if ( mInArchive ) {
        mInArchive->Release();
    }
}

BitPropVariant BitArchiveReader::getArchiveProperty( BitProperty property ) const {
    BitPropVariant propvar;
    HRESULT res = mInArchive->GetArchiveProperty( static_cast<PROPID>( property ), &propvar );
    if ( res != S_OK ) {
        throw BitException( L"Could not retrieve archive property " +
                            propertyNames[ static_cast<PROPID>(  property ) ] );
    }
    return propvar;
}

BitPropVariant BitArchiveReader::getItemProperty( uint32_t index, BitProperty property ) const {
    BitPropVariant propvar;
    HRESULT res = mInArchive->GetProperty( index, static_cast<PROPID>( property ), &propvar );
    if ( res != S_OK ) {
        throw BitException( L"Could not retrieve property " +
                            propertyNames[ static_cast<PROPID>(  property ) ] +
                            L"for item " + std::to_wstring( index ) );
    }
    return propvar;
}

map<BitProperty, BitPropVariant> BitArchiveReader::archiveProperties() const {
    map<BitProperty, BitPropVariant> result;
    for ( uint32_t i = kpidNoProperty; i <= kpidCopyLink; ++i ) {
        // Yeah, I know, I double cast property (here and in getArchiveProperty), but the code is easier to read!
        auto property = static_cast<BitProperty>( i );
        BitPropVariant propertyValue = getArchiveProperty( property );
        if ( !propertyValue.isEmpty() ) {
            result[ property ] = propertyValue;
        }
    }
    return result;
}

vector<BitArchiveItem> BitArchiveReader::items() const {
    vector<BitArchiveItem> result;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitArchiveItem item( i );
        for ( uint32_t j = kpidNoProperty; j <= kpidCopyLink; ++j ) {
            // Yeah, I know, I double cast property (here and in getItemProperty), but the code is easier to read!
            auto property = static_cast<BitProperty>( j );
            BitPropVariant propertyValue = getItemProperty( i, property );
            if ( !propertyValue.isEmpty() ) {
                item.setProperty( property, propertyValue );
            }
        }
        result.push_back( item );
    }
    return result;
}

uint32_t BitArchiveReader::itemsCount() const {
    uint32_t items_count;
    HRESULT result = mInArchive->GetNumberOfItems( &items_count );
    if ( result != S_OK ) {
        throw BitException( L"Could not retrieve the number of items in the archive" );
    }
    return items_count;
}

uint32_t BitArchiveReader::foldersCount() const {
    uint32_t result = 0;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitPropVariant prop = getItemProperty( i, BitProperty::IsDir );
        if ( !prop.isEmpty() && prop.getBool() ) {
            result += 1;
        }
    }
    return result;
}

uint32_t BitArchiveReader::filesCount() const {
    return itemsCount() - foldersCount(); //I'm lazy :)
}

uint64_t BitArchiveReader::size() const {
    uint64_t result = 0;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitPropVariant prop = getItemProperty( i, BitProperty::Size );
        if ( !prop.isEmpty() ) {
            result += prop.getUint64();
        }
    }
    return result;
}

uint64_t BitArchiveReader::packSize() const {
    uint64_t result = 0;
    for ( uint32_t i = 0; i < itemsCount(); ++i ) {
        BitPropVariant prop = getItemProperty( i, BitProperty::PackSize );
        if ( !prop.isEmpty() ) {
            result += prop.getUint64();
        }
    }
    return result;
}
