/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FILEHANDLE_HPP
#define FILEHANDLE_HPP

#include "bittypes.hpp" // For to_underlying
#include "internal/fs.hpp"
#include "internal/windows.hpp" // For HRESULT, GetLastError, and Windows-specific flags

#include <cstdint>

#ifdef _WIN32
#include <cstdio> // For SEEK_ flags
#else
#include <fcntl.h> // For SEEK_ and O_ flags
#endif

namespace bit7z {

#ifdef _WIN32
using handle_t = HANDLE;
#else
using handle_t = int;
#endif

/**
 * @brief Enumeration defining the origin of a seek operation.
 */
enum struct SeekOrigin : std::int8_t {
    /** Set the file pointer at the given offset from the beginning of the file. */
    Begin = SEEK_SET,

    /** Set the file pointer at the given offset from the current file pointer position. */
    CurrentPosition = SEEK_CUR,

    /** Set the file pointer at the given offset from the end of the file. */
    End = SEEK_END
};

#ifdef _WIN32
/**
 * @brief Enumeration defining the kind of access to be requested when opening a file handle.
 *
 *   - ReadOnly → Open a file only for reading.
 *   - WriteOnly → Open a file only for writing.
 *   - ReadWrite → Open a file for both reading and writing.
 */
enum struct AccessFlag : std::uint32_t {
    ReadOnly = GENERIC_READ,
    WriteOnly = GENERIC_WRITE,
    ReadWrite = GENERIC_READ | GENERIC_WRITE
};
#else
enum struct AccessFlag : std::uint8_t {
    ReadOnly = O_RDONLY,
    WriteOnly = O_WRONLY,
    ReadWrite = O_RDWR
};
#endif

/**
 * @brief Enumeration representing file opening modes.
 */
#ifdef _WIN32
enum struct FileFlag : std::uint8_t {
    Existing         = OPEN_EXISTING,    ///< Open an existing file; fail if it does not exist.
    TruncateExisting = TRUNCATE_EXISTING, ///< Truncate an existing file to zero bytes; fail if it does not exist.
    CreateNew        = CREATE_NEW,        ///< Create a new file; fail if it already exists.
    OpenAlways       = OPEN_ALWAYS,       ///< Open a file if it exists, or create it if it does not.
    CreateAlways     = CREATE_ALWAYS      ///< Create a file, truncating it if it already exists.
};
#else
enum struct FileFlag : std::uint16_t {
    Existing         = 0,               ///< Open an existing file; fail if it does not exist.
    TruncateExisting = O_TRUNC,          ///< Truncate an existing file to zero bytes; fail if it does not exist.
    CreateNew        = O_CREAT | O_EXCL, ///< Create a new file; fail if it already exists.
    OpenAlways       = O_CREAT,          ///< Open a file if it exists, or create it if it does not.
    CreateAlways     = O_CREAT | O_TRUNC ///< Create a file, truncating it if it already exists.
};
#endif

struct OpenFlags {
#ifdef _WIN32
    AccessFlag accessFlag;
    FileFlag fileFlag;
#else
    private:
        std::uint16_t mValue;

    public:
        constexpr OpenFlags( AccessFlag accessFlag, FileFlag fileFlag ) noexcept
            : mValue{ static_cast< std::uint16_t >( to_underlying( accessFlag ) | to_underlying( fileFlag ) ) } {}

        BIT7Z_NODISCARD
        constexpr auto value() const noexcept -> std::uint16_t {
            return mValue;
        }
#endif
};

class FileHandle {
    protected:
        handle_t mHandle; // NOLINT(*-non-private-member-variables-in-classes)

        explicit FileHandle( const native_string& filePath, OpenFlags openFlags );

    public:
        explicit FileHandle( const FileHandle& ) = delete;

        explicit FileHandle( FileHandle&& ) = delete;

        auto operator=( const FileHandle& ) = delete;

        auto operator=( FileHandle&& ) = delete;

        ~FileHandle();

        BIT7Z_NODISCARD
        auto seek( SeekOrigin origin, std::int64_t distance, std::uint64_t& newPosition ) const noexcept -> HRESULT;
};

struct OutputFile final : FileHandle {
    explicit OutputFile( const native_string& filePath, FileFlag fileFlag );

    auto write( const void* data, std::uint32_t size, std::uint32_t& processedSize ) const noexcept -> HRESULT;

    BIT7Z_NODISCARD
    auto resize( std::uint64_t newSize ) const noexcept -> bool;

#ifdef _WIN32
    BIT7Z_NODISCARD
    auto setFileTime( FILETIME creation, FILETIME access, FILETIME modified ) const noexcept -> bool;
#endif
};

struct InputFile final : FileHandle {
    explicit InputFile( const native_string& filePath );

    auto read( void* data, std::uint32_t size, std::uint32_t& processedSize ) const noexcept -> HRESULT;
};

} // namespace bit7z

#endif //FILEHANDLE_HPP
