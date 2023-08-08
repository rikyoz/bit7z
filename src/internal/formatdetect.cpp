// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/formatdetect.hpp"

#ifdef BIT7Z_AUTO_FORMAT

#include <algorithm>
#include "internal/formatdetect.hpp"

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
#include <cwctype> // for std::iswdigit
#else
#include <cctype> // for std::isdigit
#endif

#include "biterror.hpp"
#include "bitexception.hpp"
#include "internal/fsutil.hpp"
#ifndef _WIN32
#include "internal/guiddef.hpp"
#endif

#include <7zip/IStream.h>

/**
 * @brief Constexpr recursive implementation of the djb2 hashing function.
 *
 * @param input The C string to be hashed.
 *
 * @return The hash integer.
 */
auto constexpr str_hash( bit7z::tchar const* input ) -> uint64_t { // NOLINT(misc-no-recursion)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, *-magic-numbers)
    return *input != 0 ? static_cast< uint64_t >( *input ) + 33 * str_hash( input + 1 ) : 5381; //-V2563
}

namespace bit7z {
/* NOTE: Until v3, a std::unordered_map was used for mapping the extensions and the corresponding
 *       format, but the ifs are faster and have less memory footprint. */
auto findFormatByExtension( const tstring& extension ) -> const BitInFormat* {
    switch ( str_hash( extension.c_str() ) ) {
        case str_hash( BIT7Z_STRING( "7z" ) ):
            return &BitFormat::SevenZip;
        case str_hash( BIT7Z_STRING( "bzip2" ) ):
        case str_hash( BIT7Z_STRING( "bz2" ) ):
        case str_hash( BIT7Z_STRING( "tbz2" ) ):
        case str_hash( BIT7Z_STRING( "tbz" ) ):
            return &BitFormat::BZip2;
        case str_hash( BIT7Z_STRING( "gz" ) ):
        case str_hash( BIT7Z_STRING( "gzip" ) ):
        case str_hash( BIT7Z_STRING( "tgz" ) ):
            return &BitFormat::GZip;
        case str_hash( BIT7Z_STRING( "tar" ) ):
        case str_hash( BIT7Z_STRING( "ova" ) ):
            return &BitFormat::Tar;
        case str_hash( BIT7Z_STRING( "wim" ) ):
        case str_hash( BIT7Z_STRING( "swm" ) ):
            return &BitFormat::Wim;
        case str_hash( BIT7Z_STRING( "xz" ) ):
        case str_hash( BIT7Z_STRING( "txz" ) ):
            return &BitFormat::Xz;
        case str_hash( BIT7Z_STRING( "zip" ) ):
        case str_hash( BIT7Z_STRING( "zipx" ) ):
        case str_hash( BIT7Z_STRING( "jar" ) ):
        case str_hash( BIT7Z_STRING( "xpi" ) ):
        case str_hash( BIT7Z_STRING( "odt" ) ):
        case str_hash( BIT7Z_STRING( "ods" ) ):
        case str_hash( BIT7Z_STRING( "odp" ) ):
        case str_hash( BIT7Z_STRING( "docx" ) ):
        case str_hash( BIT7Z_STRING( "xlsx" ) ):
        case str_hash( BIT7Z_STRING( "pptx" ) ):
        case str_hash( BIT7Z_STRING( "epub" ) ):
            return &BitFormat::Zip;
        case str_hash( BIT7Z_STRING( "001" ) ):
            return &BitFormat::Split;
        case str_hash( BIT7Z_STRING( "ar" ) ):
        case str_hash( BIT7Z_STRING( "deb" ) ):
            return &BitFormat::Deb;
        case str_hash( BIT7Z_STRING( "apm" ) ):
            return &BitFormat::APM;
        case str_hash( BIT7Z_STRING( "arj" ) ):
            return &BitFormat::Arj;
        case str_hash( BIT7Z_STRING( "cab" ) ):
            return &BitFormat::Cab;
        case str_hash( BIT7Z_STRING( "chm" ) ):
        case str_hash( BIT7Z_STRING( "chi" ) ):
            return &BitFormat::Chm;
        case str_hash( BIT7Z_STRING( "msi" ) ):
        case str_hash( BIT7Z_STRING( "doc" ) ):
        case str_hash( BIT7Z_STRING( "xls" ) ):
        case str_hash( BIT7Z_STRING( "ppt" ) ):
        case str_hash( BIT7Z_STRING( "msg" ) ):
            return &BitFormat::Compound;
        case str_hash( BIT7Z_STRING( "obj" ) ):
            return &BitFormat::COFF;
        case str_hash( BIT7Z_STRING( "cpio" ) ):
            return &BitFormat::Cpio;
        case str_hash( BIT7Z_STRING( "cramfs" ) ):
            return &BitFormat::CramFS;
        case str_hash( BIT7Z_STRING( "dmg" ) ):
            return &BitFormat::Dmg;
        case str_hash( BIT7Z_STRING( "dll" ) ):
        case str_hash( BIT7Z_STRING( "exe" ) ):
            //note: at least for now, we do not distinguish 7z SFX executables!
            return &BitFormat::Pe;
        case str_hash( BIT7Z_STRING( "dylib" ) ):
            return &BitFormat::Macho;
        case str_hash( BIT7Z_STRING( "ext" ) ):
        case str_hash( BIT7Z_STRING( "ext2" ) ):
        case str_hash( BIT7Z_STRING( "ext3" ) ):
        case str_hash( BIT7Z_STRING( "ext4" ) ):
            return &BitFormat::Ext;
        case str_hash( BIT7Z_STRING( "fat" ) ):
            return &BitFormat::Fat;
        case str_hash( BIT7Z_STRING( "flv" ) ):
            return &BitFormat::Flv;
        case str_hash( BIT7Z_STRING( "gpt" ) ):
            return &BitFormat::GPT;
        case str_hash( BIT7Z_STRING( "hfs" ) ):
        case str_hash( BIT7Z_STRING( "hfsx" ) ):
            return &BitFormat::Hfs;
        case str_hash( BIT7Z_STRING( "hxs" ) ):
            return &BitFormat::Hxs;
        case str_hash( BIT7Z_STRING( "ihex" ) ):
            return &BitFormat::IHex;
        case str_hash( BIT7Z_STRING( "lzh" ) ):
        case str_hash( BIT7Z_STRING( "lha" ) ):
            return &BitFormat::Lzh;
        case str_hash( BIT7Z_STRING( "lzma" ) ):
            return &BitFormat::Lzma;
        case str_hash( BIT7Z_STRING( "lzma86" ) ):
            return &BitFormat::Lzma86;
        case str_hash( BIT7Z_STRING( "mbr" ) ):
            return &BitFormat::Mbr;
        case str_hash( BIT7Z_STRING( "mslz" ) ):
            return &BitFormat::Mslz;
        case str_hash( BIT7Z_STRING( "mub" ) ):
            return &BitFormat::Mub;
        case str_hash( BIT7Z_STRING( "nsis" ) ):
            return &BitFormat::Nsis;
        case str_hash( BIT7Z_STRING( "ntfs" ) ):
            return &BitFormat::Ntfs;
        case str_hash( BIT7Z_STRING( "pmd" ) ):
        case str_hash( BIT7Z_STRING( "ppmd" ) ):
            return &BitFormat::Ppmd;
        case str_hash( BIT7Z_STRING( "qcow" ) ):
        case str_hash( BIT7Z_STRING( "qcow2" ) ):
        case str_hash( BIT7Z_STRING( "qcow2c" ) ):
            return &BitFormat::QCow;
        case str_hash( BIT7Z_STRING( "rpm" ) ):
            return &BitFormat::Rpm;
        case str_hash( BIT7Z_STRING( "squashfs" ) ):
            return &BitFormat::SquashFS;
        case str_hash( BIT7Z_STRING( "swf" ) ):
            return &BitFormat::Swf;
        case str_hash( BIT7Z_STRING( "te" ) ):
            return &BitFormat::TE;
        case str_hash( BIT7Z_STRING( "udf" ) ):
            return &BitFormat::Udf;
        case str_hash( BIT7Z_STRING( "scap" ) ):
            return &BitFormat::UEFIc;
        case str_hash( BIT7Z_STRING( "uefif" ) ):
            return &BitFormat::UEFIs;
        case str_hash( BIT7Z_STRING( "vmdk" ) ):
            return &BitFormat::VMDK;
        case str_hash( BIT7Z_STRING( "vdi" ) ):
            return &BitFormat::VDI;
        case str_hash( BIT7Z_STRING( "vhd" ) ):
            return &BitFormat::Vhd;
        case str_hash( BIT7Z_STRING( "xar" ) ):
        case str_hash( BIT7Z_STRING( "pkg" ) ):
            return &BitFormat::Xar;
        case str_hash( BIT7Z_STRING( "z" ) ):
        case str_hash( BIT7Z_STRING( "taz" ) ):
            return &BitFormat::Z;
        default:
            return nullptr;
    }
}

/* NOTE 1: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0
 * NOTE 2: Until v3, a std::unordered_map was used for mapping the signatures and the corresponding
 *         format. However, the switch case is faster and has less memory footprint. */
auto findFormatBySignature( uint64_t signature ) noexcept -> const BitInFormat* {
    constexpr auto RarSignature = 0x526172211A070000ULL; // Rar! 0x1A 0x07 0x00
    constexpr auto Rar5Signature = 0x526172211A070100ULL; // Rar! 0x1A 0x07 0x01 0x00
    constexpr auto SevenZipSignature = 0x377ABCAF271C0000ULL; // 7z 0xBC 0xAF 0x27 0x1C
    constexpr auto BZip2Signature = 0x425A680000000000ULL; // BZh
    constexpr auto GZipSignature = 0x1F8B080000000000ULL; // 0x1F 0x8B 0x08
    constexpr auto WimSignature = 0x4D5357494D000000ULL; // MSWIM 0x00 0x00 0x00
    constexpr auto XzSignature = 0xFD377A585A000000ULL; // 0xFD 7zXZ 0x00
    constexpr auto ZipSignature = 0x504B000000000000ULL; // PK
    constexpr auto APMSignature = 0x4552000000000000ULL; // ER
    constexpr auto ArjSignature = 0x60EA000000000000ULL; // `EA
    constexpr auto CabSignature = 0x4D53434600000000ULL; // MSCF 0x00 0x00 0x00 0x00
    constexpr auto ChmSignature = 0x4954534603000000ULL; // ITSF 0x03
    constexpr auto CompoundSignature = 0xD0CF11E0A1B11AE1ULL; // 0xD0 0xCF 0x11 0xE0 0xA1 0xB1 0x1A 0xE1
    constexpr auto CpioSignature1 = 0xC771000000000000ULL; // 0xC7 q
    constexpr auto CpioSignature2 = 0x71C7000000000000ULL; // q 0xC7
    constexpr auto CpioSignature3 = 0x3037303730000000ULL; // 07070
    constexpr auto DebSignature = 0x213C617263683E00ULL; // !<arch>0A
    constexpr auto ElfSignature = 0x7F454C4600000000ULL; // 0x7F ELF
    constexpr auto PeSignature = 0x4D5A000000000000ULL; // MZ
    constexpr auto FlvSignature = 0x464C560100000000ULL; // FLV 0x01
    constexpr auto LzmaSignature = 0x5D00000000000000ULL; //
    constexpr auto Lzma86Signature = 0x015D000000000000ULL; //
    constexpr auto MachoSignature1 = 0xCEFAEDFE00000000ULL; // 0xCE 0xFA 0xED 0xFE
    constexpr auto MachoSignature2 = 0xCFFAEDFE00000000ULL; // 0xCF 0xFA 0xED 0xFE
    constexpr auto MachoSignature3 = 0xFEEDFACE00000000ULL; // 0xFE 0xED 0xFA 0xCE
    constexpr auto MachoSignature4 = 0xFEEDFACF00000000ULL; // 0xFE 0xED 0xFA 0xCF
    constexpr auto MubSignature1 = 0xCAFEBABE00000000ULL; // 0xCA 0xFE 0xBA 0xBE 0x00 0x00 0x00
    constexpr auto MubSignature2 = 0xB9FAF10E00000000ULL; // 0xB9 0xFA 0xF1 0x0E
    constexpr auto MslzSignature = 0x535A444488F02733ULL; // SZDD 0x88 0xF0 '3
    constexpr auto PpmdSignature = 0x8FAFAC8400000000ULL; // 0x8F 0xAF 0xAC 0x84
    constexpr auto QCowSignature = 0x514649FB00000000ULL; // QFI 0xFB 0x00 0x00 0x00
    constexpr auto RpmSignature = 0xEDABEEDB00000000ULL; // 0xED 0xAB 0xEE 0xDB
    constexpr auto SquashFSSignature1 = 0x7371736800000000ULL; // sqsh
    constexpr auto SquashFSSignature2 = 0x6873717300000000ULL; // hsqs
    constexpr auto SquashFSSignature3 = 0x7368737100000000ULL; // shsq
    constexpr auto SquashFSSignature4 = 0x7173687300000000ULL; // qshs
    constexpr auto SwfSignature = 0x4657530000000000ULL; // FWS
    constexpr auto SwfcSignature1 = 0x4357530000000000ULL; // CWS
    constexpr auto SwfcSignature2 = 0x5A57530000000000ULL; // ZWS
    constexpr auto TESignature = 0x565A000000000000ULL; // VZ
    constexpr auto VMDKSignature = 0x4B444D0000000000ULL; // KDMV
    constexpr auto VDISignature = 0x3C3C3C2000000000ULL; // Alternatively, 0x7F10DABE at offset 0x40
    constexpr auto VhdSignature = 0x636F6E6563746978ULL; // conectix
    constexpr auto XarSignature = 0x78617221001C0000ULL; // xar! 0x00 0x1C
    constexpr auto ZSignature1 = 0x1F9D000000000000ULL; // 0x1F 0x9D
    constexpr auto ZSignature2 = 0x1FA0000000000000ULL; // 0x1F 0xA0

    switch ( signature ) {
        case RarSignature:
            return &BitFormat::Rar;
        case Rar5Signature:
            return &BitFormat::Rar5;
        case SevenZipSignature:
            return &BitFormat::SevenZip;
        case BZip2Signature:
            return &BitFormat::BZip2;
        case GZipSignature:
            return &BitFormat::GZip;
        case WimSignature:
            return &BitFormat::Wim;
        case XzSignature:
            return &BitFormat::Xz;
        case ZipSignature:
            return &BitFormat::Zip;
        case APMSignature:
            return &BitFormat::APM;
        case ArjSignature:
            return &BitFormat::Arj;
        case CabSignature:
            return &BitFormat::Cab;
        case ChmSignature:
            return &BitFormat::Chm;
        case CompoundSignature:
            return &BitFormat::Compound;
        case CpioSignature1:
        case CpioSignature2:
        case CpioSignature3:
            return &BitFormat::Cpio;
        case DebSignature:
            return &BitFormat::Deb;
            /* DMG signature detection is not this simple
            case 0x7801730D62626000:
                return &BitFormat::Dmg;
            */
        case ElfSignature:
            return &BitFormat::Elf;
        case PeSignature:
            return &BitFormat::Pe;
        case FlvSignature:
            return &BitFormat::Flv;
        case LzmaSignature:
            return &BitFormat::Lzma;
        case Lzma86Signature:
            return &BitFormat::Lzma86;
        case MachoSignature1:
        case MachoSignature2:
        case MachoSignature3:
        case MachoSignature4:
            return &BitFormat::Macho;
        case MubSignature1:
        case MubSignature2:
            return &BitFormat::Mub;
        case MslzSignature:
            return &BitFormat::Mslz;
        case PpmdSignature:
            return &BitFormat::Ppmd;
        case QCowSignature:
            return &BitFormat::QCow;
        case RpmSignature:
            return &BitFormat::Rpm;
        case SquashFSSignature1:
        case SquashFSSignature2:
        case SquashFSSignature3:
        case SquashFSSignature4:
            return &BitFormat::SquashFS;
        case SwfSignature:
            return &BitFormat::Swf;
        case SwfcSignature1:
        case SwfcSignature2:
            return &BitFormat::Swfc;
        case TESignature:
            return &BitFormat::TE;
        case VMDKSignature: // K  D  M  V
            return &BitFormat::VMDK;
        case VDISignature: // Alternatively, 0x7F10DABE at offset 0x40
            return &BitFormat::VDI;
        case VhdSignature: // c  o  n  e  c  t  i  x
            return &BitFormat::Vhd;
        case XarSignature: // x  a  r  !  00 1C
            return &BitFormat::Xar;
        case ZSignature1: // 1F 9D
        case ZSignature2: // 1F A0
            return &BitFormat::Z;
        default:
            return nullptr;
    }
}

struct OffsetSignature {
    uint64_t signature;
    std::streamoff offset;
    uint32_t size;
    const BitInFormat& format;
};

#if defined(_WIN32)
#define bswap64 _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
//Note: the versions of gcc and clang that can compile bit7z should also have this builtin, hence there is no need
//      for checking the compiler version or using _has_builtin macro!
#define bswap64 __builtin_bswap64
#else
static inline uint64_t bswap64( uint64_t x ) {
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

auto readSignature( IInStream* stream, uint32_t size ) noexcept -> uint64_t {
    uint64_t signature = 0;
    stream->Read( &signature, size, nullptr );
    return bswap64( signature );
}

auto detectFormatFromSig( IInStream* stream ) -> const BitInFormat& {
    constexpr auto SIGNATURE_SIZE = 8U;
    constexpr auto BASE_SIGNATURE_MASK = 0xFFFFFFFFFFFFFFFFULL;
    constexpr auto BYTE_SHIFT = 8ULL;

    uint64_t file_signature = readSignature( stream, SIGNATURE_SIZE );
    uint64_t signature_mask = BASE_SIGNATURE_MASK;
    for ( auto i = 0U; i < SIGNATURE_SIZE - 1; ++i ) {
        const BitInFormat* format = findFormatBySignature( file_signature );
        if ( format != nullptr ) {
            stream->Seek( 0, 0, nullptr );
            return *format;
        }
        signature_mask <<= BYTE_SHIFT;    // left shifting the mask of one byte, so that
        file_signature &= signature_mask; // the least significant i bytes are masked (set to 0)
    }

    static const OffsetSignature common_signatures_with_offset[] = { // NOLINT(*-avoid-c-arrays)
        { 0x2D6C680000000000, 0x02,  3, BitFormat::Lzh },    // -lh
        { 0x4E54465320202020, 0x03,  8, BitFormat::Ntfs },   // NTFS 0x20 0x20 0x20 0x20
        { 0x4E756C6C736F6674, 0x08,  8, BitFormat::Nsis },   // Nullsoft
        { 0x436F6D7072657373, 0x10,  8, BitFormat::CramFS }, // Compress
        { 0x7F10DABE00000000, 0x40,  4, BitFormat::VDI },    // 0x7F 0x10 0xDA 0xBE
        { 0x7573746172000000, 0x101, 5, BitFormat::Tar },    // ustar
        // Note: since GPT files contain also the FAT signature, GPT must be checked before!
        { 0x4546492050415254, 0x200, 8, BitFormat::GPT },    // EFI 0x20 PART
        { 0x55AA000000000000, 0x1FE, 2, BitFormat::Fat },    // U 0xAA
        { 0x4244000000000000, 0x400, 2, BitFormat::Hfs },    // BD
        { 0x482B000400000000, 0x400, 4, BitFormat::Hfs },    // H+ 0x00 0x04
        { 0x4858000500000000, 0x400, 4, BitFormat::Hfs },   // HX 0x00 0x05
        { 0x53EF000000000000, 0x438, 2, BitFormat::Ext }    // S 0xEF
    };

    for ( const auto& sig : common_signatures_with_offset ) {
        stream->Seek( sig.offset, 0, nullptr );
        file_signature = readSignature( stream, sig.size );
        if ( file_signature == sig.signature ) {
            stream->Seek( 0, 0, nullptr );
            return sig.format;
        }
    }

    // Detecting ISO/UDF
    constexpr auto BEA_SIGNATURE = 0x4245413031000000; // BEA01 (beginning of the extended descriptor section)
    constexpr auto ISO_SIGNATURE = 0x4344303031000000; // CD001 (ISO format signature)
    constexpr auto ISO_SIGNATURE_SIZE = 5ULL;
    constexpr auto ISO_SIGNATURE_OFFSET = 0x8001;

    // Checking for ISO signature
    stream->Seek( ISO_SIGNATURE_OFFSET, 0, nullptr );
    file_signature = readSignature( stream, ISO_SIGNATURE_SIZE );

    const bool is_iso = file_signature == ISO_SIGNATURE;
    if ( is_iso || file_signature == BEA_SIGNATURE ) {
        constexpr auto MAX_VOLUME_DESCRIPTORS = 16;
        constexpr auto ISO_VOLUME_DESCRIPTOR_SIZE = 0x800; //2048

        constexpr auto UDF_SIGNATURE = 0x4E53523000000000; //NSR0
        constexpr auto UDF_SIGNATURE_SIZE = 4U;

        for ( auto descriptor_index = 1; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
            stream->Seek( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE, 0, nullptr );
            file_signature = readSignature( stream, UDF_SIGNATURE_SIZE );

            if ( file_signature == UDF_SIGNATURE ) { // The file is ISO+UDF or just UDF
                stream->Seek( 0, 0, nullptr );
                return BitFormat::Udf;
            }
        }

        if ( is_iso ) { // The file is pure ISO (no UDF).
            stream->Seek( 0, 0, nullptr );
            return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
        }
    }

    stream->Seek( 0, 0, nullptr );
    throw BitException( "Failed to detect the format of the file",
                        make_error_code( BitError::NoMatchingSignature ) );
}

#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
#   define is_digit(ch) std::iswdigit(ch) != 0
const auto to_lower = std::towlower;
#else

inline auto is_digit( unsigned char character ) -> bool {
    return std::isdigit( character ) != 0;
}

inline auto to_lower( unsigned char character ) -> char {
    return static_cast< char >( std::tolower( character ) );
}

#endif

auto detectFormatFromExt( const fs::path& in_file ) -> const BitInFormat& {
    tstring ext = filesystem::fsutil::extension( in_file );
    if ( ext.empty() ) {
        throw BitException( "Failed to detect the archive format from the extension",
                            make_error_code( BitError::NoMatchingExtension ) );
    }
    std::transform( ext.cbegin(), ext.cend(), ext.begin(), to_lower );

    // Detecting archives with common file extensions
    const BitInFormat* format = findFormatByExtension( ext );
    if ( format != nullptr ) { //extension found in the map
        return *format;
    }

    // Detecting multi-volume archives extensions
    if ( ( ext[ 0 ] == BIT7Z_STRING( 'r' ) || ext[ 0 ] == BIT7Z_STRING( 'z' ) ) &&
         ( ext.size() == 3 && is_digit( ext[ 1 ] ) && is_digit( ext[ 2 ] ) ) ) {
        // Extension follows the format zXX or rXX, where X is a number in range [0-9]
        return ext[ 0 ] == BIT7Z_STRING( 'r' ) ? BitFormat::Rar : BitFormat::Zip;
    }

    // Note: iso, img, and ima extensions can be associated with different formats -> detect by signature.

    // The extension did not match any known format extension, delegating the decision to the client.
    return BitFormat::Auto;
}
}  // namespace bit7z

#endif
