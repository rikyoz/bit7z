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

#include "../include/util.hpp"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <cwctype>
#include <iomanip>

#include "../include/fsutil.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"

using std::vector;
using std::unordered_map;
using namespace NWindows;

namespace bit7z {
    namespace util {
        CMyComPtr< IOutArchive > initOutArchive( const BitArchiveCreator& creator ) {
            CMyComPtr< IOutArchive > out_archive;
            const BitInOutFormat& format = creator.compressionFormat();
            const GUID format_GUID = format.guid();
            creator.library().createArchiveObject( &format_GUID, &::IID_IOutArchive, reinterpret_cast< void** >( &out_archive ) );

            vector< const wchar_t* > names;
            vector< BitPropVariant > values;
            if ( creator.cryptHeaders() && format.hasFeature( HEADER_ENCRYPTION ) ) {
                names.push_back( L"he" );
                values.emplace_back( true );
            }
            if ( format.hasFeature( COMPRESSION_LEVEL ) ) {
                names.push_back( L"x" );
                values.emplace_back( static_cast< uint32_t >( creator.compressionLevel() ) );
            }
            if ( format.hasFeature( SOLID_ARCHIVE ) ) {
                names.push_back( L"s" );
                values.emplace_back( creator.solidMode() );
            }

            if ( !names.empty() ) {
                CMyComPtr< ISetProperties > set_properties;
                if ( out_archive->QueryInterface( ::IID_ISetProperties,
                                                  reinterpret_cast< void** >( &set_properties ) ) != S_OK ) {
                    throw BitException( "ISetProperties unsupported" );
                }
                if ( set_properties->SetProperties( names.data(), values.data(),
                                                    static_cast< uint32_t >( names.size() ) ) != S_OK ) {
                    throw BitException( "Cannot set properties of the archive" );
                }
            }
            return out_archive;
        }

        // NOTE: this function is not a method of BitExtractor because it would dirty the header with extra dependencies
        CMyComPtr< IInArchive > openArchive( const BitArchiveHandler& handler, const BitInFormat& format, const wstring& in_file ) {
            // Opening file input stream
            auto* file_stream_spec = new CInFileStream;
            CMyComPtr< IInStream > file_stream = file_stream_spec;
            if ( !file_stream_spec->Open( in_file.c_str() ) ) {
                throw BitException( L"Cannot open archive file '" + in_file + L"'" );
            }
            // Creating open callback for the file
            auto* open_callback_spec = new OpenCallback( handler, in_file );
            CMyComPtr< IArchiveOpenCallback > open_callback( open_callback_spec );

            // Creating 7z archive object for the input file format
            CMyComPtr< IInArchive > in_archive;
            bool signature_detected = false;
            GUID format_GUID;
            if ( format == BitFormat::Auto ) { // Detecting format of the input file
                const BitInFormat& detected_format = detectFormatByExt( in_file );
                if ( detected_format == BitFormat::Auto ) { // Unknown extension, trying using the file signature
                    format_GUID = detectFormatBySig( in_file ).guid();
                    signature_detected = true;
                } else { // Format detected from file extension
                    format_GUID = detected_format.guid();
                }
            } else { // Format directly given by the user
                format_GUID = format.guid();
            }
            handler.library().createArchiveObject( &format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &in_archive ) );

            // Trying to open the file with the detected format
            HRESULT res = in_archive->Open( file_stream, nullptr, open_callback );
            if ( res == S_OK ) {
                return in_archive;
            }
            if ( format == BitFormat::Auto && !signature_detected ) {
                /* User wanted auto detection of format, an extension was detected but opening failed, so we try a more
                 * precise detection by checking the signature.
                 * NOTE: If user specified explicitly a format (i.e. not BitFormat::Auto), this check is not performed
                 *       and an exception is thrown!
                 * NOTE 2: If signature detection was already performed (signature_detected == false), it detected a
                 *         a wrong format, no further check can be done and an exception must be thrown! */
                format_GUID = detectFormatBySig( in_file ).guid();
                handler.library().createArchiveObject( &format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &in_archive ) );
                res = in_archive->Open( file_stream, nullptr, open_callback );
                if ( res == S_OK ) {
                    return in_archive;
                }
            }
            throw BitException( L"Cannot open archive '" + in_file  + L"'" );
        }

        HRESULT IsArchiveItemProp( IInArchive* archive, UInt32 index, PROPID propID, bool& result ) {
            BitPropVariant prop;
            RINOK( archive->GetProperty( index, propID, &prop ) );

            if ( prop.isEmpty() ) {
                result = false;
            } else if ( prop.type() == BitPropVariantType::Bool ) {
                result = prop.getBool();
            } else {
                return E_FAIL;
            }

            return S_OK;
        }

        HRESULT IsArchiveItemFolder( IInArchive* archive, UInt32 index, bool& result ) {
            return IsArchiveItemProp( archive, index, kpidIsDir, result );
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
            { 0x7801730D62626000, BitFormat::Dmg },
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

        const BitInFormat& detectFormatBySig( const wstring& in_file ) {
            constexpr auto SIGNATURE_SIZE = 8u;

            std::ifstream stream( in_file, std::ifstream::binary );
            if ( !stream ) {
                throw BitException( "Cannot open file to read its signature" );
            }
            stream.exceptions( std::ifstream::failbit | std::ifstream::badbit );

            try {
                uint64_t file_signature = 0;
                auto file_signature_array = reinterpret_cast< char* >( &file_signature );
                stream.read( file_signature_array, SIGNATURE_SIZE );
                file_signature = _byteswap_uint64( file_signature );
                for ( auto i = 0u; i < SIGNATURE_SIZE - 1; ++i ) {
                    auto it = common_signatures.find( file_signature );
                    if ( it != common_signatures.end() ) {
                        //std::wcout << L"Detected format: " << std::hex << it->second.value() << std::endl;
                        return it->second;
                    }
                    file_signature_array[ i ] = 0x00;
                }

                for ( auto& sig : common_signatures_with_offset ) {
                    file_signature = 0; //reset signature
                    stream.seekg( sig.offset );
                    stream.read( file_signature_array, sig.size );
                    file_signature = _byteswap_uint64( file_signature );
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
                stream.seekg( ISO_SIGNATURE_OFFSET ); //Checking only at 0x8001, other offset to be added in future!
                file_signature = 0; //reset signature
                stream.read( file_signature_array, ISO_SIGNATURE_SIZE );
                file_signature = _byteswap_uint64( file_signature );
                if ( file_signature == ISO_SIGNATURE ) {
                    //The file is ISO, checking if it is also UDF!
                    for ( auto descriptor_index = 1; descriptor_index < MAX_VOLUME_DESCRIPTORS; ++descriptor_index ) {
                        stream.seekg( ISO_SIGNATURE_OFFSET + descriptor_index * ISO_VOLUME_DESCRIPTOR_SIZE );
                        file_signature = 0; //reset signature
                        stream.read( file_signature_array, UDF_SIGNATURE_SIZE );
                        file_signature = _byteswap_uint64( file_signature );
                        if ( file_signature == UDF_SIGNATURE ) {
                            std::wcout << "UDF!" << std::endl;
                            return BitFormat::Udf;
                        }
                    }
                    std::wcout << "ISO!" << std::endl;
                    return BitFormat::Iso; //No UDF volume signature found, i.e. simple ISO!
                }
            } catch ( const std::ios_base::failure& ex ) {
                throw BitException( ex.what() );
            }

            throw BitException( "Cannot detect the format of the file" );
        }

        const BitInFormat& detectFormatByExt( const wstring& in_file ) {
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
            //return detect_format_by_sig( in_file );

            /* TODO: [ ] Find a way to reduce size of extensions map */
        }
    }
}
