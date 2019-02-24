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

#include "../include/fsutil.hpp"
#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/opencallback.hpp"

#include "7zip/Common/StreamObjects.h"

namespace bit7z {
    namespace util {
        using namespace filesystem;

        CMyComPtr< IOutArchive > initOutArchive( const BitArchiveCreator& creator ) {
            const BitInOutFormat& format = creator.compressionFormat();
            const GUID format_GUID = format.guid();

            CMyComPtr< IOutArchive > out_archive;
            creator.library().initOutputArchive( &format_GUID, out_archive );

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

        CMyComPtr< IInArchive > openArchive( const BitArchiveHandler& handler, const BitInFormat& format,
                                             const wstring& name, IInStream* in_stream ) {
            bool detected_by_signature = false;
            GUID format_GUID;
            if ( format == BitFormat::Auto ) { // Detecting format of the input file
                format_GUID = fsutil::detect_format_by_sig( in_stream ).guid();
                detected_by_signature = true;
            } else { // Format directly given by the user
                format_GUID = format.guid();
            }
            CMyComPtr< IInArchive > in_archive;
            handler.library().initInputArchive( &format_GUID, in_archive );

            // Creating open callback for the file
            auto* open_callback_spec = new OpenCallback( handler, name );
            CMyComPtr< IArchiveOpenCallback > open_callback( open_callback_spec );

            // Trying to open the file with the detected format
            HRESULT res = in_archive->Open( in_stream, nullptr, open_callback );
            if ( res == S_OK ) {
                return in_archive;
            }
            if ( format == BitFormat::Auto && !detected_by_signature ) {
                /* User wanted auto detection of format, an extension was detected but opening failed, so we try a more
                 * precise detection by checking the signature.
                 * NOTE: If user specified explicitly a format (i.e. not BitFormat::Auto), this check is not performed
                 *       and an exception is thrown!
                 * NOTE 2: If signature detection was already performed (signature_detected == false), it detected a
                 *         a wrong format, no further check can be done and an exception must be thrown! */
                format_GUID = fsutil::detect_format_by_sig( in_stream ).guid();
                handler.library().initInputArchive( &format_GUID, in_archive );
                res = in_archive->Open( in_stream, nullptr, open_callback );
                if ( res == S_OK ) {
                    return in_archive;
                }
            }
            throw BitException( L"Cannot open archive '" + name + L"'" );
        }

        CMyComPtr< IInArchive > openArchive( const BitArchiveHandler& handler, const BitInFormat& format,
                                             const wstring& in_file ) {
            auto* file_stream_spec = new CInFileStream;
            CMyComPtr< IInStream > file_stream = file_stream_spec;
            if ( !file_stream_spec->Open( in_file.c_str() ) ) {
                throw BitException( L"Cannot open archive file '" + in_file + L"'" );
            }
            const BitInFormat& detectedFormat = ( format == BitFormat::Auto ? fsutil::detect_format_by_ext( in_file ) : format );
            return openArchive( handler, detectedFormat, in_file, file_stream );
        }

        CMyComPtr< IInArchive > openArchive( const BitArchiveHandler& handler, const BitInFormat& format,
                                             const vector<byte_t>& in_buffer ) {
            auto* buf_stream_spec = new CBufInStream;
            CMyComPtr< IInStream > buf_stream = buf_stream_spec;
            buf_stream_spec->Init( in_buffer.data(), in_buffer.size() );
            return openArchive( handler, format, L".", buf_stream );
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
