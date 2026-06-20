// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/formatdetect.hpp"

// Note: the formatdetect.hpp header must be included before this ifdef since the BIT7Z_AUTO_FORMAT
// flag might be manually specified in the bitdefines.hpp header (included by formatdetect.hpp).
#ifdef BIT7Z_AUTO_FORMAT

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitformat.hpp"
#include "bittypes.hpp"
#include "internal/fsutil.hpp"
#ifndef _WIN32
#include "internal/guiddef.hpp"
#endif

#include <7zip/IStream.h>

#include <algorithm>
#include <cstdint>
#include <ios>

#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
#include <cwctype> // for std::iswdigit
#else
#include <cctype> // for std::isdigit
#endif

#ifdef BIT7Z_DETECT_FROM_EXTENSION
namespace {
/**
 * @brief Constexpr recursive implementation of the djb2 hashing function.
 *
 * @param input The C string to be hashed.
 *
 * @return The hash integer.
 */
constexpr auto strHash( const bit7z::tchar* input ) -> std::uint64_t { // NOLINT(misc-no-recursion)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, *-magic-numbers)
    return *input != 0 ? static_cast< std::uint64_t >( *input ) + 33 * strHash( input + 1 ) : 5381; //-V2563
}
} // namespace
#endif

namespace bit7z {
namespace {
#ifdef BIT7Z_DETECT_FROM_EXTENSION
/* NOTE: Until v3, a std::unordered_map was used for mapping the extensions and the corresponding
 *       format, but the ifs are faster and have less memory footprint. */
auto findFormatByExtension( const tstring& extension ) -> const BitInFormat* {
    switch ( strHash( extension.c_str() ) ) {
        case strHash( BIT7Z_STRING( "7z" ) ):
            return &BitFormat::SevenZip;
        case strHash( BIT7Z_STRING( "bzip2" ) ):
        case strHash( BIT7Z_STRING( "bz2" ) ):
        case strHash( BIT7Z_STRING( "tbz2" ) ):
        case strHash( BIT7Z_STRING( "tbz" ) ):
            return &BitFormat::BZip2;
        case strHash( BIT7Z_STRING( "gz" ) ):
        case strHash( BIT7Z_STRING( "gzip" ) ):
        case strHash( BIT7Z_STRING( "tgz" ) ):
            return &BitFormat::GZip;
        case strHash( BIT7Z_STRING( "tar" ) ):
        case strHash( BIT7Z_STRING( "ova" ) ):
            return &BitFormat::Tar;
        case strHash( BIT7Z_STRING( "wim" ) ):
        case strHash( BIT7Z_STRING( "swm" ) ):
            return &BitFormat::Wim;
        case strHash( BIT7Z_STRING( "xz" ) ):
        case strHash( BIT7Z_STRING( "txz" ) ):
            return &BitFormat::Xz;
        case strHash( BIT7Z_STRING( "zip" ) ):
        case strHash( BIT7Z_STRING( "zipx" ) ):
        case strHash( BIT7Z_STRING( "jar" ) ):
        case strHash( BIT7Z_STRING( "xpi" ) ):
        case strHash( BIT7Z_STRING( "odt" ) ):
        case strHash( BIT7Z_STRING( "ods" ) ):
        case strHash( BIT7Z_STRING( "odp" ) ):
        case strHash( BIT7Z_STRING( "docx" ) ):
        case strHash( BIT7Z_STRING( "xlsx" ) ):
        case strHash( BIT7Z_STRING( "pptx" ) ):
        case strHash( BIT7Z_STRING( "epub" ) ):
            return &BitFormat::Zip;
        case strHash( BIT7Z_STRING( "001" ) ):
            return &BitFormat::Split;
        case strHash( BIT7Z_STRING( "ar" ) ):
        case strHash( BIT7Z_STRING( "deb" ) ):
            return &BitFormat::Deb;
        case strHash( BIT7Z_STRING( "apfs" ) ):
            return &BitFormat::APFS;
        case strHash( BIT7Z_STRING( "avb" ) ):
            return &BitFormat::AVB;
        case strHash( BIT7Z_STRING( "apm" ) ):
            return &BitFormat::APM;
        case strHash( BIT7Z_STRING( "arj" ) ):
            return &BitFormat::Arj;
        case strHash( BIT7Z_STRING( "cab" ) ):
            return &BitFormat::Cab;
        case strHash( BIT7Z_STRING( "chm" ) ):
        case strHash( BIT7Z_STRING( "chi" ) ):
            return &BitFormat::Chm;
        case strHash( BIT7Z_STRING( "msi" ) ):
        case strHash( BIT7Z_STRING( "doc" ) ):
        case strHash( BIT7Z_STRING( "xls" ) ):
        case strHash( BIT7Z_STRING( "ppt" ) ):
        case strHash( BIT7Z_STRING( "msg" ) ):
            return &BitFormat::Compound;
        case strHash( BIT7Z_STRING( "obj" ) ):
            return &BitFormat::COFF;
        case strHash( BIT7Z_STRING( "cpio" ) ):
            return &BitFormat::Cpio;
        case strHash( BIT7Z_STRING( "cramfs" ) ):
            return &BitFormat::CramFS;
        case strHash( BIT7Z_STRING( "dmg" ) ):
            return &BitFormat::Dmg;
        case strHash( BIT7Z_STRING( "dll" ) ):
        case strHash( BIT7Z_STRING( "exe" ) ):
            //note: at least for now, we do not distinguish 7z SFX executables.
            return &BitFormat::Pe;
        case strHash( BIT7Z_STRING( "dylib" ) ):
            return &BitFormat::Macho;
        case strHash( BIT7Z_STRING( "ext" ) ):
        case strHash( BIT7Z_STRING( "ext2" ) ):
        case strHash( BIT7Z_STRING( "ext3" ) ):
        case strHash( BIT7Z_STRING( "ext4" ) ):
            return &BitFormat::Ext;
        case strHash( BIT7Z_STRING( "fat" ) ):
            return &BitFormat::Fat;
        case strHash( BIT7Z_STRING( "flv" ) ):
            return &BitFormat::Flv;
        case strHash( BIT7Z_STRING( "gpt" ) ):
            return &BitFormat::GPT;
        case strHash( BIT7Z_STRING( "hfs" ) ):
        case strHash( BIT7Z_STRING( "hfsx" ) ):
            return &BitFormat::Hfs;
        case strHash( BIT7Z_STRING( "hxs" ) ):
            return &BitFormat::Hxs;
        case strHash( BIT7Z_STRING( "ihex" ) ):
            return &BitFormat::IHex;
        case strHash( BIT7Z_STRING( "lzh" ) ):
        case strHash( BIT7Z_STRING( "lha" ) ):
            return &BitFormat::Lzh;
        case strHash( BIT7Z_STRING( "lpimg" ) ):
            return &BitFormat::LP;
        case strHash( BIT7Z_STRING( "lvm" ) ):
            return &BitFormat::LVM;
        case strHash( BIT7Z_STRING( "lzma" ) ):
            return &BitFormat::Lzma;
        case strHash( BIT7Z_STRING( "lzma86" ) ):
            return &BitFormat::Lzma86;
        case strHash( BIT7Z_STRING( "mbr" ) ):
            return &BitFormat::Mbr;
        case strHash( BIT7Z_STRING( "mslz" ) ):
            return &BitFormat::Mslz;
        case strHash( BIT7Z_STRING( "mub" ) ):
            return &BitFormat::Mub;
        case strHash( BIT7Z_STRING( "nsis" ) ):
            return &BitFormat::Nsis;
        case strHash( BIT7Z_STRING( "ntfs" ) ):
            return &BitFormat::Ntfs;
        case strHash( BIT7Z_STRING( "pmd" ) ):
        case strHash( BIT7Z_STRING( "ppmd" ) ):
            return &BitFormat::Ppmd;
        case strHash( BIT7Z_STRING( "qcow" ) ):
        case strHash( BIT7Z_STRING( "qcow2" ) ):
        case strHash( BIT7Z_STRING( "qcow2c" ) ):
            return &BitFormat::QCow;
        case strHash( BIT7Z_STRING( "rpm" ) ):
            return &BitFormat::Rpm;
        case strHash( BIT7Z_STRING( "simg" ) ):
            return &BitFormat::Sparse;
        case strHash( BIT7Z_STRING( "squashfs" ) ):
            return &BitFormat::SquashFS;
        case strHash( BIT7Z_STRING( "swf" ) ):
            return &BitFormat::Swf;
        case strHash( BIT7Z_STRING( "te" ) ):
            return &BitFormat::TE;
        case strHash( BIT7Z_STRING( "udf" ) ):
            return &BitFormat::Udf;
        case strHash( BIT7Z_STRING( "scap" ) ):
            return &BitFormat::UEFIc;
        case strHash( BIT7Z_STRING( "uefif" ) ):
            return &BitFormat::UEFIs;
        case strHash( BIT7Z_STRING( "vmdk" ) ):
            return &BitFormat::VMDK;
        case strHash( BIT7Z_STRING( "vdi" ) ):
            return &BitFormat::VDI;
        case strHash( BIT7Z_STRING( "vhd" ) ):
            return &BitFormat::Vhd;
        case strHash( BIT7Z_STRING( "vhdx" ) ):
        case strHash( BIT7Z_STRING( "avhdx" ) ):
            return &BitFormat::Vhdx;
        case strHash( BIT7Z_STRING( "xar" ) ):
        case strHash( BIT7Z_STRING( "pkg" ) ):
            return &BitFormat::Xar;
        case strHash( BIT7Z_STRING( "z" ) ):
        case strHash( BIT7Z_STRING( "taz" ) ):
            return &BitFormat::Z;
        case strHash( BIT7Z_STRING( "zst" ) ):
            return &BitFormat::Zstd;
        default:
            return nullptr;
    }
}
#endif

/* NOTE 1: For signatures with less than 8 bytes (size of std::uint64_t), remaining bytes are set to 0
 * NOTE 2: Until v3, a std::unordered_map was used for mapping the signatures and the corresponding
 *         format. However, the switch case is faster and has less memory footprint. */
auto findFormatBySignature( std::uint64_t signature ) noexcept -> const BitInFormat* {
    // cppcheck-suppress-begin unreadVariable
    constexpr auto kRarSignature = 0x526172211A070000ULL; // Rar! 0x1A 0x07 0x00
    constexpr auto kRar5Signature = 0x526172211A070100ULL; // Rar! 0x1A 0x07 0x01 0x00
    constexpr auto kSevenzipSignature = 0x377ABCAF271C0000ULL; // 7z 0xBC 0xAF 0x27 0x1C
    constexpr auto kBzip2Signature = 0x425A680000000000ULL; // BZh
    constexpr auto kGzipSignature = 0x1F8B080000000000ULL; // 0x1F 0x8B 0x08
    constexpr auto kWimSignature = 0x4D5357494D000000ULL; // MSWIM 0x00 0x00 0x00
    constexpr auto kXzSignature = 0xFD377A585A000000ULL; // 0xFD 7zXZ 0x00
    constexpr auto kZipSignature = 0x504B000000000000ULL; // PK
    constexpr auto kAvbSignature = 0x4156426600000000ULL; // AVBf 0x00 0x00 0x00
    constexpr auto kApmSignature = 0x4552000000000000ULL; // ER
    constexpr auto kArjSignature = 0x60EA000000000000ULL; // `EA
    constexpr auto kCabSignature = 0x4D53434600000000ULL; // MSCF 0x00 0x00 0x00 0x00
    constexpr auto kChmSignature = 0x4954534603000000ULL; // ITSF 0x03
    constexpr auto kCompoundSignature = 0xD0CF11E0A1B11AE1ULL; // 0xD0 0xCF 0x11 0xE0 0xA1 0xB1 0x1A 0xE1
    constexpr auto kCpioSignature1 = 0xC771000000000000ULL; // 0xC7 q
    constexpr auto kCpioSignature2 = 0x71C7000000000000ULL; // q 0xC7
    constexpr auto kCpioSignature3 = 0x3037303730000000ULL; // 07070
    constexpr auto kDebSignature = 0x213C617263683E00ULL; // !<arch>0A
    constexpr auto kElfSignature = 0x7F454C4600000000ULL; // 0x7F ELF
    constexpr auto kPeSignature = 0x4D5A000000000000ULL; // MZ
    constexpr auto kFlvSignature = 0x464C560100000000ULL; // FLV 0x01
    constexpr auto kLpSignature = 0x67446C6134000000ULL;
    constexpr auto kLvmSignature = 0x4C4142454C4F4E45; // LABELONE
    constexpr auto kLzmaSignature = 0x5D00000000000000ULL; //
    constexpr auto kLzma86Signature = 0x015D000000000000ULL; //
    constexpr auto kMachoSignature1 = 0xCEFAEDFE00000000ULL; // 0xCE 0xFA 0xED 0xFE
    constexpr auto kMachoSignature2 = 0xCFFAEDFE00000000ULL; // 0xCF 0xFA 0xED 0xFE
    constexpr auto kMachoSignature3 = 0xFEEDFACE00000000ULL; // 0xFE 0xED 0xFA 0xCE
    constexpr auto kMachoSignature4 = 0xFEEDFACF00000000ULL; // 0xFE 0xED 0xFA 0xCF
    constexpr auto kMubSignature1 = 0xCAFEBABE00000000ULL; // 0xCA 0xFE 0xBA 0xBE 0x00 0x00 0x00
    constexpr auto kMubSignature2 = 0xB9FAF10E00000000ULL; // 0xB9 0xFA 0xF1 0x0E
    constexpr auto kMslzSignature = 0x535A444488F02733ULL; // SZDD 0x88 0xF0 '3
    constexpr auto kPpmdSignature = 0x8FAFAC8400000000ULL; // 0x8F 0xAF 0xAC 0x84
    constexpr auto kQcowSignature = 0x514649FB00000000ULL; // QFI 0xFB 0x00 0x00 0x00
    constexpr auto kRpmSignature = 0xEDABEEDB00000000ULL; // 0xED 0xAB 0xEE 0xDB
    constexpr auto kSparseSignature = 0x3AFF26ED00000000ULL; // 0x3A 0xFF 0x26 0xED
    constexpr auto kSquashfsSignature1 = 0x7371736800000000ULL; // sqsh
    constexpr auto kSquashfsSignature2 = 0x6873717300000000ULL; // hsqs
    constexpr auto kSquashfsSignature3 = 0x7368737100000000ULL; // shsq
    constexpr auto kSquashfsSignature4 = 0x7173687300000000ULL; // qshs
    constexpr auto kSwfSignature = 0x4657530000000000ULL; // FWS
    constexpr auto kSwfcSignature1 = 0x4357530000000000ULL; // CWS
    constexpr auto kSwfcSignature2 = 0x5A57530000000000ULL; // ZWS
    constexpr auto kTeSignature = 0x565A000000000000ULL; // VZ
    constexpr auto kVmdkSignature = 0x4B444D0000000000ULL; // KDMV
    constexpr auto kVdiSignature = 0x3C3C3C2000000000ULL; // Alternatively, 0x7F10DABE at offset 0x40
    constexpr auto kVhdSignature = 0x636F6E6563746978ULL; // conectix
    constexpr auto kVhdxSignature = 0x7668647866696C65; // vhdxfile
    constexpr auto kXarSignature = 0x78617221001C0000ULL; // xar! 0x00 0x1C
    constexpr auto kZSignature1 = 0x1F9D000000000000ULL; // 0x1F 0x9D
    constexpr auto kZSignature2 = 0x1FA0000000000000ULL; // 0x1F 0xA0
    constexpr auto kZstdSignature = 0x28B52FFD00000000ULL;
    // cppcheck-suppress-end unreadVariable

    switch ( signature ) {
        case kRarSignature:
            return &BitFormat::Rar;
        case kRar5Signature:
            return &BitFormat::Rar5;
        case kSevenzipSignature:
            return &BitFormat::SevenZip;
        case kBzip2Signature:
            return &BitFormat::BZip2;
        case kGzipSignature:
            return &BitFormat::GZip;
        case kWimSignature:
            return &BitFormat::Wim;
        case kXzSignature:
            return &BitFormat::Xz;
        case kZipSignature:
            return &BitFormat::Zip;
        case kAvbSignature:
            return &BitFormat::AVB;
        case kApmSignature:
            return &BitFormat::APM;
        case kArjSignature:
            return &BitFormat::Arj;
        case kCabSignature:
            return &BitFormat::Cab;
        case kChmSignature:
            return &BitFormat::Chm;
        case kCompoundSignature:
            return &BitFormat::Compound;
        case kCpioSignature1:
        case kCpioSignature2:
        case kCpioSignature3:
            return &BitFormat::Cpio;
        case kDebSignature:
            return &BitFormat::Deb;
        /* DMG signature detection is not so easy to detect.
        case 0x7801730D62626000:
            return &BitFormat::Dmg;
        */
        case kElfSignature:
            return &BitFormat::Elf;
        case kPeSignature:
            return &BitFormat::Pe;
        case kFlvSignature:
            return &BitFormat::Flv;
        case kLpSignature:
            return &BitFormat::LP;
        case kLvmSignature:
            return &BitFormat::LVM;
        case kLzmaSignature:
            return &BitFormat::Lzma;
        case kLzma86Signature:
            return &BitFormat::Lzma86;
        case kMachoSignature1:
        case kMachoSignature2:
        case kMachoSignature3:
        case kMachoSignature4:
            return &BitFormat::Macho;
        case kMubSignature1:
        case kMubSignature2:
            return &BitFormat::Mub;
        case kMslzSignature:
            return &BitFormat::Mslz;
        case kPpmdSignature:
            return &BitFormat::Ppmd;
        case kQcowSignature:
            return &BitFormat::QCow;
        case kRpmSignature:
            return &BitFormat::Rpm;
        case kSparseSignature:
            return &BitFormat::Sparse;
        case kSquashfsSignature1:
        case kSquashfsSignature2:
        case kSquashfsSignature3:
        case kSquashfsSignature4:
            return &BitFormat::SquashFS;
        case kSwfSignature:
            return &BitFormat::Swf;
        case kSwfcSignature1:
        case kSwfcSignature2:
            return &BitFormat::Swfc;
        case kTeSignature:
            return &BitFormat::TE;
        case kVmdkSignature: // K  D  M  V
            return &BitFormat::VMDK;
        case kVdiSignature: // Alternatively, 0x7F10DABE at offset 0x40
            return &BitFormat::VDI;
        case kVhdSignature: // c  o  n  e  c  t  i  x
            return &BitFormat::Vhd;
        case kVhdxSignature: // v  h  d  x  f  i  l  e
            return &BitFormat::Vhdx;
        case kXarSignature: // x  a  r  !  00 1C
            return &BitFormat::Xar;
        case kZSignature1: // 1F 9D
        case kZSignature2: // 1F A0
            return &BitFormat::Z;
        case kZstdSignature:
            return &BitFormat::Zstd;
        default:
            return nullptr;
    }
}

struct OffsetSignature { // NOLINT(*-member-init)
    std::uint64_t signature;
    std::streamoff offset;
    std::uint32_t size;
    std::reference_wrapper< const BitInFormat > format;
};
} // namespace

#ifdef _WIN32
#define bswap64 _byteswap_uint64
#elif defined(__GNUC__) || defined(__clang__)
//Note: the versions of gcc and clang that can compile bit7z should also have this builtin, hence there is no need
//      for checking the compiler version or using the _has_builtin macro.
#ifndef bswap64
#define bswap64 __builtin_bswap64
#endif
#else
static inline std::uint64_t bswap64( std::uint64_t x ) {
    return ( ( x << 56 ) & 0xff00000000000000ULL ) |
           ( ( x << 40 ) & 0x00ff000000000000ULL ) |
           ( ( x << 24 ) & 0x0000ff0000000000ULL ) |
           ( ( x << 8 ) & 0x000000ff00000000ULL ) |
           ( ( x >> 8 ) & 0x00000000ff000000ULL ) |
           ( ( x >> 24 ) & 0x0000000000ff0000ULL ) |
           ( ( x >> 40 ) & 0x000000000000ff00ULL ) |
           ( ( x >> 56 ) & 0x00000000000000ffULL );
}
#endif

namespace {
auto readSignature( IInStream* stream, std::uint32_t size ) noexcept -> std::uint64_t {
    std::uint64_t signature = 0;
    ( void )stream->Read( &signature, size, nullptr );
    return bswap64( signature );
}
} // namespace

// Note: the left shifting of the signature mask might overflow, but it is intentional, so we suppress the sanitizer.
auto detectFormatFromSignature( IInStream* stream ) -> const BitInFormat& {
    constexpr auto kSignatureSize = 8U;
    constexpr auto kBaseSignatureMask = 0xFFFFFFFFFFFFFFFFULL;
    constexpr auto kByteShift = 8ULL;

    std::uint64_t fileSignature = readSignature( stream, kSignatureSize );
    std::uint64_t signatureMask = kBaseSignatureMask;
    for ( auto i = 0U; i < kSignatureSize - 1; ++i ) {
        const BitInFormat* format = findFormatBySignature( fileSignature );
        if ( format != nullptr ) {
            ( void )stream->Seek( 0, 0, nullptr );
            return *format;
        }
        signatureMask <<= kByteShift; // left shifting the mask of one byte, so that
        fileSignature &= signatureMask; // the least significant i bytes are masked (set to 0)
    }

    static const OffsetSignature commonSignaturesWithOffset[ ] = { // NOLINT(*-avoid-c-arrays)
        { 0x2D6C680000000000, 0x02, 3, BitFormat::Lzh }, // -lh
        { 0x4E54465320202020, 0x03, 8, BitFormat::Ntfs }, // NTFS 0x20 0x20 0x20 0x20
        { 0x4E756C6C736F6674, 0x08, 8, BitFormat::Nsis }, // Nullsoft
        { 0x436F6D7072657373, 0x10, 8, BitFormat::CramFS }, // Compress
        { 0x4E58534200000000, 0x20, 4, BitFormat::APFS }, // NXSB
        { 0x7F10DABE00000000, 0x40, 4, BitFormat::VDI }, // 0x7F 0x10 0xDA 0xBE
        { 0x7573746172000000, 0x101, 5, BitFormat::Tar }, // ustar
        /* Note: since GPT files contain also the FAT signature, we must check the GPT signature before the FAT one. */
        { 0x4546492050415254, 0x200, 8, BitFormat::GPT }, // EFI 0x20 PART
        { 0x55AA000000000000, 0x1FE, 2, BitFormat::Fat }, // U 0xAA
        { 0x4244000000000000, 0x400, 2, BitFormat::Hfs }, // BD
        { 0x482B000400000000, 0x400, 4, BitFormat::Hfs }, // H+ 0x00 0x04
        { 0x4858000500000000, 0x400, 4, BitFormat::Hfs }, // HX 0x00 0x05
        { 0x53EF000000000000, 0x438, 2, BitFormat::Ext } // S 0xEF
    };

    for ( const auto& sig : commonSignaturesWithOffset ) {
        ( void )stream->Seek( sig.offset, 0, nullptr );
        fileSignature = readSignature( stream, sig.size );
        if ( fileSignature == sig.signature ) {
            ( void )stream->Seek( 0, 0, nullptr );
            return sig.format;
        }
    }

    // Detecting ISO/UDF
    constexpr auto kBeaSignature = 0x4245413031000000; // BEA01 (beginning of the extended descriptor section)
    constexpr auto kIsoSignature = 0x4344303031000000; // CD001 (ISO format signature)
    constexpr auto kIsoSignatureSize = 5ULL;
    constexpr auto kIsoSignatureOffset = 0x8001;

    // Checking for ISO signature
    ( void )stream->Seek( kIsoSignatureOffset, 0, nullptr );
    fileSignature = readSignature( stream, kIsoSignatureSize );

    const bool isIso = fileSignature == kIsoSignature;
    if ( isIso || fileSignature == kBeaSignature ) {
        constexpr auto kMaxVolumeDescriptors = 16;
        constexpr auto kIsoVolumeDescriptorSize = 0x800; //2048

        constexpr auto kUdfSignature = 0x4E53523000000000; //NSR0
        constexpr auto kUdfSignatureSize = 4U;

        for ( auto descriptorIndex = 1; descriptorIndex < kMaxVolumeDescriptors; ++descriptorIndex ) {
            ( void )stream->Seek( kIsoSignatureOffset + ( descriptorIndex * kIsoVolumeDescriptorSize ), 0, nullptr );
            fileSignature = readSignature( stream, kUdfSignatureSize );

            if ( fileSignature == kUdfSignature ) { // The file is ISO+UDF or just UDF
                ( void )stream->Seek( 0, 0, nullptr );
                return BitFormat::Udf;
            }
        }

        if ( isIso ) { // The file is pure ISO (no UDF).
            ( void )stream->Seek( 0, 0, nullptr );
            return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
        }
    }

    ( void )stream->Seek( 0, 0, nullptr );
    throw BitException(
        "Failed to detect the format of the file",
        make_error_code( BitError::NoMatchingSignature )
    );
}

#ifdef BIT7Z_DETECT_FROM_EXTENSION
namespace {
BIT7Z_ALWAYS_INLINE
auto isDigit( tchar character ) -> bool {
#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
    return std::iswdigit( character ) != 0;
#else
    return std::isdigit( character ) != 0;
#endif
}

#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
const auto toLower = std::towlower;
#else
auto toLower( unsigned char character ) -> char {
    return static_cast< char >( std::tolower( character ) );
}
#endif
} // namespace

auto detectFormatFromExtension( const fs::path& inFile ) -> const BitInFormat& {
    tstring ext = filesystem::fsutil::extension( inFile );
    if ( ext.empty() ) {
        return BitFormat::Auto;
    }
    std::transform( ext.cbegin(), ext.cend(), ext.begin(), toLower );

    // Detecting archives with common file extensions
    const BitInFormat* format = findFormatByExtension( ext );
    if ( format != nullptr ) { //extension found in the map
        return *format;
    }

    // Detecting multi-volume archives extensions
    if ( ( ext[ 0 ] == BIT7Z_STRING( 'r' ) || ext[ 0 ] == BIT7Z_STRING( 'z' ) ) &&
         ( ext.size() == 3 && isDigit( ext[ 1 ] ) && isDigit( ext[ 2 ] ) ) ) {
        // Extension follows the format zXX or rXX, where X is a number in range [0-9]
        return ext[ 0 ] == BIT7Z_STRING( 'r' ) ? BitFormat::Rar : BitFormat::Zip;
    }

    // Note: iso, img, and ima extensions can be associated with different formats -> detect by signature.

    // The extension did not match any known format extension, delegating the decision to the client.
    return BitFormat::Auto;
}
#endif
} // namespace bit7z

#endif
