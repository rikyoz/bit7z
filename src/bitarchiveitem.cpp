// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
        return fsutil::filename( path(), true );
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
    return propvar.isEmpty() ? L"" : propvar.getString();
}

uint64_t BitArchiveItem::size() const {
    BitPropVariant propvar = getProperty( BitProperty::Size );
    return propvar.isEmpty() ? 0 : propvar.getUint64();
}

uint64_t BitArchiveItem::packSize() const {
    BitPropVariant propvar = getProperty( BitProperty::PackSize );
    return propvar.isEmpty() ? 0 : propvar.getUint64();
}

BitPropVariant BitArchiveItem::getProperty( BitProperty property ) const {
    auto prop_it = mItemProperties.find( property );
    return ( prop_it != mItemProperties.end() ? ( *prop_it ).second : BitPropVariant() );
}

map<BitProperty, BitPropVariant> BitArchiveItem::itemProperties() const {
    return mItemProperties;
}

void BitArchiveItem::setProperty( BitProperty property, const BitPropVariant& value ) {
    mItemProperties[ property ] = value;
}
