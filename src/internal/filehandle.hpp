/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FILEHANDLE_HPP
#define FILEHANDLE_HPP

#include "bitwindows.hpp" // For HRESULT
#include "internal/fs.hpp"

#include <Common/MyTypes.h> // For 7-Zip integer types

#include <cstdint>
#include <cstdio>

namespace bit7z {

using handle_t = int;

enum class SeekOrigin : std::uint8_t {
    Begin = SEEK_SET,
    CurrentPosition = SEEK_CUR,
    End = SEEK_END
};

class FileHandle {
    protected:
        handle_t mHandle{ -1 };

        explicit FileHandle( handle_t handle );

    public:
        explicit FileHandle( const FileHandle& ) = delete;

        explicit FileHandle( FileHandle&& ) = delete;

        auto operator=( const FileHandle& ) = delete;

        auto operator=( FileHandle&& ) = delete;

        ~FileHandle();

        auto seek( SeekOrigin origin, Int64 distance, UInt64* newPosition ) const noexcept -> HRESULT;
};

struct OutputFile final : public FileHandle {
    explicit OutputFile( const fs::path& filePath, bool createAlways );

    auto write( const void* data, UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT;
};

struct InputFile final : public FileHandle {
    explicit InputFile( const fs::path& filePath );

    auto read( void* data, UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT;
};

} // namespace bit7z

#endif //FILEHANDLE_HPP
