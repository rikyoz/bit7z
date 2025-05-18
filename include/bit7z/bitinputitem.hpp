/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITINPUTITEM_HPP
#define BITINPUTITEM_HPP

#include "bitfs.hpp"
#include "bitinputarchive.hpp"

#include <cstdint>
#include <functional>

struct ISequentialInStream;

namespace bit7z {

/** @cond **/
enum struct SymlinkPolicy : std::uint8_t {
    Follow,
    DoNotFollow
};
/** @endcond **/

struct FilesystemInputItem final {
    SymlinkPolicy symlinkPolicy = SymlinkPolicy::Follow;
};

using BufferInputItem = std::reference_wrapper< const buffer_t >;

using StdInputItem = std::reference_wrapper< std::istream >;

struct RenamedInputItemInitTag final {};

struct RenamedInputItem final {
    explicit RenamedInputItem( RenamedInputItemInitTag /* initTag */ ) {}
};

enum struct InputItemType : std::uint8_t {
    Filesystem, Buffer, StdStream, RenamedItem
};

struct InputItemProperties final {
    std::uint64_t size;
    FILETIME lastWriteTime;
    FILETIME lastAccessTime;
    FILETIME creationTime;
    std::uint32_t attributes;
    bool isDir;
    bool isSymLink;
    InputItemType inputType;
};

class BitInputItem final {
    public:
        explicit BitInputItem( const fs::path& itemPath, SymlinkPolicy symlinkPolicy = SymlinkPolicy::Follow );

        BitInputItem( const fs::path& itemPath,
                      const fs::path& inArchivePath,
                      SymlinkPolicy symlinkPolicy = SymlinkPolicy::Follow );

        BitInputItem( const fs::path& searchPath, const fs::directory_entry& entry, SymlinkPolicy symlinkPolicy );

        BitInputItem( const buffer_t& buffer, const tstring& path );

        BitInputItem( std::istream& stream, const tstring& path );

        BitInputItem( const BitInputArchive& inputArchive, std::uint32_t index, const tstring& newPath );

        BIT7Z_NODISCARD
        auto isDir() const noexcept -> bool;

        BIT7Z_NODISCARD
        auto isSymLink() const noexcept -> bool;

        BIT7Z_NODISCARD
        auto size() const noexcept -> std::uint64_t;

        BIT7Z_NODISCARD
        auto path() const -> const native_string&;

        BIT7Z_NODISCARD
        auto attributes() const noexcept -> std::uint32_t;

        BIT7Z_NODISCARD
        auto inArchivePath() const -> const sevenzip_string&;

        BIT7Z_NODISCARD
        auto itemProperty( BitProperty property ) const -> BitPropVariant;

        BIT7Z_NODISCARD
        auto getStream( ISequentialInStream** inStream ) const -> HRESULT;

        BIT7Z_NODISCARD
        auto hasNewData() const noexcept -> bool;

    private:
        InputItemProperties mProperties;
        // Note: we need to store paths as strings rather than fs::path as the public API is in C++14.
        native_string mPath; // std::wstring on Windows, std::string elsewhere.
        sevenzip_string mInArchivePath; // std::wstring on every OS.

        // Unfortunately, we cannot use std::variant as we need to support C++11/C++14 in the public API.
        union {
            FilesystemInputItem mFilesystemItem;
            BufferInputItem mBufferItem;
            StdInputItem mStdItem;
            RenamedInputItem mRenamedItem;
        };
};

} // namespace bit7z

#endif //BITINPUTITEM_HPP
