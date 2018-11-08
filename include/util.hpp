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

#ifndef UTIL_HPP
#define UTIL_HPP

#include "7zip/Archive/IArchive.h"
#include "7zip/Common/FileStreams.h"

#include "../include/bit7zlibrary.hpp"
#include "../include/bitcompressionlevel.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    namespace util {
        CMyComPtr< IOutArchive > initOutArchive( const Bit7zLibrary& lib, const BitInOutFormat& format,
                const BitCompressionLevel compressionLevel,
                const bool cryptHeaders, const bool solidMode );

        CMyComPtr< IInArchive > openArchive( const Bit7zLibrary& lib, const BitInFormat& format,
                                             const wstring& in_file, const BitArchiveOpener& opener );

        HRESULT IsArchiveItemProp( IInArchive* archive, UInt32 index, PROPID propID, bool& result );

        HRESULT IsArchiveItemFolder( IInArchive* archive, UInt32 index, bool& result );
    }
}

#endif // UTIL_HPP
