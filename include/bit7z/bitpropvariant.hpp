/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITPROPVARIANT_HPP
#define BITPROPVARIANT_HPP

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>

namespace bit7z {

/**
 * @brief A type representing a time point measured using the system clock.
 */
using time_type = std::chrono::time_point< std::chrono::system_clock >;

/**
 * @brief The BitProperty enum represents the archive/item properties that 7-zip can read or write.
 */
enum struct BitProperty : PROPID { // NOLINT(*-enum-size)
    NoProperty = 0,         ///< No property (placeholder for an invalid or unset property).
    MainSubfile,            ///< The index of the main subfile of the archive.
    HandlerItemIndex,       ///< The internal item index used by the archive handler.
    Path,                   ///< The path of the item.
    Name,                   ///< The name of the item.
    Extension,              ///< The file extension of the item.
    IsDir,                  ///< Whether the item is a directory.
    Size,                   ///< The uncompressed size of the item.
    PackSize,               ///< The compressed size of the item.
    Attrib,                 ///< The attributes of the item.
    CTime,                  ///< The creation time of the item.
    ATime,                  ///< The last access time of the item.
    MTime,                  ///< The last write time of the item.
    Solid,                  ///< Whether the item uses solid compression.
    Commented,              ///< Whether the item has a comment.
    Encrypted,              ///< Whether the item is encrypted.
    SplitBefore,            ///< Whether the item is continued from a previous volume.
    SplitAfter,             ///< Whether the item is continued in the next volume.
    DictionarySize,         ///< The dictionary size used to compress the item.
    CRC,                    ///< The CRC checksum of the item.
    Type,                   ///< The type of the item.
    IsAnti,                 ///< Whether the item is an anti-item (a deletion marker).
    Method,                 ///< The compression method used for the item.
    HostOS,                 ///< The host OS on which the item was created.
    FileSystem,             ///< The file system on which the item was stored.
    User,                   ///< The user owning the item.
    Group,                  ///< The group owning the item.
    Block,                  ///< The index of the block containing the item.
    Comment,                ///< The comment associated with the item.
    Position,               ///< The position of the item.
    Prefix,                 ///< The path prefix of the item.
    NumSubDirs,             ///< The number of subdirectories contained in the item.
    NumSubFiles,            ///< The number of subfiles contained in the item.
    UnpackVer,              ///< The format version required to extract the item.
    Volume,                 ///< The volume the item belongs to.
    IsVolume,               ///< Whether the archive is a multi-volume archive.
    Offset,                 ///< The offset of the item.
    Links,                  ///< The number of hard links to the item.
    NumBlocks,              ///< The number of blocks in the archive.
    NumVolumes,             ///< The number of volumes composing the archive.
    TimeType,               ///< The type (precision) of the item timestamps.
    Bit64,                  ///< Whether the archive uses 64-bit values.
    BigEndian,              ///< Whether the values are stored in big-endian byte order.
    Cpu,                    ///< The target CPU architecture.
    PhySize,                ///< The physical size of the archive.
    HeadersSize,            ///< The size of the archive headers.
    Checksum,               ///< The checksum of the item.
    Characts,               ///< The characteristics of the item.
    Va,                     ///< The virtual address of the item.
    Id,                     ///< The identifier of the item.
    ShortName,              ///< The short (DOS 8.3) name of the item.
    CreatorApp,             ///< The application that created the archive.
    SectorSize,             ///< The sector size of the volume.
    PosixAttrib,            ///< The POSIX attributes of the item.
    SymLink,                ///< The symbolic link target of the item.
    Error,                  ///< An error associated with the item.
    TotalSize,              ///< The total size of the volume.
    FreeSpace,              ///< The free space available on the volume.
    ClusterSize,            ///< The cluster size of the volume.
    VolumeName,             ///< The label of the volume.
    LocalName,              ///< The local name of the item.
    Provider,               ///< The provider of the item.
    NtSecure,               ///< The NT security descriptor of the item.
    IsAltStream,            ///< Whether the item is an alternate data stream.
    IsAux,                  ///< Whether the item is an auxiliary item.
    IsDeleted,              ///< Whether the item is marked as deleted.
    IsTree,                 ///< Whether the archive items are organized as a tree.
    Sha1,                   ///< The SHA-1 hash of the item.
    Sha256,                 ///< The SHA-256 hash of the item.
    ErrorType,              ///< The type of error detected while opening the archive.
    NumErrors,              ///< The number of errors detected in the archive.
    ErrorFlags,             ///< The flags describing the errors detected while opening the archive.
    WarningFlags,           ///< The flags describing the warnings detected while opening the archive.
    Warning,                ///< A warning associated with the archive.
    NumStreams,             ///< The number of streams in the archive.
    NumAltStreams,          ///< The number of alternate data streams of the item.
    AltStreamsSize,         ///< The total size of the alternate data streams of the item.
    VirtualSize,            ///< The virtual size of the item.
    UnpackSize,             ///< The uncompressed size of the archive.
    TotalPhySize,           ///< The total physical size of the multi-volume archive.
    VolumeIndex,            ///< The index of the volume containing the item.
    SubType,                ///< The subtype of the archive.
    ShortComment,           ///< A short comment describing the archive.
    CodePage,               ///< The code page used to encode the item.
    IsNotArcType,           ///< Whether the file is not of the detected archive type.
    PhySizeCantBeDetected,  ///< Whether the physical size of the archive cannot be detected.
    ZerosTailIsAllowed,     ///< Whether a tail of zero bytes is allowed after the archive data.
    TailSize,               ///< The size of the data following the archive (tail).
    EmbeddedStubSize,       ///< The size of the executable stub embedded before the archive data.
    NtReparse,              ///< The NT reparse point data of the item.
    HardLink,               ///< The hard link target of the item.
    INode,                  ///< The inode number of the item.
    StreamId,               ///< The identifier of the data stream of the item.
    ReadOnly,               ///< Whether the item is read-only.
    OutName,                ///< The suggested output name for the item.
    CopyLink                ///< The copy link of the item.
};

/**
 * @brief Returns the name of the given archive/item property.
 *
 * @param property  the property whose name will be returned.
 *
 * @return the name of the given property.
 */
auto to_string( BitProperty property ) -> std::string;

/**
 * @brief The BitPropVariantType enum represents the possible types that a BitPropVariant can store.
 */
enum struct BitPropVariantType : std::uint8_t {
    Empty,      ///< Empty BitPropVariant type
    Bool,       ///< Boolean BitPropVariant type
    String,     ///< String BitPropVariant type
    UInt8,      ///< 8-bit unsigned int BitPropVariant type
    UInt16,     ///< 16-bit unsigned int BitPropVariant type
    UInt32,     ///< 32-bit unsigned int BitPropVariant type
    UInt64,     ///< 64-bit unsigned int BitPropVariant type
    Int8,       ///< 8-bit signed int BitPropVariant type
    Int16,      ///< 16-bit signed int BitPropVariant type
    Int32,      ///< 32-bit signed int BitPropVariant type
    Int64,      ///< 64-bit signed int BitPropVariant type
    FileTime    ///< FILETIME BitPropVariant type
};

/**
 * @brief The BitPropVariant struct is a light extension to the WinAPI PROPVARIANT struct providing useful getters.
 */
struct BitPropVariant final : PROPVARIANT {
        /**
         * @brief Constructs an empty BitPropVariant object.
         */
        BitPropVariant() noexcept;

        /**
         * @brief Copy constructs this BitPropVariant from another one.
         *
         * @param other the variant to be copied.
         */
        BitPropVariant( const BitPropVariant& other );

        /**
         * @brief Move constructs this BitPropVariant from another one.
         *
         * @param other the variant to be moved.
         */
        BitPropVariant( BitPropVariant&& other ) noexcept;

        /**
         * @brief Constructs a boolean BitPropVariant
         *
         * @param value the bool value of the BitPropVariant
         */
        explicit BitPropVariant( bool value ) noexcept;

        /**
         * @brief Constructs a string BitPropVariant from a null-terminated C wide string
         *
         * @param value the null-terminated C wide string value of the BitPropVariant
         */
        explicit BitPropVariant( const sevenzip_string::value_type* value );

        /**
         * @brief Constructs a string BitPropVariant from a sevenzip_string
         *
         * @param value the sevenzip_string value of the BitPropVariant
         */
        explicit BitPropVariant( const sevenzip_string& value );

        /**
         * @brief Constructs an 8-bit unsigned integer BitPropVariant
         *
         * @param value the std::uint8_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::uint8_t value ) noexcept;

        /**
         * @brief Constructs a 16-bit unsigned integer BitPropVariant
         *
         * @param value the std::uint16_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::uint16_t value ) noexcept;

        /**
         * @brief Constructs a 32-bit unsigned integer BitPropVariant
         *
         * @param value the std::uint32_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::uint32_t value ) noexcept;

        /**
         * @brief Constructs a 64-bit unsigned integer BitPropVariant
         *
         * @param value the std::uint64_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::uint64_t value ) noexcept;

        /**
         * @brief Constructs an 8-bit integer BitPropVariant
         *
         * @param value the std::int8_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::int8_t value ) noexcept;

        /**
         * @brief Constructs a 16-bit integer BitPropVariant
         *
         * @param value the std::int16_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::int16_t value ) noexcept;

        /**
         * @brief Constructs a 32-bit integer BitPropVariant
         *
         * @param value the std::int32_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::int32_t value ) noexcept;

        /**
         * @brief Constructs a 64-bit integer BitPropVariant
         *
         * @param value the std::int64_t value of the BitPropVariant
         */
        explicit BitPropVariant( std::int64_t value ) noexcept;

        /**
         * @brief Constructs a FILETIME BitPropVariant
         *
         * @param value the FILETIME value of the BitPropVariant
         */
        explicit BitPropVariant( FILETIME value ) noexcept;

        /**
         * @brief BitPropVariant destructor.
         *
         * @note This is not virtual to maintain the same memory layout of the base struct.
         */
        ~BitPropVariant();

        /**
         * @brief Copy assignment operator.
         *
         * @param other the variant to be copied.
         *
         * @return a reference to *this object (with the copied values from other).
         */
        auto operator=( const BitPropVariant& other ) -> BitPropVariant&;

        /**
         * @brief Move assignment operator.
         *
         * @param other the variant to be moved.
         *
         * @return a reference to *this object (with the moved values from other).
         */
        auto operator=( BitPropVariant&& other ) noexcept -> BitPropVariant&;

        /**
         * @brief Assignment operator
         *
         * @note this will work only for T types for which a BitPropVariant constructor is defined.
         *
         * @param value the value to be assigned to the object
         *
         * @return a reference to *this object having the value as new variant value
         */
        template< typename T >
        auto operator=( const T& value ) noexcept( std::is_integral< T >::value ) -> BitPropVariant& {
            *this = BitPropVariant{ value };
            return *this;
        }

        /**
         * @return the boolean value of this variant
         * (it throws an exception if the variant is not a boolean value).
         */
        BIT7Z_NODISCARD auto getBool() const -> bool;

        /**
         * @return the string value of this variant
         * (it throws an exception if the variant is not a string).
         */
        BIT7Z_NODISCARD auto getString() const -> tstring;

        /**
         * @return the native string value of this variant
         * (it throws an exception if the variant is not a string).
         */
        BIT7Z_NODISCARD auto getNativeString() const -> native_string;

        /**
         * @return the raw string value of this variant
         * (it throws an exception if the variant is not a string).
         */
        BIT7Z_NODISCARD auto getRawString() const -> sevenzip_string;

        /**
         * @return the 8-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8-bit unsigned integer).
         */
        BIT7Z_NODISCARD auto getUInt8() const -> std::uint8_t;

        /**
         * @return the 16-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit unsigned integer).
         */
        BIT7Z_NODISCARD auto getUInt16() const -> std::uint16_t;

        /**
         * @return the 32-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit unsigned integer).
         */
        BIT7Z_NODISCARD auto getUInt32() const -> std::uint32_t;

        /**
         * @return the 64-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit unsigned integer).
         */
        BIT7Z_NODISCARD auto getUInt64() const -> std::uint64_t;

        /**
         * @return the 8-bit integer value of this variant
         * (it throws an exception if the variant is not an 8-bit integer).
         */
        BIT7Z_NODISCARD auto getInt8() const -> std::int8_t;

        /**
         * @return the 16-bit integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit integer).
         */
        BIT7Z_NODISCARD auto getInt16() const -> std::int16_t;

        /**
         * @return the 32-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit integer).
         */
        BIT7Z_NODISCARD auto getInt32() const -> std::int32_t;

        /**
         * @return the 64-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit integer).
         */
        BIT7Z_NODISCARD auto getInt64() const -> std::int64_t;

        /**
         * @return the FILETIME value of this variant
         * (it throws an exception if the variant is not a filetime).
         */
        BIT7Z_NODISCARD auto getFileTime() const -> FILETIME;

        /**
         * @return the FILETIME value of this variant converted to std::time_point
         * (it throws an exception if the variant is not a filetime).
         */
        BIT7Z_NODISCARD auto getTimePoint() const -> time_type;

        /**
         * @return the value of this variant converted from any supported type to std::wstring.
         */
        BIT7Z_NODISCARD auto toString() const -> tstring;

        /**
         * @return a boolean value indicating whether the variant is empty.
         */
        BIT7Z_NODISCARD auto isEmpty() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is a boolean value.
         */
        BIT7Z_NODISCARD auto isBool() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is a string.
         */
        BIT7Z_NODISCARD auto isString() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8-bit unsigned integer.
         */
        BIT7Z_NODISCARD auto isUInt8() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8 or 16-bit unsigned integer.
         */
        BIT7Z_NODISCARD auto isUInt16() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8, 16 or 32-bit unsigned integer.
         */
        BIT7Z_NODISCARD auto isUInt32() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8, 16, 32 or 64-bit unsigned integer.
         */
        BIT7Z_NODISCARD auto isUInt64() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8-bit integer.
         */
        BIT7Z_NODISCARD auto isInt8() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8 or 16-bit integer.
         */
        BIT7Z_NODISCARD auto isInt16() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8, 16 or 32-bit integer.
         */
        BIT7Z_NODISCARD auto isInt32() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is an 8, 16, 32 or 64-bit integer.
         */
        BIT7Z_NODISCARD auto isInt64() const noexcept -> bool;

        /**
         * @return a boolean value indicating whether the variant is a FILETIME structure.
         */
        BIT7Z_NODISCARD auto isFileTime() const noexcept -> bool;

        /**
         * @return the BitPropVariantType of this variant.
         */
        BIT7Z_NODISCARD auto type() const -> BitPropVariantType;

        /**
         * @brief Clears the current value of the variant object
         */
        void clear() noexcept;

    private:
        void internalClear() noexcept;
};

/**
 * @brief Checks whether two BitPropVariant objects have the same type and value.
 *
 * @param lhs  the first variant to be compared.
 * @param rhs  the second variant to be compared.
 *
 * @return true if and only if the two variants have the same type and value.
 */
auto operator==( const BitPropVariant& lhs, const BitPropVariant& rhs ) noexcept -> bool;

/**
 * @brief Checks whether two BitPropVariant objects differ in type or value.
 *
 * @param lhs  the first variant to be compared.
 * @param rhs  the second variant to be compared.
 *
 * @return true if and only if the two variants differ in type or value.
 */
auto operator!=( const BitPropVariant& lhs, const BitPropVariant& rhs ) noexcept -> bool;

} // namespace bit7z

#endif // BITPROPVARIANT_HPP
