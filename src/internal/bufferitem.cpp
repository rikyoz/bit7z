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

#include "internal/bufferitem.hpp"

#include "internal/cbufferinstream.hpp"
#include "internal/dateutil.hpp"
#include "internal/util.hpp"

using bit7z::BufferItem;
using bit7z::byte_t;
using bit7z::tstring;
using std::vector;

BufferItem::BufferItem( const vector< byte_t >& buffer, const tstring& name )
    : mBuffer{ buffer }, mBufferName{ name } {}

tstring BufferItem::name() const {
    return mBufferName.filename().string< tchar >();
}

tstring BufferItem::path() const {
    return mBufferName.string< tchar >();
}

fs::path BufferItem::inArchivePath() const {
    return mBufferName;
}

HRESULT BufferItem::getStream( ISequentialInStream** inStream ) const {
    auto inStreamLoc = bit7z::make_com< CBufferInStream, ISequentialInStream >( mBuffer );
    *inStream = inStreamLoc.Detach();
    return S_OK;
}

bool BufferItem::isDir() const noexcept {
    return false;
}

uint64_t BufferItem::size() const noexcept {
    return sizeof( byte_t ) * static_cast< uint64_t >( mBuffer.size() );
}

FILETIME BufferItem::creationTime() const noexcept { //-V524
    return currentFileTime();
}

FILETIME BufferItem::lastAccessTime() const noexcept { //-V524
    return currentFileTime();
}

FILETIME BufferItem::lastWriteTime() const noexcept {
    return currentFileTime();
}

uint32_t BufferItem::attributes() const noexcept {
    return static_cast< uint32_t >( FILE_ATTRIBUTE_NORMAL );
}
