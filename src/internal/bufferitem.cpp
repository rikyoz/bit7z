// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/bufferitem.hpp"

#include "bittypes.hpp"
#include "bitwindows.hpp"
#include "internal/cbufferinstream.hpp"
#include "internal/dateutil.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

#include <cstdint>
#include <utility>

namespace bit7z {

BufferItem::BufferItem( const buffer_t& buffer, fs::path name )
    : mBuffer{ buffer }, mBufferName{ std::move( name ) } {}

auto BufferItem::name() const -> tstring {
    return path_to_tstring( mBufferName.filename() );
}

auto BufferItem::path() const -> tstring {
    return path_to_tstring( mBufferName );
}

auto BufferItem::inArchivePath() const -> fs::path {
    return mBufferName;
}

auto BufferItem::getStream( ISequentialInStream** inStream ) const -> HRESULT {
    auto inStreamLoc = bit7z::make_com< CBufferInStream, ISequentialInStream >( mBuffer );
    *inStream = inStreamLoc.Detach();
    return S_OK;
}

auto BufferItem::isDir() const noexcept -> bool {
    return false;
}

auto BufferItem::size() const noexcept -> std::uint64_t {
    return sizeof( byte_t ) * static_cast< std::uint64_t >( mBuffer.get().size() );
}

auto BufferItem::creationTime() const noexcept -> FILETIME { //-V524
    return current_file_time();
}

auto BufferItem::lastAccessTime() const noexcept -> FILETIME { //-V524
    return current_file_time();
}

auto BufferItem::lastWriteTime() const noexcept -> FILETIME {
    return current_file_time();
}

auto BufferItem::attributes() const noexcept -> std::uint32_t {
    return static_cast< std::uint32_t >( FILE_ATTRIBUTE_NORMAL );
}

auto BufferItem::filesystemPath() const -> const fs::path& {
    return mBufferName;
}

} // namespace bit7z