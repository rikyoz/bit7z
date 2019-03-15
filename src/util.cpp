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

#include "../include/bitexception.hpp"
#include "../include/bitpropvariant.hpp"

#include "Common/MyCom.h"

#include <vector>

namespace bit7z {
    namespace util {
        using std::vector;

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
    }
}
