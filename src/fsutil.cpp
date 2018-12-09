// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include "../include/fsutil.hpp"

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <cwctype>
#include <iomanip>

#include "../include/bitformat.hpp"
#include "../include/bitexception.hpp"

#include <Windows.h>

using namespace std;
using namespace bit7z;
using namespace bit7z::filesystem;

bool fsutil::is_directory( const wstring& path ) {
    return 0 != ( GetFileAttributes( path.c_str() ) & FILE_ATTRIBUTE_DIRECTORY );
}

bool fsutil::path_exists( const wstring& path ) {
    return GetFileAttributes( path.c_str() ) != INVALID_FILE_ATTRIBUTES;
}

bool fsutil::rename_file( const wstring& old_name, const wstring& new_name ) {
    //NOTE: It overwrites the destination file!
    return MoveFileEx( old_name.c_str(), new_name.c_str(), MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING ) != FALSE; //WinAPI BOOL
}

void fsutil::normalize_path( wstring& path ) { //this assumes that the passed path is not a file path!
    if ( !path.empty() && path.back() != L'\\' && path.back() != L'/' ) {
        path.append( L"\\" );
    }
}

wstring fsutil::dirname( const wstring& path ) {
    //the directory containing the path (hence, up directory if the path is a folder)
    size_t pos = path.find_last_of( L"\\/" );
    return ( pos != wstring::npos ) ? path.substr( 0, pos ) : L"";
}

wstring fsutil::filename( const wstring& path, bool ext ) {
    size_t start = path.find_last_of( L"/\\" ) + 1;
    size_t end   = ext ? path.size() : path.find_last_of( L'.' );
    return path.substr( start, end - start ); //RVO :)
}

wstring fsutil::extension( const wstring& path ) {
    wstring name = filename( path, true );
    size_t last_dot = name.find_last_of( L'.' );
    return last_dot != wstring::npos ? name.substr( last_dot + 1 ) : L"";
}

// TODO: check if find_first_of is necessary or use front()
bool fsutil::is_relative_path( const wstring& path ) {
    //return PathIsRelativeW( path.c_str() ); //WinAPI version (requires Shlwapi lib!)
    return path.empty() || ( path.find_first_of( L"/\\" ) != 0 && !( path.length() >= 2 && path[1] == L':' ) );
}

// Modified version of code found here: https://stackoverflow.com/a/3300547
bool w_match( const wchar_t* needle, const wchar_t* haystack, size_t max ) {
    for ( ; *needle != L'\0'; ++needle ) {
        switch ( *needle ) {
            case L'?':
                if ( *haystack == L'\0' ) {
                    return false;
                }
                ++haystack;
                break;
            case L'*': {
                if ( needle[1] == L'\0' ) {
                    return true;
                }
                for ( size_t i = 0; i < max; i++ ) {
                    if ( w_match( needle + 1, haystack + i, max - i ) ) {
                        return true;
                    }
                }
                return false;
            }
            default:
                if ( *haystack != *needle ) {
                    return false;
                }
                ++haystack;
        }
    }
    return *haystack == L'\0';
}

bool fsutil::wildcard_match( const wstring& pattern, const wstring& str ) {
    return w_match( pattern.empty() ? L"*" : pattern.c_str(), str.c_str(), str.size() );
}

const unordered_map< wstring, const BitInFormat& > common_extensions {
    { L"7z",       BitFormat::SevenZip },
    { L"bzip2",    BitFormat::BZip2 },
    { L"bz2",      BitFormat::BZip2 },
    { L"tbz2",     BitFormat::BZip2 },
    { L"tbz",      BitFormat::BZip2 },
    { L"gz",       BitFormat::GZip },
    { L"gzip",     BitFormat::GZip },
    { L"tgz",      BitFormat::GZip },
    { L"tar",      BitFormat::Tar },
    { L"wim",      BitFormat::Wim },
    { L"swm",      BitFormat::Wim },
    { L"xz",       BitFormat::Xz },
    { L"txz",      BitFormat::Xz },
    { L"zip",      BitFormat::Zip },
    { L"zipx",     BitFormat::Zip },
    { L"jar",      BitFormat::Zip },
    { L"xpi",      BitFormat::Zip },
    { L"odt",      BitFormat::Zip },
    { L"ods",      BitFormat::Zip },
    { L"odp",      BitFormat::Zip },
    { L"docx",     BitFormat::Zip },
    { L"xlsx",     BitFormat::Zip },
    { L"pptx",     BitFormat::Zip },
    { L"epub",     BitFormat::Zip },
    { L"001",      BitFormat::Split },
    { L"ar",       BitFormat::Deb },
    { L"apm",      BitFormat::APM },
    { L"arj",      BitFormat::Arj },
    { L"cab",      BitFormat::Cab },
    { L"chm",      BitFormat::Chm },
    { L"chi",      BitFormat::Chm },
    { L"msi",      BitFormat::Compound },
    { L"doc",      BitFormat::Compound },
    { L"xls",      BitFormat::Compound },
    { L"ppt",      BitFormat::Compound },
    { L"msg",      BitFormat::Compound },
    { L"cpio",     BitFormat::Cpio },
    { L"cramfs",   BitFormat::CramFS },
    { L"deb",      BitFormat::Deb },
    { L"dmg",      BitFormat::Dmg },
    { L"dll",      BitFormat::Pe },
    { L"dylib",    BitFormat::Macho },
    { L"exe",      BitFormat::Pe }, //note: we do not distinguish 7z SFX exe at the moment!
    { L"ext",      BitFormat::Ext },
    { L"ext2",     BitFormat::Ext },
    { L"ext3",     BitFormat::Ext },
    { L"ext4",     BitFormat::Ext },
    { L"fat",      BitFormat::Fat },
    { L"flv",      BitFormat::Flv },
    { L"hfs",      BitFormat::Hfs },
    { L"hfsx",     BitFormat::Hfs },
    { L"hxs",      BitFormat::Hxs },
    { L"ihex",     BitFormat::IHex },
    { L"lzh",      BitFormat::Lzh },
    { L"lha",      BitFormat::Lzh },
    { L"lzma",     BitFormat::Lzma },
    { L"lzma86",   BitFormat::Lzma86 },
    { L"mbr",      BitFormat::Mbr },
    { L"mslz",     BitFormat::Mslz },
    { L"mub",      BitFormat::Mub },
    { L"nsis",     BitFormat::Nsis },
    { L"ntfs",     BitFormat::Ntfs },
    { L"ppmd",     BitFormat::Ppmd },
    { L"qcow",     BitFormat::QCow },
    { L"qcow2",    BitFormat::QCow },
    { L"qcow2c",   BitFormat::QCow },
    { L"rpm",      BitFormat::Rpm },
    { L"scap",     BitFormat::UEFIc },
    { L"squashfs", BitFormat::SquashFS },
    { L"udf",      BitFormat::Udf },
    { L"uefif",    BitFormat::UEFIs },
    { L"vmdk",     BitFormat::VMDK },
    { L"vdi",      BitFormat::VDI },
    { L"vhd",      BitFormat::Vhd },
    { L"xar",      BitFormat::Xar },
    { L"pkg",      BitFormat::Xar },
    { L"z",        BitFormat::Z },
    { L"taz",      BitFormat::Z }
};

/* NOTE: For signatures with less than 8 bytes (size of uint64_t), remaining bytes are set to 0 */
const unordered_map< uint64_t, const BitInFormat& > common_signatures = {
    { 0x526172211A070000, BitFormat::Rar },
    { 0x526172211A070100, BitFormat::Rar5 },
    { 0x4657530000000000, BitFormat::Swf },
    { 0x4357530000000000, BitFormat::Swfc },
    { 0x377ABCAF271C0000, BitFormat::SevenZip },
    { 0x425A680000000000, BitFormat::BZip2 },
    { 0x1F8B080000000000, BitFormat::GZip },
    { 0x4D5357494D000000, BitFormat::Wim },
    { 0xFD377A585A000000, BitFormat::Xz },
    { 0x504B000000000000, BitFormat::Zip },
    { 0x4552000000000000, BitFormat::APM },
    { 0x60EA000000000000, BitFormat::Arj },
    { 0x4D53434600000000, BitFormat::Cab },
    { 0x4954534600000000, BitFormat::Chm },
    { 0xD0CF11E0A1B11AE1, BitFormat::Compound },
    { 0xC771000000000000, BitFormat::Cpio },
    { 0x71C7000000000000, BitFormat::Cpio },
    { 0x3037303730000000, BitFormat::Cpio },
    { 0x213C617263683E00, BitFormat::Deb },
    //{ 0x7801730D62626000, BitFormat::Dmg }, /* DMG signature detection is not this simple */
    { 0x7F454C4600000000, BitFormat::Elf },
    { 0x4D5A000000000000, BitFormat::Pe },
    { 0x464C560100000000, BitFormat::Flv },
    { 0x5D00000000000000, BitFormat::Lzma },
    { 0x015D000000000000, BitFormat::Lzma86 },
    { 0xCFFAEDFE00000000, BitFormat::Macho },
    { 0xCAFEBABE00000000, BitFormat::Macho },
    { 0x514649FB00000000, BitFormat::QCow },
    { 0xEDABEEDB00000000, BitFormat::Rpm },
    { 0x7371736800000000, BitFormat::SquashFS },
    { 0x6873717300000000, BitFormat::SquashFS },
    { 0x4B444D0000000000, BitFormat::VMDK },
    { 0x3C3C3C2000000000, BitFormat::VDI }, //Alternatively 0x7F10DABE at offset 0x40
    { 0x636F6E6563746978, BitFormat::Vhd },
    { 0x7861722100000000, BitFormat::Xar },
    { 0x1F9D000000000000, BitFormat::Z },
    { 0x1FA0000000000000, BitFormat::Z }
};

struct OffsetSignature {
    std::streamoff offset;
    std::streamsize size;
    uint64_t signature;
    const BitInFormat& format;
};

const vector< OffsetSignature > common_signatures_with_offset = {
    { 0x02,   3, 0x2D6C680000000000, BitFormat::Lzh },
    { 0x40,   4, 0x7F10DABE00000000, BitFormat::VDI },
    { 0x101,  5, 0x7573746172000000, BitFormat::Tar }/*,
    { 0x8001, 5, 0x4344303031, BitFormat::Iso },
    { 0x8801, 5, 0x4344303031, BitFormat::Iso },
    { 0x9001, 5, 0x4344303031, BitFormat::Iso }*/
};

uint64_t read_signature( std::ifstream& stream, std::streamsize size ) {
    uint64_t signature = 0;
    stream.read( reinterpret_cast< char* >( &signature ), size );
    return _byteswap_uint64( signature );
}

const BitInFormat& fsutil::detect_format( const std::wstring& in_file, bool& detected_by_signature ) {
    const BitInFormat& detected_format = detect_format_by_ext( in_file );
    if ( detected_format == BitFormat::Auto ) { // Unknown extension, trying using the file signature
        detected_by_signature = true;
        return detect_format_by_sig( in_file );
    } else { // Format detected from file extension
        return detected_format;
    }
}

const BitInFormat& fsutil::detect_format_by_sig( const wstring& in_file ) {
    constexpr auto SIGNATURE_SIZE = 8u;

    std::ifstream stream( in_file, std::ifstream::binary );
    if ( !stream ) {
        throw BitException( "Cannot open file to read its signature" );
    }
    stream.exceptions( std::ifstream::failbit | std::ifstream::badbit );

    try {
        uint64_t file_signature = read_signature( stream, SIGNATURE_SIZE );
        uint64_t signature_mask = 0xFFFFFFFFFFFFFFFFull;
        for ( auto i = 0u; i < SIGNATURE_SIZE - 1; ++i ) {
            auto it = common_signatures.find( file_signature );
            if ( it != common_signatures.end() ) {
                //std::wcout << L"Detected format: " << std::hex << it->second.value() << std::endl;
                return it->second;
            }
            signature_mask <<= 8ull;          // left shifting the mask of 1 byte, so that
            file_signature &= signature_mask; // the least significant i bytes are masked (set to 0)
        }

        for ( auto& sig : common_signatures_with_offset ) {
            stream.seekg( sig.offset );
            file_signature = read_signature( stream, sig.size );
            if ( file_signature == sig.signature ) {
                return sig.format;
            }
        }

        // Detecting ISO/UDF
        constexpr auto MAX_VOLUME_DESCRIPTORS     = 16;
        constexpr auto ISO_VOLUME_DESCRIPTOR_SIZE = 0x800; //2048

        constexpr auto ISO_SIGNATURE              = 0x4344303031000000; //CD001
        constexpr auto ISO_SIGNATURE_SIZE         = 5u;
        constexpr auto ISO_SIGNATURE_OFFSET       = 0x8001;

        constexpr auto UDF_SIGNATURE              = 0x4E53523000000000; //NSR0
        constexpr auto UDF_SIGNATURE_SIZE         = 4u;

        //Checking for ISO signature
        stream.seekg( ISO_SIGNATURE_OFFSET );
        file_signature = read_signature( stream, ISO_SIGNATURE_SIZE );
        if ( file_signature == ISO_SIGNATURE ) {
            //The file is ISO, checking if it is also UDF!
            for ( auto descriptor_index = 1; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
                stream.seekg( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE );
                file_signature = read_signature( stream, UDF_SIGNATURE_SIZE );
                if ( file_signature == UDF_SIGNATURE ) {
                    //std::wcout << "UDF!" << std::endl;
                    return BitFormat::Udf;
                }
            }
            //std::wcout << "ISO!" << std::endl;
            return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
        }
    } catch ( const std::ios_base::failure& ex ) {
        throw BitException( ex.what() );
    }

    throw BitException( "Cannot detect the format of the file" );
}

const BitInFormat& fsutil::detect_format_by_ext( const wstring& in_file ) {
    wstring ext = filesystem::fsutil::extension( in_file );
    if ( ext.empty() ) {
        throw BitException( "Cannot detect the archive format from the extension" );
    }

    std::transform( ext.cbegin(), ext.cend(), ext.begin(), std::towlower );
    //std::wcout << ext << std::endl;

    //detecting archives with common file extensions
    auto it = common_extensions.find( ext );
    if ( it != common_extensions.end() ) { //extension found in map
        return it->second;
    }

    //detecting multivolume archives extension
    if ( ( ext[ 0 ] == L'r' || ext[ 0 ] == L'z' ) &&
            ( ext.size() == 3 && iswdigit( ext[ 1 ] ) != 0 && iswdigit( ext[ 2 ] ) != 0 ) ) {
        //extension follows the format zXX or rXX, where X is a number in range [0-9]
        return ext[ 0 ] == L'r' ? BitFormat::Rar : BitFormat::Zip;
    }

    //TODO: 7z SFX detection
    /*if ( ext == L"exe" ) { //check properties to see if 7z SFX

            }*/

    //Note: iso, img and ima extensions can be associated with different formats -> detect by signature

    /* The extension did not match any known format extension, delegating the decision to the client */
    return BitFormat::Auto;

    /* TODO: [ ] Find a way to reduce size of extensions map */
}
