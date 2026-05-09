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
#if defined( _MSC_VER ) && _MSC_VER <= 1900
    explicit FilesystemInputItem( SymlinkPolicy policy ) : symlinkPolicy{ policy } {}
#endif
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
    InputItemType inputType;
};

/**
 * A generic input item for compression operations.
 */
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

        /**
         * @return true if the item represents a directory.
         */
        BIT7Z_NODISCARD
        auto isDir() const noexcept -> bool;

        /**
         * @return true if the item represents a symbolic link.
         */
        BIT7Z_NODISCARD
        auto isSymLink() const noexcept -> bool;

        /**
         * @return the uncompressed size of the item, in bytes.
         */
        BIT7Z_NODISCARD
        auto size() const noexcept -> std::uint64_t;

        /**
         * @return the native filesystem path of the item.
         */
        BIT7Z_NODISCARD
        auto path() const -> const native_string&;

        /**
         * @return the filesystem attributes of the item.
         */
        BIT7Z_NODISCARD
        auto attributes() const noexcept -> std::uint32_t;

        /**
         * @return the path of the item as it will be stored inside the archive.
         */
        BIT7Z_NODISCARD
        auto inArchivePath() const -> const sevenzip_string&;

        /**
         * @param property the property to retrieve.
         *
         * @return the value of the given property for this item.
         */
        BIT7Z_NODISCARD
        auto itemProperty( BitProperty property ) const -> BitPropVariant;

        /**
         * @brief Provides the input stream for this item to be used during compression.
         *
         * @note For internal use; not meant to be used by the user.
         *
         * @param inStream output parameter set to the item's input stream.
         *
         * @return S_OK on success, or an error HRESULT if the stream could not be opened.
         */
        BIT7Z_NODISCARD
        auto getStream( ISequentialInStream** inStream ) const -> HRESULT;

        /**
         * @return true if the item contributes new data to the archive
         *         (i.e., it is not merely a renamed entry from an existing archive).
         */
        BIT7Z_NODISCARD
        auto hasNewData() const noexcept -> bool;

        /**
         * @brief Sets the creation time of the item.
         *
         * @param creationTime the creation time to set.
         */
        void setCreationTime( time_type creationTime ) noexcept;

        /**
         * @brief Sets the last write time of the item.
         *
         * @param lastWriteTime the last write time to set.
         */
        void setLastWriteTime( time_type lastWriteTime ) noexcept;

        /**
         * @brief Sets the last access time of the item.
         *
         * @param lastAccessTime the last access time to set.
         */
        void setLastAccessTime( time_type lastAccessTime ) noexcept;

    private:
        InputItemProperties mProperties;
        // Note: we need to store paths as strings rather than fs::path as the public API is in C++14.
        native_string mPath; // std::wstring on Windows, std::string elsewhere.
        sevenzip_string mInArchivePath; // std::wstring on every OS, used by 7-Zip.

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
