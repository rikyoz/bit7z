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

#include "bittypes.hpp" // For to_underlying
#include "bitwindows.hpp" // For HRESULT
#include "internal/fs.hpp"

#include <Common/MyTypes.h> // For 7-Zip integer types

#include <cstdint>
#include <cstdio>
#include <fcntl.h>

namespace bit7z {

using handle_t = int;

enum struct SeekOrigin : std::int8_t {
    /** Set the file pointer at the given offset from the beginning of the file. */
    Begin = SEEK_SET,

    /** Set the file pointer at the given offset from the current file pointer position. */
    CurrentPosition = SEEK_CUR,

    /** Set the file pointer at the given offset from the end of the file. */
    End = SEEK_END
};

#ifdef _WIN32
#define O_FLAG(flag) _O_##flag

#ifndef S_IRUSR
constexpr std::uint16_t S_IRUSR = _S_IREAD;
#endif
#ifndef S_IWUSR
constexpr std::uint16_t S_IWUSR = _S_IWRITE;
#endif
#else
#define O_FLAG(flag) O_##flag

constexpr std::uint16_t O_BINARY = 0;
#endif

enum struct AccessFlag : std::uint8_t {
    /** Open a file only for reading. */
    ReadOnly = O_FLAG( RDONLY ),

    /** Open a file only for writing. */
    WriteOnly = O_FLAG( WRONLY ),

    /** Open a file for both reading and writing. */
    ReadWrite = O_FLAG( RDWR )
};

enum struct FileFlag : std::uint16_t {
    /** Open a file only if it already exists, fail otherwise. */
    Existing = O_FLAG( BINARY ),

    /** Open a file only if it already exists, truncating it to zero bytes. */
    TruncateExisting = O_FLAG( TRUNC ) | O_FLAG( BINARY ),

    /** Open a file only if it already exists, setting the file pointer to the end of the file. */
    AppendExisting = O_FLAG( APPEND ) | O_FLAG( BINARY ),

    /** Create a new file if it doesn't exist, fail otherwise. */
    CreateNew = O_FLAG( CREAT ) | O_FLAG( EXCL ) | O_FLAG( BINARY ),

    /** Open a file, creating it if it doesn't exist. */
    OpenAlways = O_FLAG( CREAT ) | O_FLAG( BINARY ),

    /** Open a file, creating it if it doesn't exist, truncating it if it already exists. */
    CreateAlways = O_FLAG( CREAT ) | O_FLAG( TRUNC ) | O_FLAG( BINARY ),

    /** Open a file, creating it if it doesn't exist, setting the file pointer to the end of the file. */
    AppendAlways = O_FLAG( CREAT ) | O_FLAG( APPEND ) | O_FLAG( BINARY ),
};

/** Strong typedef for the POSIX open flags. */
enum struct OpenFlags : std::uint16_t {};

/** Open flags can be created only by concatenating access and file flags. */
constexpr auto operator|( AccessFlag accessFlag, FileFlag fileFlag ) -> OpenFlags {
    return static_cast< OpenFlags >( to_underlying( accessFlag ) | to_underlying( fileFlag ) );
}

enum struct PermissionFlag : std::uint16_t {
    /** The user will be able to read the file. */
    Read = S_IRUSR,

    /** The user will be able to write to the file. */
    Write = S_IWUSR,

    /** The user will be able to read and write the file. */
    ReadWrite = S_IRUSR | S_IWUSR
};

class FileHandle {
    protected:
        handle_t mHandle{ -1 };

        explicit FileHandle( const fs::path& filePath, OpenFlags openFlags, PermissionFlag permissionFlag );

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
