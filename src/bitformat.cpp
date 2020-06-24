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

#ifdef BIT7Z_AUTO_FORMAT

#include "../include/bitexception.hpp"
#include "../include/fsutil.hpp"

#include <cwctype>

#include <7zip/IStream.h>

#if defined(_WIN32)
#define bswap64 _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
//Note: the versions of gcc and clang that can compile bit7z should also have this builtin, hence there is no need
//      for checking compiler version or using _has_builtin macro!
#define bswap64 __builtin_bswap64
#else
static inline uint64_t bswap64 (uint64_t x) {
    return  ((x << 56) & 0xff00000000000000ULL) |
            ((x << 40) & 0x00ff000000000000ULL) |
            ((x << 24) & 0x0000ff0000000000ULL) |
            ((x << 8)  & 0x000000ff00000000ULL) |
            ((x >> 8)  & 0x00000000ff000000ULL) |
            ((x >> 24) & 0x0000000000ff0000ULL) |
            ((x >> 40) & 0x000000000000ff00ULL) |
            ((x >> 56) & 0x00000000000000ffULL);
}
#endif

#endif


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

#ifdef BIT7Z_AUTO_FORMAT
        /* NOTE: Until v3, a std::unordered_map was used for mapping the extensions and the corresponding
         *       format, however the ifs are faster and have less memory footprint.
         * TODO: Cleanup and improve */
        bool findFormatByExtension( const tstring& ext, const BitInFormat** format ) {
            if ( ext == TSTRING( "7z" ) ) {
                *format = &SevenZip;
                return true;
            }
            if ( ext == TSTRING( "bzip2" ) || ext == TSTRING( "bz2" ) ||
                 ext == TSTRING( "tbz2" ) || ext == TSTRING( "tbz" ) ) {
                *format = &BZip2;
                return true;
            }
            if ( ext == TSTRING( "gz" ) || ext == TSTRING( "gzip" ) || ext == TSTRING( "tgz" ) ) {
                *format = &GZip;
                return true;
            }
            if ( ext == TSTRING( "tar" ) ) {
                *format = &Tar;
                return true;
            }
            if ( ext == TSTRING( "wim" ) || ext == TSTRING( "swm" ) ) {
                *format = &Wim;
                return true;
            }
            if ( ext == TSTRING( "xz" ) || ext == TSTRING( "txz" ) ) {
                *format = &Xz;
                return true;
            }
            if ( ext == TSTRING( "zip" ) || ext == TSTRING( "zipx" ) || ext == TSTRING( "jar" ) ||
                 ext == TSTRING( "xpi" ) || ext == TSTRING( "odt" ) || ext == TSTRING( "ods" ) ||
                 ext == TSTRING( "odp" ) || ext == TSTRING( "docx" ) || ext == TSTRING( "xlsx" ) ||
                 ext == TSTRING( "pptx" ) || ext == TSTRING( "epub" ) ) {
                *format = &Zip;
                return true;
            }
            if ( ext == TSTRING( "001" ) ) {
                *format = &Split;
                return true;
            }
            if ( ext == TSTRING( "ar" ) ) {
                *format = &Deb;
                return true;
            }
            if ( ext == TSTRING( "apm" ) ) {
                *format = &APM;
                return true;
            }
            if ( ext == TSTRING( "arj" ) ) {
                *format = &Arj;
                return true;
            }
            if ( ext == TSTRING( "cab" ) ) {
                *format = &Cab;
                return true;
            }
            if ( ext == TSTRING( "chm" ) || ext == TSTRING( "chi" ) ) {
                *format = &Chm;
                return true;
            }
            if ( ext == TSTRING( "msi" ) || ext == TSTRING( "doc" ) || ext == TSTRING( "xls" ) ||
                 ext == TSTRING( "ppt" ) || ext == TSTRING( "msg" ) ) {
                *format = &Compound;
                return true;
            }
            if ( ext == TSTRING( "obj" ) ) {
                *format = &COFF;
                return true;
            }
            if ( ext == TSTRING( "cpio" ) ) {
                *format = &Cpio;
                return true;
            }
            if ( ext == TSTRING( "cramfs" ) ) {
                *format = &CramFS;
                return true;
            }
            if ( ext == TSTRING( "deb" ) ) {
                *format = &Deb;
                return true;
            }
            if ( ext == TSTRING( "dmg" ) ) {
                *format = &Dmg;
                return true;
            }
            if ( ext == TSTRING( "dll" ) || ext == TSTRING( "exe" ) ) {
                //note: we do not distinguish 7z SFX exe at the moment!
                *format = &Pe;
                return true;
            }
            if ( ext == TSTRING( "dylib" ) ) {
                *format = &Macho;
                return true;
            }
            if ( ext == TSTRING( "ext" ) || ext == TSTRING( "ext2" ) || ext == TSTRING( "ext3" ) ||
                 ext == TSTRING( "ext4" ) ) {
                *format = &Ext;
                return true;
            }
            if ( ext == TSTRING( "fat" ) ) {
                *format = &Fat;
                return true;
            }
            if ( ext == TSTRING( "flv" ) ) {
                *format = &Flv;
                return true;
            }
            if ( ext == TSTRING( "gpt" ) ) {
                *format = &GPT;
                return true;
            }
            if ( ext == TSTRING( "hfs" ) || ext == TSTRING( "hfsx" ) ) {
                *format = &Hfs;
                return true;
            }
            if ( ext == TSTRING( "hxs" ) ) {
                *format = &Hxs;
                return true;
            }
            if ( ext == TSTRING( "ihex" ) ) {
                *format = &IHex;
                return true;
            }
            if ( ext == TSTRING( "lzh" ) || ext == TSTRING( "lha" ) ) {
                *format = &Lzh;
                return true;
            }
            if ( ext == TSTRING( "lzma" ) ) {
                *format = &Lzma;
                return true;
            }
            if ( ext == TSTRING( "lzma86" ) ) {
                *format = &Lzma86;
                return true;
            }
            if ( ext == TSTRING( "mbr" ) ) {
                *format = &Mbr;
                return true;
            }
            if ( ext == TSTRING( "mslz" ) ) {
                *format = &Mslz;
                return true;
            }
            if ( ext == TSTRING( "mub" ) ) {
                *format = &Mub;
                return true;
            }
            if ( ext == TSTRING( "nsis" ) ) {
                *format = &Nsis;
                return true;
            }
            if ( ext == TSTRING( "ntfs" ) ) {
                *format = &Ntfs;
                return true;
            }
            if ( ext == TSTRING( "pmd" ) ) {
                *format = &Ppmd;
                return true;
            }
            if ( ext == TSTRING( "qcow" ) || ext == TSTRING( "qcow2" ) || ext == TSTRING( "qcow2c" ) ) {
                *format = &QCow;
                return true;
            }
            if ( ext == TSTRING( "rpm" ) ) {
                *format = &Rpm;
                return true;
            }
            if ( ext == TSTRING( "squashfs" ) ) {
                *format = &SquashFS;
                return true;
            }
            if ( ext == TSTRING( "te" ) ) {
                *format = &TE;
                return true;
            }
            if ( ext == TSTRING( "udf" ) ) {
                *format = &Udf;
                return true;
            }
            if ( ext == TSTRING( "scap" ) ) {
                *format = &UEFIc;
                return true;
            }
            if ( ext == TSTRING( "uefif" ) ) {
                *format = &UEFIs;
                return true;
            }
            if ( ext == TSTRING( "vmdk" ) ) {
                *format = &VMDK;
                return true;
            }
            if ( ext == TSTRING( "vdi" ) ) {
                *format = &VDI;
                return true;
            }
            if ( ext == TSTRING( "vhd" ) ) {
                *format = &Vhd;
                return true;
            }
            if ( ext == TSTRING( "xar" ) || ext == TSTRING( "pkg" ) ) {
                *format = &Xar;
                return true;
            }
            if ( ext == TSTRING( "z" ) || ext == TSTRING( "taz" ) ) {
                *format = &Z;
                return true;
            }
            return false;
        }

        /* NOTE 1: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0
         * NOTE 2: Until v3, a std::unordered_map was used for mapping the signatures and the corresponding
         *         format, however the switch case is faster and has less memory footprint. */
        bool findFormatBySignature( uint64_t signature, const BitInFormat** format ) {
            switch ( signature ) {
                case 0x526172211A070000: // R  a  r  !  1A 07 00
                    *format = &Rar;
                    return true;
                case 0x526172211A070100: // R  a  r  !  1A 07 01 00
                    *format = &Rar5;
                    return true;
                case 0x377ABCAF271C0000: // 7  z  BC AF 27 1C
                    *format = &SevenZip;
                    return true;
                case 0x425A680000000000: // B  Z  h
                    *format = &BZip2;
                    return true;
                case 0x1F8B080000000000: // 1F 8B 08
                    *format = &GZip;
                    return true;
                case 0x4D5357494D000000: // M  S  W  I  M  00 00 00
                    *format = &Wim;
                    return true;
                case 0xFD377A585A000000: // FD 7  z  X  Z  00
                    *format = &Xz;
                    return true;
                case 0x504B000000000000: // P  K
                    *format = &Zip;
                    return true;
                case 0x4552000000000000: // E  R
                    *format = &APM;
                    return true;
                case 0x60EA000000000000: // `  EA
                    *format = &Arj;
                    return true;
                case 0x4D53434600000000: // M  S  C  F  00 00 00 00
                    *format = &Cab;
                    return true;
                case 0x4954534603000000: // I  T  S  F  03
                    *format = &Chm;
                    return true;
                case 0xD0CF11E0A1B11AE1: // D0 CF 11 E0 A1 B1 1A E1
                    *format = &Compound;
                    return true;
                case 0xC771000000000000: // C7 q
                case 0x71C7000000000000: // q  C7
                case 0x3037303730000000: // 0  7  0  7  0
                    *format = &Cpio;
                    return true;
                case 0x213C617263683E00: // !  <  a  r  c  h  >  0A
                    *format = &Deb;
                    return true;
                    /* DMG signature detection is not this simple
                    case 0x7801730D62626000:
                        *format = &Dmg;
                        return true;
                    */
                case 0x7F454C4600000000: // 7F E  L  F
                    *format = &Elf;
                    return true;
                case 0x4D5A000000000000: // M  Z
                    *format = &Pe;
                    return true;
                case 0x464C560100000000: // F  L  V  01
                    *format = &Flv;
                    return true;
                case 0x5D00000000000000: //
                    *format = &Lzma;
                    return true;
                case 0x015D000000000000: //
                    *format = &Lzma86;
                    return true;
                case 0xCEFAEDFE00000000: // CE FA ED FE
                case 0xCFFAEDFE00000000: // CF FA ED FE
                case 0xFEEDFACE00000000: // FE ED FA CE
                case 0xFEEDFACF00000000: // FE ED FA CF
                    *format = &Macho;
                    return true;
                case 0xCAFEBABE00000000: // CA FE BA BE 00 00 00
                case 0xB9FAF10E00000000: // B9 FA F1 0E
                    *format = &Mub;
                    return true;
                case 0x535A444488F02733: // S  Z  D  D  88 F0 '  3
                    *format = &Mslz;
                    return true;
                case 0x8FAFAC8400000000: // 8F AF AC 84
                    *format = &Ppmd;
                    return true;
                case 0x514649FB00000000: // Q  F  I  FB 00 00 00
                    *format = &QCow;
                    return true;
                case 0xEDABEEDB00000000: // ED AB EE DB
                    *format = &Rpm;
                    return true;
                case 0x7371736800000000: // s  q  s  h
                case 0x6873717300000000: // h  s  q  s
                case 0x7368737100000000: // s  h  s  q
                case 0x7173687300000000: // q  s  h  s
                    *format = &SquashFS;
                    return true;
                case 0x4657530000000000: // F  W  S
                    *format = &Swf;
                    return true;
                case 0x4357530000000000: // C  W  S
                case 0x5A57530000000000: // Z  W  S
                    *format = &Swfc;
                    return true;
                case 0x565A000000000000: // V  Z
                    *format = &TE;
                    return true;
                case 0x4B444D0000000000: // K  D  M  V
                    *format = &VMDK;
                    return true;
                case 0x3C3C3C2000000000: // Alternatively 0x7F10DABE at offset 0x40)
                    *format = &VDI;
                    return true;
                case 0x636F6E6563746978: // c  o  n  e  c  t  i  x
                    *format = &Vhd;
                    return true;
                case 0x78617221001C0000: // x  a  r  !  00 1C
                    *format = &Xar;
                    return true;
                case 0x1F9D000000000000: // 1F 9D
                case 0x1FA0000000000000: // 1F A0
                    *format = &Z;
                    return true;
                default:
                    return false;
            }
        }

        struct OffsetSignature {
            uint64_t signature;
            std::streamoff offset;
            uint32_t size;
            const BitInFormat& format;
        };

        const OffsetSignature common_signatures_with_offset[] = {
            { 0x2D6C680000000000, 0x02,  3, Lzh },    // -  l  h
            { 0x4E54465320202020, 0x03,  8, Ntfs },   // N  T  F  S  20 20 20 20
            { 0x4E756C6C736F6674, 0x08,  8, Nsis },   // N  u  l  l  s  o  f  t
            { 0x436F6D7072657373, 0x10,  8, CramFS }, // C  o  m  p  r  e  s  s
            { 0x7F10DABE00000000, 0x40,  4, VDI },    // 7F 10 DA BE
            { 0x7573746172000000, 0x101, 5, Tar },    // u  s  t  a  r
            // Note: since GPT files contain also the FAT signature, GPT must be checked before!
            { 0x4546492050415254, 0x200, 8, GPT },    // E  F  I  20 P  A  R  T
            { 0x55AA000000000000, 0x1FE, 2, Fat },    // U  AA
            { 0x4244000000000000, 0x400, 2, Hfs },    // B  D
            { 0x482B000400000000, 0x400, 4, Hfs },    // H  +  00 04
            { 0x4858000500000000, 0x400, 4, Hfs },    // H  X  00 05
            { 0x53EF000000000000, 0x438, 2, Ext }     // S  EF
        };

        uint64_t readSignature( IInStream* stream, uint32_t size ) {
            uint64_t signature = 0;
            stream->Read( &signature, size, nullptr );
            return bswap64( signature );
        }

        const BitInFormat& detectFormatFromSig( IInStream* stream ) {
            constexpr auto SIGNATURE_SIZE = 8u;
            constexpr auto BASE_SIGNATURE_MASK = 0xFFFFFFFFFFFFFFFFull;
            constexpr auto BYTE_SHIFT = 8ull;

            uint64_t file_signature = readSignature( stream, SIGNATURE_SIZE );
            uint64_t signature_mask = BASE_SIGNATURE_MASK;
            for ( auto i = 0u; i < SIGNATURE_SIZE - 1; ++i ) {
                const BitInFormat* format = nullptr;
                if ( findFormatBySignature( file_signature, &format ) ) {
                    stream->Seek( 0, 0, nullptr );
                    return *format;
                }
                signature_mask <<= BYTE_SHIFT;    // left shifting the mask of 1 byte, so that
                file_signature &= signature_mask; // the least significant i bytes are masked (set to 0)
            }

            for ( auto& sig : common_signatures_with_offset ) {
                stream->Seek( sig.offset, 0, nullptr );
                file_signature = readSignature( stream, sig.size );
                if ( file_signature == sig.signature ) {
                    stream->Seek( 0, 0, nullptr );
                    return sig.format;
                }
            }

            // Detecting ISO/UDF
            constexpr auto ISO_SIGNATURE = 0x4344303031000000; //CD001
            constexpr auto ISO_SIGNATURE_SIZE = 5ull;
            constexpr auto ISO_SIGNATURE_OFFSET = 0x8001;

            // Checking for ISO signature
            stream->Seek( ISO_SIGNATURE_OFFSET, 0, nullptr );
            file_signature = readSignature( stream, ISO_SIGNATURE_SIZE );
            if ( file_signature == ISO_SIGNATURE ) {
                constexpr auto MAX_VOLUME_DESCRIPTORS = 16;
                constexpr auto ISO_VOLUME_DESCRIPTOR_SIZE = 0x800; //2048

                constexpr auto UDF_SIGNATURE = 0x4E53523000000000; //NSR0
                constexpr auto UDF_SIGNATURE_SIZE = 4u;

                // The file is ISO, checking if it is also UDF!
                for ( auto descriptor_index = 1ull; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
                    stream->Seek( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE, 0, nullptr );
                    file_signature = readSignature( stream, UDF_SIGNATURE_SIZE );
                    if ( file_signature == UDF_SIGNATURE ) {
                        stream->Seek( 0, 0, nullptr );
                        return Udf;
                    }
                }
                stream->Seek( 0, 0, nullptr );
                return Iso; //No UDF volume signature found, i.e. simple ISO!
            }

            stream->Seek( 0, 0, nullptr );
            throw BitException( "Cannot detect the format of the file" );
        }

        const BitInFormat& detectFormatFromExt( const tstring& in_file ) {
            tstring ext = filesystem::fsutil::extension( in_file );
            if ( ext.empty() ) {
                throw BitException( "Cannot detect the archive format from the extension" );
            }
            std::transform( ext.cbegin(), ext.cend(), ext.begin(), std::towlower );

            // Detecting archives with common file extensions
            const BitInFormat* format = nullptr;
            if ( findFormatByExtension( ext, &format ) ) { //extension found in map
                return *format;
            }

            // Detecting multi-volume archives extension
            if ( ( ext[ 0 ] == L'r' || ext[ 0 ] == L'z' ) &&
                 ( ext.size() == 3 && iswdigit( ext[ 1 ] ) != 0 && iswdigit( ext[ 2 ] ) != 0 ) ) {
                // Extension follows the format zXX or rXX, where X is a number in range [0-9]
                return ext[ 0 ] == L'r' ? Rar : Zip;
            }

            // Note: iso, img and ima extensions can be associated with different formats -> detect by signature

            // The extension did not match any known format extension, delegating the decision to the client
            return Auto;
        }

#endif

    }

}

using namespace bit7z;

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
