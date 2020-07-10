#ifdef BIT7Z_AUTO_FORMAT

#include "../include/bitexception.hpp"
#include "../include/formatdetect.hpp"
#include "../include/fsutil.hpp"

#ifdef _WIN32
#include <cwctype>
#else
#include <cctype>
#endif

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

namespace bit7z {
    unsigned constexpr str_hash( tchar const* input ) {
        return *input ? static_cast< unsigned int >( *input ) + 33 * str_hash( input + 1 ) : 5381;
    }

    /* NOTE: Until v3, a std::unordered_map was used for mapping the extensions and the corresponding
     *       format, however the ifs are faster and have less memory footprint. */
    bool findFormatByExtension( const tstring& ext, const BitInFormat** format ) {
        switch ( str_hash( ext.c_str() ) ) {
            case str_hash( TSTRING( "7z" ) ):
                *format = &BitFormat::SevenZip;
                return true;
            case str_hash( TSTRING( "bzip2" ) ):
            case str_hash( TSTRING( "bz2" ) ):
            case str_hash( TSTRING( "tbz2" ) ):
            case str_hash( TSTRING( "tbz" ) ):
                *format = &BitFormat::BZip2;
                return true;
            case str_hash( TSTRING( "gz" ) ):
            case str_hash( TSTRING( "gzip" ) ):
            case str_hash( TSTRING( "tgz" ) ):
                *format = &BitFormat::GZip;
                return true;
            case str_hash( TSTRING( "tar" ) ):
                *format = &BitFormat::Tar;
                return true;
            case str_hash( TSTRING( "wim" ) ):
            case str_hash( TSTRING( "swm" ) ):
                *format = &BitFormat::Wim;
                return true;
            case str_hash( TSTRING( "xz" ) ):
            case str_hash( TSTRING( "txz" ) ):
                *format = &BitFormat::Xz;
                return true;
            case str_hash( TSTRING( "zip" ) ):
            case str_hash( TSTRING( "zipx" ) ):
            case str_hash( TSTRING( "jar" ) ):
            case str_hash( TSTRING( "xpi" ) ):
            case str_hash( TSTRING( "odt" ) ):
            case str_hash( TSTRING( "ods" ) ):
            case str_hash( TSTRING( "odp" ) ):
            case str_hash( TSTRING( "docx" ) ):
            case str_hash( TSTRING( "xlsx" ) ):
            case str_hash( TSTRING( "pptx" ) ):
            case str_hash( TSTRING( "epub" ) ):
                *format = &BitFormat::Zip;
                return true;
            case str_hash( TSTRING( "001" ) ):
                *format = &BitFormat::Split;
                return true;
            case str_hash( TSTRING( "ar" ) ):
                *format = &BitFormat::Deb;
                return true;
            case str_hash( TSTRING( "apm" ) ):
                *format = &BitFormat::APM;
                return true;
            case str_hash( TSTRING( "arj" ) ):
                *format = &BitFormat::Arj;
                return true;
            case str_hash( TSTRING( "cab" ) ):
                *format = &BitFormat::Cab;
                return true;
            case str_hash( TSTRING( "chm" ) ):
            case str_hash( TSTRING( "chi" ) ):
                *format = &BitFormat::Chm;
                return true;
            case str_hash( TSTRING( "msi" ) ):
            case str_hash( TSTRING( "doc" ) ):
            case str_hash( TSTRING( "xls" ) ):
            case str_hash( TSTRING( "ppt" ) ):
            case str_hash( TSTRING( "msg" ) ):
                *format = &BitFormat::Compound;
                return true;
            case str_hash( TSTRING( "obj" ) ):
                *format = &BitFormat::COFF;
                return true;
            case str_hash( TSTRING( "cpio" ) ):
                *format = &BitFormat::Cpio;
                return true;
            case str_hash( TSTRING( "cramfs" ) ):
                *format = &BitFormat::CramFS;
                return true;
            case str_hash( TSTRING( "deb" ) ):
                *format = &BitFormat::Deb;
                return true;
            case str_hash( TSTRING( "dmg" ) ):
                *format = &BitFormat::Dmg;
                return true;
            case str_hash( TSTRING( "dll" ) ):
            case str_hash( TSTRING( "exe" ) ):
                //note: we do not distinguish 7z SFX exe at the moment!
                *format = &BitFormat::Pe;
                return true;
            case str_hash( TSTRING( "dylib" ) ):
                *format = &BitFormat::Macho;
                return true;
            case str_hash( TSTRING( "ext" ) ):
            case str_hash( TSTRING( "ext2" ) ):
            case str_hash( TSTRING( "ext3" ) ):
            case str_hash( TSTRING( "ext4" ) ):
                *format = &BitFormat::Ext;
                return true;
            case str_hash( TSTRING( "fat" ) ):
                *format = &BitFormat::Fat;
                return true;
            case str_hash( TSTRING( "flv" ) ):
                *format = &BitFormat::Flv;
                return true;
            case str_hash( TSTRING( "gpt" ) ):
                *format = &BitFormat::GPT;
                return true;
            case str_hash( TSTRING( "hfs" ) ):
            case str_hash( TSTRING( "hfsx" ) ):
                *format = &BitFormat::Hfs;
                return true;
            case str_hash( TSTRING( "hxs" ) ):
                *format = &BitFormat::Hxs;
                return true;
            case str_hash( TSTRING( "ihex" ) ):
                *format = &BitFormat::IHex;
                return true;
            case str_hash( TSTRING( "lzh" ) ):
            case str_hash( TSTRING( "lha" ) ):
                *format = &BitFormat::Lzh;
                return true;
            case str_hash( TSTRING( "lzma" ) ):
                *format = &BitFormat::Lzma;
                return true;
            case str_hash( TSTRING( "lzma86" ) ):
                *format = &BitFormat::Lzma86;
                return true;
            case str_hash( TSTRING( "mbr" ) ):
                *format = &BitFormat::Mbr;
                return true;
            case str_hash( TSTRING( "mslz" ) ):
                *format = &BitFormat::Mslz;
                return true;
            case str_hash( TSTRING( "mub" ) ):
                *format = &BitFormat::Mub;
                return true;
            case str_hash( TSTRING( "nsis" ) ):
                *format = &BitFormat::Nsis;
                return true;
            case str_hash( TSTRING( "ntfs" ) ):
                *format = &BitFormat::Ntfs;
                return true;
            case str_hash( TSTRING( "pmd" ) ):
                *format = &BitFormat::Ppmd;
                return true;
            case str_hash( TSTRING( "qcow" ) ):
            case str_hash( TSTRING( "qcow2" ) ):
            case str_hash( TSTRING( "qcow2c" ) ):
                *format = &BitFormat::QCow;
                return true;
            case str_hash( TSTRING( "rpm" ) ):
                *format = &BitFormat::Rpm;
                return true;
            case str_hash( TSTRING( "squashfs" ) ):
                *format = &BitFormat::SquashFS;
                return true;
            case str_hash( TSTRING( "te" ) ):
                *format = &BitFormat::TE;
                return true;
            case str_hash( TSTRING( "udf" ) ):
                *format = &BitFormat::Udf;
                return true;
            case str_hash( TSTRING( "scap" ) ):
                *format = &BitFormat::UEFIc;
                return true;
            case str_hash( TSTRING( "uefif" ) ):
                *format = &BitFormat::UEFIs;
                return true;
            case str_hash( TSTRING( "vmdk" ) ):
                *format = &BitFormat::VMDK;
                return true;
            case str_hash( TSTRING( "vdi" ) ):
                *format = &BitFormat::VDI;
                return true;
            case str_hash( TSTRING( "vhd" ) ):
                *format = &BitFormat::Vhd;
                return true;
            case str_hash( TSTRING( "xar" ) ):
            case str_hash( TSTRING( "pkg" ) ):
                *format = &BitFormat::Xar;
                return true;
            case str_hash( TSTRING( "z" ) ):
            case str_hash( TSTRING( "taz" ) ):
                *format = &BitFormat::Z;
                return true;
            default:
                return false;
        }
    }

    /* NOTE 1: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0
     * NOTE 2: Until v3, a std::unordered_map was used for mapping the signatures and the corresponding
     *         format, however the switch case is faster and has less memory footprint. */
    bool findFormatBySignature( uint64_t signature, const BitInFormat** format ) {
        constexpr auto RarSignature = 0x526172211A070000ull; // R  a  r  !  1A 07 00
        constexpr auto Rar5Signature = 0x526172211A070100ull; // R  a  r  !  1A 07 01 00
        constexpr auto SevenZipSignature = 0x377ABCAF271C0000ull; // 7  z  BC AF 27 1C
        constexpr auto BZip2Signature = 0x425A680000000000ull; // B  Z  h
        constexpr auto GZipSignature = 0x1F8B080000000000ull; // 1F 8B 08
        constexpr auto WimSignature = 0x4D5357494D000000ull; // M  S  W  I  M  00 00 00
        constexpr auto XzSignature = 0xFD377A585A000000ull; // FD 7  z  X  Z  00
        constexpr auto ZipSignature = 0x504B000000000000ull; // P  K
        constexpr auto APMSignature = 0x4552000000000000ull; // E  R
        constexpr auto ArjSignature = 0x60EA000000000000ull; // `  EA
        constexpr auto CabSignature = 0x4D53434600000000ull; // M  S  C  F  00 00 00 00
        constexpr auto ChmSignature = 0x4954534603000000ull; // I  T  S  F  03
        constexpr auto CompoundSignature = 0xD0CF11E0A1B11AE1ull; // D0 CF 11 E0 A1 B1 1A E1
        constexpr auto CpioSignature1 = 0xC771000000000000ull; // C7 q
        constexpr auto CpioSignature2 = 0x71C7000000000000ull; // q  C7
        constexpr auto CpioSignature3 = 0x3037303730000000ull; // 0  7  0  7  0
        constexpr auto DebSignature = 0x213C617263683E00ull; // !  <  a  r  c  h  >  0A
        constexpr auto ElfSignature = 0x7F454C4600000000ull; // 7F E  L  F
        constexpr auto PeSignature = 0x4D5A000000000000ull; // M  Z
        constexpr auto FlvSignature = 0x464C560100000000ull; // F  L  V  01
        constexpr auto LzmaSignature = 0x5D00000000000000ull; //
        constexpr auto Lzma86Signature = 0x015D000000000000ull; //
        constexpr auto MachoSignature1 = 0xCEFAEDFE00000000ull; // CE FA ED FE
        constexpr auto MachoSignature2 = 0xCFFAEDFE00000000ull; // CF FA ED FE
        constexpr auto MachoSignature3 = 0xFEEDFACE00000000ull; // FE ED FA CE
        constexpr auto MachoSignature4 = 0xFEEDFACF00000000ull; // FE ED FA CF
        constexpr auto MubSignature1 = 0xCAFEBABE00000000ull; // CA FE BA BE 00 00 00
        constexpr auto MubSignature2 = 0xB9FAF10E00000000ull; // B9 FA F1 0E
        constexpr auto MslzSignature = 0x535A444488F02733ull; // S  Z  D  D  88 F0 '  3
        constexpr auto PpmdSignature = 0x8FAFAC8400000000ull; // 8F AF AC 84
        constexpr auto QCowSignature = 0x514649FB00000000ull; // Q  F  I  FB 00 00 00
        constexpr auto RpmSignature = 0xEDABEEDB00000000ull; // ED AB EE DB
        constexpr auto SquashFSSignature1 = 0x7371736800000000ull; // s  q  s  h
        constexpr auto SquashFSSignature2 = 0x6873717300000000ull; // h  s  q  s
        constexpr auto SquashFSSignature3 = 0x7368737100000000ull; // s  h  s  q
        constexpr auto SquashFSSignature4 = 0x7173687300000000ull; // q  s  h  s
        constexpr auto SwfSignature = 0x4657530000000000ull; // F  W  S
        constexpr auto SwfcSignature1 = 0x4357530000000000ull; // C  W  S
        constexpr auto SwfcSignature2 = 0x5A57530000000000ull; // Z  W  S
        constexpr auto TESignature = 0x565A000000000000ull; // V  Z
        constexpr auto VMDKSignature = 0x4B444D0000000000ull; // K  D  M  V
        constexpr auto VDISignature = 0x3C3C3C2000000000ull; // Alternatively 0x7F10DABE at offset 0x40)
        constexpr auto VhdSignature = 0x636F6E6563746978ull; // c  o  n  e  c  t  i  x
        constexpr auto XarSignature = 0x78617221001C0000ull; // x  a  r  !  00 1C
        constexpr auto ZSignature1 = 0x1F9D000000000000ull; // 1F 9D
        constexpr auto ZSignature2 = 0x1FA0000000000000ull; // 1F A0

        switch ( signature ) {
            case RarSignature:
                *format = &BitFormat::Rar;
                return true;
            case Rar5Signature:
                *format = &BitFormat::Rar5;
                return true;
            case SevenZipSignature:
                *format = &BitFormat::SevenZip;
                return true;
            case BZip2Signature:
                *format = &BitFormat::BZip2;
                return true;
            case GZipSignature:
                *format = &BitFormat::GZip;
                return true;
            case WimSignature:
                *format = &BitFormat::Wim;
                return true;
            case XzSignature:
                *format = &BitFormat::Xz;
                return true;
            case ZipSignature:
                *format = &BitFormat::Zip;
                return true;
            case APMSignature:
                *format = &BitFormat::APM;
                return true;
            case ArjSignature:
                *format = &BitFormat::Arj;
                return true;
            case CabSignature:
                *format = &BitFormat::Cab;
                return true;
            case ChmSignature:
                *format = &BitFormat::Chm;
                return true;
            case CompoundSignature:
                *format = &BitFormat::Compound;
                return true;
            case CpioSignature1:
            case CpioSignature2:
            case CpioSignature3:
                *format = &BitFormat::Cpio;
                return true;
            case DebSignature:
                *format = &BitFormat::Deb;
                return true;
                /* DMG signature detection is not this simple
                case 0x7801730D62626000:
                    *format = &BitFormat::Dmg;
                    return true;
                */
            case ElfSignature:
                *format = &BitFormat::Elf;
                return true;
            case PeSignature:
                *format = &BitFormat::Pe;
                return true;
            case FlvSignature:
                *format = &BitFormat::Flv;
                return true;
            case LzmaSignature:
                *format = &BitFormat::Lzma;
                return true;
            case Lzma86Signature:
                *format = &BitFormat::Lzma86;
                return true;
            case MachoSignature1:
            case MachoSignature2:
            case MachoSignature3:
            case MachoSignature4:
                *format = &BitFormat::Macho;
                return true;
            case MubSignature1:
            case MubSignature2:
                *format = &BitFormat::Mub;
                return true;
            case MslzSignature:
                *format = &BitFormat::Mslz;
                return true;
            case PpmdSignature:
                *format = &BitFormat::Ppmd;
                return true;
            case QCowSignature:
                *format = &BitFormat::QCow;
                return true;
            case RpmSignature:
                *format = &BitFormat::Rpm;
                return true;
            case SquashFSSignature1:
            case SquashFSSignature2:
            case SquashFSSignature3:
            case SquashFSSignature4:
                *format = &BitFormat::SquashFS;
                return true;
            case SwfSignature:
                *format = &BitFormat::Swf;
                return true;
            case SwfcSignature1:
            case SwfcSignature2:
                *format = &BitFormat::Swfc;
                return true;
            case TESignature:
                *format = &BitFormat::TE;
                return true;
            case VMDKSignature: // K  D  M  V
                *format = &BitFormat::VMDK;
                return true;
            case VDISignature: // Alternatively 0x7F10DABE at offset 0x40)
                *format = &BitFormat::VDI;
                return true;
            case VhdSignature: // c  o  n  e  c  t  i  x
                *format = &BitFormat::Vhd;
                return true;
            case XarSignature: // x  a  r  !  00 1C
                *format = &BitFormat::Xar;
                return true;
            case ZSignature1: // 1F 9D
            case ZSignature2: // 1F A0
                *format = &BitFormat::Z;
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
        { 0x2D6C680000000000, 0x02,  3, BitFormat::Lzh },    // -  l  h
        { 0x4E54465320202020, 0x03,  8, BitFormat::Ntfs },   // N  T  F  S  20 20 20 20
        { 0x4E756C6C736F6674, 0x08,  8, BitFormat::Nsis },   // N  u  l  l  s  o  f  t
        { 0x436F6D7072657373, 0x10,  8, BitFormat::CramFS }, // C  o  m  p  r  e  s  s
        { 0x7F10DABE00000000, 0x40,  4, BitFormat::VDI },    // 7F 10 DA BE
        { 0x7573746172000000, 0x101, 5, BitFormat::Tar },    // u  s  t  a  r
        // Note: since GPT files contain also the FAT signature, GPT must be checked before!
        { 0x4546492050415254, 0x200, 8, BitFormat::GPT },    // E  F  I  20 P  A  R  T
        { 0x55AA000000000000, 0x1FE, 2, BitFormat::Fat },    // U  AA
        { 0x4244000000000000, 0x400, 2, BitFormat::Hfs },    // B  D
        { 0x482B000400000000, 0x400, 4, BitFormat::Hfs },    // H  +  00 04
        { 0x4858000500000000, 0x400, 4, BitFormat::Hfs },    // H  X  00 05
        { 0x53EF000000000000, 0x438, 2, BitFormat::Ext }     // S  EF
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
                    return BitFormat::Udf;
                }
            }
            stream->Seek( 0, 0, nullptr );
            return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
        }

        stream->Seek( 0, 0, nullptr );
        throw BitException( "Cannot detect the format of the file" );
    }

#ifdef _WIN32
#define is_digit(ch) std::iswdigit(ch) != 0
#define to_lower std::towlower
#else
#define is_digit(ch) std::isdigit(ch) != 0
#define to_lower ::tolower  //Note: using std::tolower would be ambiguous (multiple definitions of std::tolower)
#endif

    const BitInFormat& detectFormatFromExt( const tstring& in_file ) {
        tstring ext = filesystem::fsutil::extension( in_file );
        if ( ext.empty() ) {
            throw BitException( "Cannot detect the archive format from the extension" );
        }
        std::transform( ext.cbegin(), ext.cend(), ext.begin(), to_lower );

        // Detecting archives with common file extensions
        const BitInFormat* format = nullptr;
        if ( findFormatByExtension( ext, &format ) ) { //extension found in map
            return *format;
        }

        // Detecting multi-volume archives extension
        if ( ( ext[ 0 ] == TSTRING('r') || ext[ 0 ] == TSTRING('z') ) &&
             ( ext.size() == 3 && is_digit( ext[ 1 ] ) && is_digit( ext[ 2 ] ) ) ) {
            // Extension follows the format zXX or rXX, where X is a number in range [0-9]
            return ext[ 0 ] == TSTRING('r') ? BitFormat::Rar : BitFormat::Zip;
        }

        // Note: iso, img and ima extensions can be associated with different formats -> detect by signature

        // The extension did not match any known format extension, delegating the decision to the client
        return BitFormat::Auto;
    }
}

#endif