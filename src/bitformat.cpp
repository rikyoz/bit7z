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

#include "../include/bitformat.hpp"

using namespace std;

namespace bit7z {
    namespace BitFormat {
#ifdef BIT7Z_AUTO_FORMAT
        const BitInFormat        Auto( 0x00 );
#endif
        const BitInOutFormat      Zip( 0x01, TSTRING(".zip"), BitCompressionMethod::Deflate,
                                       MULTIPLE_FILES | COMPRESSION_LEVEL | ENCRYPTION | MULTIPLE_METHODS );
        const BitInOutFormat    BZip2( 0x02, TSTRING(".bz2"), BitCompressionMethod::BZip2, COMPRESSION_LEVEL );
        const BitInFormat         Rar( 0x03 );
        const BitInFormat         Arj( 0x04 ); //-V112
        const BitInFormat           Z( 0x05 );
        const BitInFormat         Lzh( 0x06 );
        const BitInOutFormat SevenZip( 0x07, TSTRING(".7z"), BitCompressionMethod::Lzma2,
                                       MULTIPLE_FILES | SOLID_ARCHIVE | COMPRESSION_LEVEL |
                                       ENCRYPTION | HEADER_ENCRYPTION | MULTIPLE_METHODS );
        const BitInFormat         Cab( 0x08 );
        const BitInFormat        Nsis( 0x09 );
        const BitInFormat        Lzma( 0x0A );
        const BitInFormat      Lzma86( 0x0B );
        const BitInOutFormat       Xz( 0x0C, TSTRING(".xz"), BitCompressionMethod::Lzma2,
                                       COMPRESSION_LEVEL | ENCRYPTION | HEADER_ENCRYPTION);
        const BitInFormat        Ppmd( 0x0D );
        const BitInFormat        COFF( 0xC6 );
        const BitInFormat         Ext( 0xC7 );
        const BitInFormat        VMDK( 0xC8 );
        const BitInFormat         VDI( 0xC9 );
        const BitInFormat        QCow( 0xCA );
        const BitInFormat         GPT( 0xCB );
        const BitInFormat        Rar5( 0xCC );
        const BitInFormat        IHex( 0xCD );
        const BitInFormat         Hxs( 0xCE );
        const BitInFormat          TE( 0xCF );
        const BitInFormat       UEFIc( 0xD0 );
        const BitInFormat       UEFIs( 0xD1 );
        const BitInFormat    SquashFS( 0xD2 );
        const BitInFormat      CramFS( 0xD3 );
        const BitInFormat         APM( 0xD4 );
        const BitInFormat        Mslz( 0xD5 );
        const BitInFormat         Flv( 0xD6 );
        const BitInFormat         Swf( 0xD7 );
        const BitInFormat        Swfc( 0xD8 );
        const BitInFormat        Ntfs( 0xD9 );
        const BitInFormat         Fat( 0xDA );
        const BitInFormat         Mbr( 0xDB );
        const BitInFormat         Vhd( 0xDC );
        const BitInFormat          Pe( 0xDD );
        const BitInFormat         Elf( 0xDE );
        const BitInFormat       Macho( 0xDF );
        const BitInFormat         Udf( 0xE0 );
        const BitInFormat         Xar( 0xE1 );
        const BitInFormat         Mub( 0xE2 );
        const BitInFormat         Hfs( 0xE3 );
        const BitInFormat         Dmg( 0xE4 );
        const BitInFormat    Compound( 0xE5 );
        const BitInOutFormat      Wim( 0xE6, TSTRING(".wim"), BitCompressionMethod::Copy, MULTIPLE_FILES );
        const BitInFormat         Iso( 0xE7 );
        const BitInFormat         Chm( 0xE9 );
        const BitInFormat       Split( 0xEA );
        const BitInFormat         Rpm( 0xEB );
        const BitInFormat         Deb( 0xEC );
        const BitInFormat        Cpio( 0xED );
        const BitInOutFormat      Tar( 0xEE, TSTRING(".tar"), BitCompressionMethod::Copy, MULTIPLE_FILES );
        const BitInOutFormat     GZip( 0xEF, TSTRING(".gz"), BitCompressionMethod::Deflate, COMPRESSION_LEVEL );
    }

    BitInFormat::BitInFormat( unsigned char value ) noexcept : mValue( value ) {}

    int BitInFormat::value() const {
        return mValue;
    }

    bool BitInFormat::operator==( const BitInFormat& other ) const {
        return mValue == other.value();
    }

    bool BitInFormat::operator!=( const BitInFormat& other ) const {
        return !( *this == other );
    }

    GUID BitInFormat::guid() const {
        return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, mValue, 0x00, 0x00 } }; // NOLINT
    }

    BitInOutFormat::BitInOutFormat( unsigned char value,
                                    const tchar* ext,
                                    BitCompressionMethod defaultMethod,
                                    const FeaturesSet& features ) noexcept
        : BitInFormat( value ), mExtension( ext ), mDefaultMethod( defaultMethod ), mFeatures( features ) {}

    const tstring& BitInOutFormat::extension() const {
        return mExtension;
    }

    FeaturesSet BitInOutFormat::features() const {
        return mFeatures;
    }

    bool BitInOutFormat::hasFeature( FormatFeatures feature ) const {
        return ( mFeatures & FeaturesSet( feature ) ) != 0;
    }

    BitCompressionMethod BitInOutFormat::defaultMethod() const {
        return mDefaultMethod;
    }

}

