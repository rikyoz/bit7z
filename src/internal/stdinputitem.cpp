// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#include "internal/stdinputitem.hpp"

#include "internal/cstdinstream.hpp"

#ifndef _WIN32
#include "internal/dateutil.hpp"
#endif

using bit7z::BitPropVariant;
using bit7z::StdInputItem;
using bit7z::tstring;
using std::istream;

StdInputItem::StdInputItem( istream& stream, const tstring& path ) : mStream{ stream }, mStreamPath{ path } {}

tstring StdInputItem::name() const {
    return mStreamPath.filename();
}

tstring StdInputItem::path() const {
    return mStreamPath;
}

fs::path StdInputItem::inArchivePath() const {
    return mStreamPath;
}

HRESULT StdInputItem::getStream( ISequentialInStream** inStream ) const {
    auto inStreamLoc = bit7z::make_com< CStdInStream, ISequentialInStream >( mStream );
    *inStream = inStreamLoc.Detach(); //Note: 7-zip will take care of freeing the memory!
    return S_OK;
}

bool StdInputItem::isDir() const noexcept {
    return false;
}

uint64_t StdInputItem::size() const {
    const auto original_pos = mStream.tellg();
    mStream.seekg( 0, std::ios::end ); // seeking to the end of the stream
    const auto result = static_cast< uint64_t >( mStream.tellg() - original_pos ); // size of the stream
    mStream.seekg( original_pos ); // seeking back to the original position in the stream
    return result;
}

FILETIME StdInputItem::creationTime() const noexcept { //-V524
    return lastWriteTime();
}

FILETIME StdInputItem::lastAccessTime() const noexcept { //-V524
    return lastWriteTime();
}

FILETIME StdInputItem::lastWriteTime() const noexcept {
#ifdef _WIN32
    FILETIME ft{};
    SYSTEMTIME st{};

    GetSystemTime( &st ); // gets current time
    SystemTimeToFileTime( &st, &ft ); // converts to file time format
    return ft;
#else
    auto current_time = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t( current_time );
    return time_to_FILETIME( time );
#endif
}

uint32_t StdInputItem::attributes() const noexcept {
    return static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
}
