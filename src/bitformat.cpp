#include "../include/bitformat.hpp"

using namespace std;

namespace bit7z {
    namespace BitFormat {
        const BitInOutFormat Zip( 0x01, L".zip", MULTIPLE_FILES | COMPRESSION_LEVEL | ENCRYPTION );
        const BitInOutFormat BZip2( 0x02, L".bz2", COMPRESSION_LEVEL | INMEM_COMPRESSION );
        const BitInFormat Rar( 0x03 );
        const BitInFormat Arj( 0x04 );
        const BitInFormat Z( 0x05 );
        const BitInFormat Lzh( 0x06 );
        const BitInOutFormat SevenZip( 0x07, L".7z", MULTIPLE_FILES | SOLID_ARCHIVE | COMPRESSION_LEVEL |
                                       ENCRYPTION | HEADER_ENCRYPTION );
        const BitInFormat Cab( 0x08 );
        const BitInFormat Nsis( 0x09 );
        const BitInFormat Lzma( 0x0A );
        const BitInFormat Lzma86( 0x0B );
        const BitInOutFormat Xz( 0x0C, L".xz",
                                 COMPRESSION_LEVEL | ENCRYPTION | HEADER_ENCRYPTION | INMEM_COMPRESSION );
        const BitInFormat Ppmd( 0x0D );
        const BitInFormat Ext( 0xC7 );
        const BitInFormat VMDK( 0xC8 );
        const BitInFormat VDI( 0xC9 );
        const BitInFormat Qcow( 0xCA );
        const BitInFormat GPT( 0xCB );
        const BitInFormat Rar5( 0xCC );
        const BitInFormat IHex( 0xCD );
        const BitInFormat Hxs( 0xCE );
        const BitInFormat TE( 0xCF );
        const BitInFormat UEFIc( 0xD0 );
        const BitInFormat UEFIs( 0xD1 );
        const BitInFormat SquashFS( 0xD2 );
        const BitInFormat CramFS( 0xD3 );
        const BitInFormat APM( 0xD4 );
        const BitInFormat Mslz( 0xD5 );
        const BitInFormat Flv( 0xD6 );
        const BitInFormat Swf( 0xD7 );
        const BitInFormat Swfc( 0xD8 );
        const BitInFormat Ntfs( 0xD9 );
        const BitInFormat Fat( 0xDA );
        const BitInFormat Mbr( 0xDB );
        const BitInFormat Vhd( 0xDC );
        const BitInFormat Pe( 0xDD );
        const BitInFormat Elf( 0xDE );
        const BitInFormat Macho( 0xDF );
        const BitInFormat Udf( 0xE0 );
        const BitInFormat Xar( 0xE1 );
        const BitInFormat Mub( 0xE2 );
        const BitInFormat Hfs( 0xE3 );
        const BitInFormat Dmg( 0xE4 );
        const BitInFormat Compound( 0xE5 );
        const BitInOutFormat Wim( 0xE6, L".wim", MULTIPLE_FILES );
        const BitInFormat Iso( 0xE7 );
        const BitInFormat Chm( 0xE9 );
        const BitInFormat Split( 0xEA );
        const BitInFormat Rpm( 0xEB );
        const BitInFormat Deb( 0xEC );
        const BitInFormat Cpio( 0xED );
        const BitInOutFormat Tar( 0xEE, L".tar", MULTIPLE_FILES | INMEM_COMPRESSION );
        const BitInOutFormat GZip( 0xEF, L".gz", COMPRESSION_LEVEL | INMEM_COMPRESSION );
    }
}

using namespace bit7z;

BitInFormat::BitInFormat( unsigned char value ) : mValue( value ) {}

int BitInFormat::value() const {
    return mValue;
}

bool BitInFormat::operator==( const BitInFormat &other ) const {
    return mValue == other.value();
}

bool BitInFormat::operator!=( const BitInFormat &other ) const {
    return !( *this == other );
}

const GUID BitInFormat::guid() const {
#if _MSC_VER <= 1600
    GUID ret;
    ret.Data1 = 0x23170F69;
    ret.Data2 = 0x40C1;
    ret.Data3 = 0x278A;
    
    const unsigned char data4 [] = { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 };
    std::copy(data4, data4+8, ret.Data4);

    return ret;
#else
    return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 } };
#endif
}

BitInOutFormat::BitInOutFormat( unsigned char value, const wstring &ext, bitset< FEATURES_COUNT > features ) :
    BitInFormat( value ), mExtension( ext ), mFeatures( features ) {}

const wstring &BitInOutFormat::extension() const {
    return mExtension;
}

const bitset< FEATURES_COUNT > BitInOutFormat::features() const {
    return mFeatures;
}

bool BitInOutFormat::hasFeature( FormatFeatures feature ) const {
    return ( mFeatures & bitset< FEATURES_COUNT >( feature ) ) != 0;
}
