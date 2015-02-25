#include "../include/bitformat.hpp"

using namespace std;
using namespace bit7z;

const BitOutFormat      BitFormat::Zip( 0x01, L".zip", MULTIPLE_FILES | COMPRESSION_LEVEL | ENCRYPTION );
const BitOutFormat    BitFormat::BZip2( 0x02, L".bz2", COMPRESSION_LEVEL | INMEM_COMPRESSION );
const BitInFormat       BitFormat::Rar( 0x03 );
const BitInFormat       BitFormat::Arj( 0x04 );
const BitInFormat         BitFormat::Z( 0x05 );
const BitInFormat       BitFormat::Lzh( 0x06 );
const BitOutFormat BitFormat::SevenZip( 0x07, L".7z", MULTIPLE_FILES | SOLID_ARCHIVE | COMPRESSION_LEVEL | ENCRYPTION |
                                        HEADER_ENCRYPTION );
const BitInFormat       BitFormat::Cab( 0x08 );
const BitInFormat      BitFormat::Nsis( 0x09 );
const BitInFormat      BitFormat::Lzma( 0x0A );
const BitInFormat    BitFormat::Lzma86( 0x0B );
const BitOutFormat       BitFormat::Xz( 0x0C, L".xz",
                                        COMPRESSION_LEVEL | ENCRYPTION | HEADER_ENCRYPTION | INMEM_COMPRESSION );
const BitInFormat      BitFormat::Ppmd( 0x0D );
const BitInFormat        BitFormat::TE( 0xCF );
const BitInFormat     BitFormat::UEFIc( 0xD0 );
const BitInFormat     BitFormat::UEFIs( 0xD1 );
const BitInFormat  BitFormat::SquashFS( 0xD2 );
const BitInFormat    BitFormat::CramFS( 0xD3 );
const BitInFormat       BitFormat::APM( 0xD4 );
const BitInFormat      BitFormat::Mslz( 0xD5 );
const BitInFormat       BitFormat::Flv( 0xD6 );
const BitInFormat       BitFormat::Swf( 0xD7 );
const BitInFormat      BitFormat::Swfc( 0xD8 );
const BitInFormat      BitFormat::Ntfs( 0xD9 );
const BitInFormat       BitFormat::Fat( 0xDA );
const BitInFormat       BitFormat::Mbr( 0xDB );
const BitInFormat       BitFormat::Vhd( 0xDC );
const BitInFormat        BitFormat::Pe( 0xDD );
const BitInFormat       BitFormat::Elf( 0xDE );
const BitInFormat     BitFormat::Macho( 0xDF );
const BitInFormat       BitFormat::Udf( 0xE0 );
const BitInFormat       BitFormat::Xar( 0xE1 );
const BitInFormat       BitFormat::Mub( 0xE2 );
const BitInFormat       BitFormat::Hfs( 0xE3 );
const BitInFormat       BitFormat::Dmg( 0xE4 );
const BitInFormat  BitFormat::Compound( 0xE5 );
const BitOutFormat      BitFormat::Wim( 0xE6, L".wim", MULTIPLE_FILES );
const BitInFormat       BitFormat::Iso( 0xE7 );
const BitInFormat       BitFormat::Chm( 0xE9 );
const BitInFormat     BitFormat::Split( 0xEA );
const BitInFormat       BitFormat::Rpm( 0xEB );
const BitInFormat       BitFormat::Deb( 0xEC );
const BitInFormat      BitFormat::Cpio( 0xED );
const BitOutFormat      BitFormat::Tar( 0xEE, L".tar", MULTIPLE_FILES | INMEM_COMPRESSION );
const BitOutFormat     BitFormat::GZip( 0xEF, L".gz", COMPRESSION_LEVEL | INMEM_COMPRESSION );

BitInFormat::BitInFormat( int value ) : mValue( value ) {}

int BitInFormat::value() const {
    return mValue;
}

bool BitInFormat::operator==( const BitInFormat& other ) const {
    return mValue == other.value();
}

bool BitInFormat::operator!=( const BitInFormat& other ) const {
    return !( *this == other );
}

const GUID BitInFormat::guid() const {
    return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 } };
}


BitOutFormat::BitOutFormat( int value, const wstring& ext,
                            bitset<FEATURES_COUNT> features ) : BitInFormat( value ), mExtension( ext ), mFeatures( features ) {}

const wstring& BitOutFormat::extension() const {
    return mExtension;
}

const bitset<FEATURES_COUNT> BitOutFormat::features() const {
    return mFeatures;
}

bool BitOutFormat::hasFeature( FormatFeatures feature ) const {
    return ( mFeatures & bitset<FEATURES_COUNT>( feature ) ) != 0;
}
