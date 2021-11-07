// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "bitformat.hpp"

using namespace std;

namespace bit7z {

#ifndef __cpp_inline_variables
    namespace BitFormat {
#ifdef BIT7Z_AUTO_FORMAT
        const BitInFormat Auto( 0x00 );
#endif
        const BitInOutFormat Zip( 0x01, BIT7Z_STRING( ".zip" ), BitCompressionMethod::Deflate,
                                  FormatFeatures::MultipleFiles | FormatFeatures::CompressionLevel |
                                  FormatFeatures::Encryption | FormatFeatures::MultipleMethods );
        const BitInOutFormat BZip2( 0x02, BIT7Z_STRING( ".bz2" ), BitCompressionMethod::BZip2,
                                    FormatFeatures::CompressionLevel );
        const BitInFormat Rar( 0x03 );
        const BitInFormat Arj( 0x04 ); //-V112
        const BitInFormat Z( 0x05 );
        const BitInFormat Lzh( 0x06 );
        const BitInOutFormat SevenZip( 0x07, BIT7Z_STRING( ".7z" ), BitCompressionMethod::Lzma2,
                                       FormatFeatures::MultipleFiles | FormatFeatures::SolidArchive |
                                       FormatFeatures::CompressionLevel | FormatFeatures::Encryption |
                                       FormatFeatures::HeaderEncryption | FormatFeatures::MultipleMethods );
        const BitInFormat Cab( 0x08 );
        const BitInFormat Nsis( 0x09 );
        const BitInFormat Lzma( 0x0A );
        const BitInFormat Lzma86( 0x0B );
        const BitInOutFormat Xz( 0x0C, BIT7Z_STRING( ".xz" ), BitCompressionMethod::Lzma2,
                                 FormatFeatures::CompressionLevel );
        const BitInFormat Ppmd( 0x0D );
        const BitInFormat COFF( 0xC6 );
        const BitInFormat Ext( 0xC7 );
        const BitInFormat VMDK( 0xC8 );
        const BitInFormat VDI( 0xC9 );
        const BitInFormat QCow( 0xCA );
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
        const BitInOutFormat Wim( 0xE6, BIT7Z_STRING( ".wim" ), BitCompressionMethod::Copy, FormatFeatures::MultipleFiles );
        const BitInFormat Iso( 0xE7 );
        const BitInFormat Chm( 0xE9 );
        const BitInFormat Split( 0xEA );
        const BitInFormat Rpm( 0xEB );
        const BitInFormat Deb( 0xEC );
        const BitInFormat Cpio( 0xED );
        const BitInOutFormat Tar( 0xEE, BIT7Z_STRING( ".tar" ), BitCompressionMethod::Copy, FormatFeatures::MultipleFiles );
        const BitInOutFormat GZip( 0xEF, BIT7Z_STRING( ".gz" ), BitCompressionMethod::Deflate,
                                   FormatFeatures::CompressionLevel );
    }
#endif

    unsigned char BitInFormat::value() const noexcept {
        return mValue;
    }

    bool BitInFormat::operator==( const BitInFormat& other ) const noexcept {
        return mValue == other.value();
    }

    bool BitInFormat::operator!=( const BitInFormat& other ) const noexcept {
        return !( *this == other );
    }

    const tchar* BitInOutFormat::extension() const noexcept {
        return mExtension;
    }

    FormatFeatures BitInOutFormat::features() const noexcept {
        return mFeatures;
    }

    bool BitInOutFormat::hasFeature( FormatFeatures feature ) const noexcept {
        return ( mFeatures & feature ) != 0;
    }

    BitCompressionMethod BitInOutFormat::defaultMethod() const noexcept {
        return mDefaultMethod;
    }

}

