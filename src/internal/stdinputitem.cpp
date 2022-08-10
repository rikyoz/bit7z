// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/stdinputitem.hpp"

#include "internal/cstdinstream.hpp"
#include "internal/dateutil.hpp"

using bit7z::StdInputItem;
using bit7z::tstring;
using std::istream;

StdInputItem::StdInputItem( istream& stream, const tstring& path ) : mStream{ stream }, mStreamPath{ path } {}

tstring StdInputItem::name() const {
    return mStreamPath.filename().string< tchar >();
}

tstring StdInputItem::path() const {
    return mStreamPath.string< tchar >();
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
    return currentFileTime();
}

FILETIME StdInputItem::lastAccessTime() const noexcept { //-V524
    return currentFileTime();
}

FILETIME StdInputItem::lastWriteTime() const noexcept {
    return currentFileTime();
}

uint32_t StdInputItem::attributes() const noexcept {
    return static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
}
