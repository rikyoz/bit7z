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
#include <regex>
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
            CMyComPtr< IInArchive > in_archive;
            const GUID format_GUID = ( format == BitFormat::Auto ) ? detect_format( in_file ).guid() : format.guid();
            handler.library().createArchiveObject( &format_GUID, &::IID_IInArchive, reinterpret_cast< void** >( &in_archive ) );

            auto* file_stream_spec = new CInFileStream;
            CMyComPtr< IInStream > file_stream = file_stream_spec;
            if ( !file_stream_spec->Open( in_file.c_str() ) ) {
                throw BitException( L"Cannot open archive file '" + in_file + L"'" );
            }

            auto* open_callback_spec = new OpenCallback( handler, in_file );

            CMyComPtr< IArchiveOpenCallback > open_callback( open_callback_spec );
            HRESULT res = in_archive->Open( file_stream, nullptr, open_callback );
            if ( res != S_OK ) {
                throw BitException( L"Cannot open archive '" + in_file  + L"'" );
            }
            return in_archive;
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
            { L"esd",      BitFormat::Wim },
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
            { L"z",        BitFormat::Z },
            { L"taz",      BitFormat::Z }
        };

        /*void print_signature_vec( const vector< byte_t >& signature ) {
            std::wcout << std::hex;
            for ( auto byte : signature ) {
                std::wcout << L"0x" << std::setw(2) << std::setfill(L'0') << byte << L" ";
            }
            std::wcout << std::dec << std::endl;
        }

        vector< byte_t > read_signature_vec( const wstring& in_file, size_t size, std::streamoff offset = 0 ) {
            vector< byte_t > signature( size );
            std::ifstream stream( in_file, std::ifstream::binary );
            if ( !stream ) {
                throw BitException( "Cannot read file signature" );
            }
            if ( offset > 0 ) {
                stream.seekg( offset );
            }
            stream.read( reinterpret_cast< char* >( signature.data() ), static_cast< std::streamsize >( size ) );
            return signature;
        }*/

        uint64_t read_signature( const wstring& in_file, size_t size, std::streamoff offset = 0 ) {
            if ( size > 8 ) { return 0; }

            uint64_t signature = 0;
            std::ifstream stream( in_file, std::ifstream::binary );
            if ( !stream ) {
                throw BitException( "Cannot read file signature" );
            }
            if ( offset > 0 ) {
                stream.seekg( offset );
            }
            stream.read( reinterpret_cast< char* >( &signature ), static_cast< std::streamsize >( size ) );
            return signature;
        }

        /* NOTE: File signatures are specified in big endian order and swapped to little endian
                 via MSVC specific function _byteswap_uint64.
                 Moreover, for signatures with less than 8 bytes (size of uint64), remaining bytes are set to 0.
                 When calling read_signature, remember to request the correct number of bytes (size) and offset */
        const unordered_map< uint64_t, const BitInFormat& > common_signatures = {
            { _byteswap_uint64( 0x526172211A070000 ), BitFormat::Rar },
            { _byteswap_uint64( 0x526172211A070100 ), BitFormat::Rar5 },
            { _byteswap_uint64( 0x4657530000000000 ), BitFormat::Swf },
            { _byteswap_uint64( 0x4357530000000000 ), BitFormat::Swfc },
            { _byteswap_uint64( 0x377ABCAF271C0000 ), BitFormat::SevenZip },
            { _byteswap_uint64( 0x425A680000000000 ), BitFormat::BZip2 },
            { _byteswap_uint64( 0x1F8B080000000000 ), BitFormat::GZip },
            { _byteswap_uint64( 0x7573746172000000 ), BitFormat::Tar }, //OFFSET??
            { _byteswap_uint64( 0x4D5357494D000000 ), BitFormat::Wim },
            { _byteswap_uint64( 0xFD377A585A000000 ), BitFormat::Xz },
            { _byteswap_uint64( 0x504B000000000000 ), BitFormat::Zip },
            { _byteswap_uint64( 0x60EA000000000000 ), BitFormat::Arj },
            { _byteswap_uint64( 0x4D53434600000000 ), BitFormat::Cab },
            { _byteswap_uint64( 0x4954534600000000 ), BitFormat::Chm },
            { _byteswap_uint64( 0xD0CF11E0A1B11AE1 ), BitFormat::Compound },
            { _byteswap_uint64( 0xC771000000000000 ), BitFormat::Cpio },
            { _byteswap_uint64( 0x71C7000000000000 ), BitFormat::Cpio },
            { _byteswap_uint64( 0x3037303730000000 ), BitFormat::Cpio },
            { _byteswap_uint64( 0x213C617263683E00 ), BitFormat::Deb },
            { _byteswap_uint64( 0x7801730D62626000 ), BitFormat::Dmg },
            { _byteswap_uint64( 0x7F454C4600000000 ), BitFormat::Elf },
            { _byteswap_uint64( 0x464C560100000000 ), BitFormat::Flv },
            { _byteswap_uint64( 0x2D6C680000000000 ), BitFormat::Lzh }, //OFFSET??
            { _byteswap_uint64( 0x5D00000000000000 ), BitFormat::Lzma },
            { _byteswap_uint64( 0x015D000000000000 ), BitFormat::Lzma86 },
            { _byteswap_uint64( 0xCFFAEDFE00000000 ), BitFormat::Macho },
            { _byteswap_uint64( 0xCAFEBABE00000000 ), BitFormat::Macho },
            { _byteswap_uint64( 0x514649FB00000000 ), BitFormat::QCow },
            { _byteswap_uint64( 0xEDABEEDB00000000 ), BitFormat::Rpm },
            { _byteswap_uint64( 0x7371736800000000 ), BitFormat::SquashFS },
            { _byteswap_uint64( 0x6873717300000000 ), BitFormat::SquashFS },
            { _byteswap_uint64( 0x4B444D0000000000 ), BitFormat::VMDK },
            { _byteswap_uint64( 0x3C3C3C2000000000 ), BitFormat::VDI }, //Alternatively 0x7F10DABE at offset 0x40
            { _byteswap_uint64( 0x636F6E6563746978 ), BitFormat::Vhd },
            { _byteswap_uint64( 0x7861722100000000 ), BitFormat::Xar },
            { _byteswap_uint64( 0x1F9D000000000000 ), BitFormat::Z },
            { _byteswap_uint64( 0x1FA0000000000000 ), BitFormat::Z }
        };

        //const vector< tuple< const BitFormat&, uint64_t, std::streamoff > > = {};

        const BitInFormat& detect_format( const wstring& in_file ) {
            wstring ext = filesystem::fsutil::extension( in_file );
            if ( ext.empty() ) {
                throw BitException( "Cannot detect the archive format from the extension" );
            }

            std::transform( ext.cbegin(), ext.cend(), ext.begin(), std::towlower );
            std::wcout << ext << std::endl;

            auto it = common_extensions.find( ext ); //detecting archives with common file extensions
            if ( it != common_extensions.end() ) { //extension found in map
                return it->second;
            }

            /* TODO: [ ] Find a way to reduce size of extensions map
             *       [ ] Check endianness problem
             *       [ ] Check signatures map keys size problem
             *       [ ] Check offset problem
             *       [ ] Check multi-volume detection problem
             */

            if ( ext == L"rar" ) {
                //detect rar version
                /*const vector< byte_t > rar4_sig = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00 };
                const vector< byte_t > rar5_sig = { 0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01 };
                auto file_signature = read_signature( in_file, 7 ); //reading first 8 bytes
                print_signature( file_signature );
                std::wcout << std::hex << read_signature_2( in_file, 7 ) << std::dec << std::endl;
                //print_signature( rar5_sig );
                //print_signature( rar4_sig );
                if ( file_signature == rar4_sig ) {
                    return BitFormat::Rar;
                } else if ( file_signature == rar5_sig ) {
                    return BitFormat::Rar5;
                }*/
                //auto file_signature_vec = read_signature_vec( in_file, 7 ); //reading first 8 bytes
                //std::wcout << L"File signature vector: ";
                //print_signature_vec( file_signature_vec );
                /*for ( auto& sig : common_signatures ) {
                    std::wcout << std::hex << sig.first << std::dec << std::endl;
                }*/
                auto file_signature = read_signature( in_file, 7 ); //reading first 8 bytes
                //std::wcout << L"File signature: " << std::hex << file_signature << std::dec << std::endl;
                //std::wcout << L"File signature (little endian): " << std::hex << _byteswap_uint64( file_signature ) << std::dec << std::endl;
                auto it = common_signatures.find( file_signature );
                if ( it != common_signatures.end() ) {
                    //std::wcout << L"Detected format: " << std::hex << it->second.value() << std::endl;
                    return it->second;
                }
            }

            //TODO: 7z SFX detection
            /*if ( ext == L"exe" ) { //check properties to see if 7z SFX

            }*/

            if ( ext == L"iso" || ext == L"img" || ext == L"ima" ) {

            }

            /*if ( ext == L"swf" ) {
                const vector< byte_t > swf_sig = { 0x46, 0x57, 0x53 };
                const vector< byte_t > swfc_sig = { 0x43, 0x57, 0x53 };
                auto file_signature = read_signature( in_file, 3 ); //reading first 3 bytes
                if ( file_signature == swf_sig ) {
                    return BitFormat::Swf;
                } else if ( file_signature == swfc_sig ) {
                    return BitFormat::Swfc;
                }
            }*/

            std::wregex split_regex( L"([0-9]{3,})" );
            if ( std::regex_match( ext, split_regex ) ) { //the file is a splitted archive
                return BitFormat::Split;
            }

            //ELF format?

            /*std::wregex multivolume_regex( L"(r|z)([0-9]+)" );
            bool is_multivolume = std::regex_match( ext, multivolume_regex );
            if ( is_multivolume ) {
                if ( ext[0] == L'r' ) {
                    //detect RAR version
                } else {
                    return BitFormat::Zip;
                }
            }*/

            /*if ( ext[0] == L'z' || ext[0] == L'r' ) {
                bool is_multi_volume = false;
            }*/

            return BitFormat::SevenZip;
        }
    }
}
