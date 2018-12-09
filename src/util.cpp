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

#include "../include/fsutil.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"

using std::vector;
using namespace NWindows;

namespace bit7z {
    namespace util {
        using namespace filesystem;

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
                const BitInFormat& detected_format = fsutil::detect_format_by_ext( in_file );
                if ( detected_format == BitFormat::Auto ) { // Unknown extension, trying using the file signature
                    format_GUID = fsutil::detect_format_by_sig( in_file ).guid();
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
                format_GUID = fsutil::detect_format_by_sig( in_file ).guid();
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
    }
}
