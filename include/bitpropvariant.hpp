/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITPROPVARIANT_HPP
#define BITPROPVARIANT_HPP

#include <chrono>
#include <cstdint>

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"

namespace bit7z {
using time_type = std::chrono::time_point< std::chrono::system_clock >;

enum struct BitProperty : PROPID {
    NoProperty = 0,         ///<
    MainSubfile,            ///<
    HandlerItemIndex,       ///<
    Path,                   ///<
    Name,                   ///<
    Extension,              ///<
    IsDir,                  ///<
    Size,                   ///<
    PackSize,               ///<
    Attrib,                 ///<
    CTime,                  ///<
    ATime,                  ///<
    MTime,                  ///<
    Solid,                  ///<
    Commented,              ///<
    Encrypted,              ///<
    SplitBefore,            ///<
    SplitAfter,             ///<
    DictionarySize,         ///<
    CRC,                    ///<
    Type,                   ///<
    IsAnti,                 ///<
    Method,                 ///<
    HostOS,                 ///<
    FileSystem,             ///<
    User,                   ///<
    Group,                  ///<
    Block,                  ///<
    Comment,                ///<
    Position,               ///<
    Prefix,                 ///<
    NumSubDirs,             ///<
    NumSubFiles,            ///<
    UnpackVer,              ///<
    Volume,                 ///<
    IsVolume,               ///<
    Offset,                 ///<
    Links,                  ///<
    NumBlocks,              ///<
    NumVolumes,             ///<
    TimeType,               ///<
    Bit64,                  ///<
    BigEndian,              ///<
    Cpu,                    ///<
    PhySize,                ///<
    HeadersSize,            ///<
    Checksum,               ///<
    Characts,               ///<
    Va,                     ///<
    Id,                     ///<
    ShortName,              ///<
    CreatorApp,             ///<
    SectorSize,             ///<
    PosixAttrib,            ///<
    SymLink,                ///<
    Error,                  ///<
    TotalSize,              ///<
    FreeSpace,              ///<
    ClusterSize,            ///<
    VolumeName,             ///<
    LocalName,              ///<
    Provider,               ///<
    NtSecure,               ///<
    IsAltStream,            ///<
    IsAux,                  ///<
    IsDeleted,              ///<
    IsTree,                 ///<
    Sha1,                   ///<
    Sha256,                 ///<
    ErrorType,              ///<
    NumErrors,              ///<
    ErrorFlags,             ///<
    WarningFlags,           ///<
    Warning,                ///<
    NumStreams,             ///<
    NumAltStreams,          ///<
    AltStreamsSize,         ///<
    VirtualSize,            ///<
    UnpackSize,             ///<
    TotalPhySize,           ///<
    VolumeIndex,            ///<
    SubType,                ///<
    ShortComment,           ///<
    CodePage,               ///<
    IsNotArcType,           ///<
    PhySizeCantBeDetected,  ///<
    ZerosTailIsAllowed,     ///<
    TailSize,               ///<
    EmbeddedStubSize,       ///<
    NtReparse,              ///<
    HardLink,               ///<
    INode,                  ///<
    StreamId,               ///<
    ReadOnly,               ///<
    OutName,                ///<
    CopyLink                ///<
};

enum struct BitPropVariantType : uint32_t {
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
struct BitPropVariant final : public PROPVARIANT {
        /**
         * @brief Constructs an empty BitPropVariant object.
         */
        BitPropVariant();

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
        explicit BitPropVariant( const wchar_t* value );

        /**
         * @brief Constructs a string BitPropVariant from a wstring
         *
         * @param value the wstring value of the BitPropVariant
         */
        explicit BitPropVariant( const std::wstring& value );

        /**
         * @brief Constructs an 8-bit unsigned integer BitPropVariant
         *
         * @param value the uint8_t value of the BitPropVariant
         */
        explicit BitPropVariant( uint8_t value ) noexcept;

        /**
         * @brief Constructs a 16-bit unsigned integer BitPropVariant
         *
         * @param value the uint16_t value of the BitPropVariant
         */
        explicit BitPropVariant( uint16_t value ) noexcept;

        /**
         * @brief Constructs a 32-bit unsigned integer BitPropVariant
         *
         * @param value the uint32_t value of the BitPropVariant
         */
        explicit BitPropVariant( uint32_t value ) noexcept;

        /**
         * @brief Constructs a 64-bit unsigned integer BitPropVariant
         *
         * @param value the uint64_t value of the BitPropVariant
         */
        explicit BitPropVariant( uint64_t value ) noexcept;

        /**
         * @brief Constructs an 8-bit integer BitPropVariant
         *
         * @param value the int8_t value of the BitPropVariant
         */
        explicit BitPropVariant( int8_t value ) noexcept;

        /**
         * @brief Constructs a 16-bit integer BitPropVariant
         *
         * @param value the int16_t value of the BitPropVariant
         */
        explicit BitPropVariant( int16_t value ) noexcept;

        /**
         * @brief Constructs a 32-bit integer BitPropVariant
         *
         * @param value the int32_t value of the BitPropVariant
         */
        explicit BitPropVariant( int32_t value ) noexcept;

        /**
         * @brief Constructs a 64-bit integer BitPropVariant
         *
         * @param value the int64_t value of the BitPropVariant
         */
        explicit BitPropVariant( int64_t value ) noexcept;

        /**
         * @brief Constructs a FILETIME BitPropVariant
         *
         * @param value the FILETIME value of the BitPropVariant
         */
        explicit BitPropVariant( const FILETIME& value ) noexcept;

        /**
         * @brief BitPropVariant destructor.
         *
         * @note This is not virtual to maintain the same memory layout of the base struct!
         */
        ~BitPropVariant();

        /**
         * @brief Copy assignment operator.
         *
         * @param other the variant to be copied.
         *
         * @return a reference to *this object (with the copied values from other).
         */
        BitPropVariant& operator=( const BitPropVariant& other );

        /**
         * @brief Move assignment operator.
         *
         * @param other the variant to be moved.
         *
         * @return a reference to *this object (with the moved values from other).
         */
        BitPropVariant& operator=( BitPropVariant&& other ) noexcept;

        /**
         * @brief Assignment operator
         *
         * @note this will work only for T types for which a BitPropVariant constructor is defined!
         *
         * @param value the value to be assigned to the object
         *
         * @return a reference to *this object having the value as new variant value
         */
        template< typename T >
        BitPropVariant& operator=( const T& value ) noexcept( std::is_integral< T >::value ) {
            *this = BitPropVariant( value );
            return *this;
        }

        /**
         * @return the boolean value of this variant
         * (it throws an exception if the variant is not a boolean).
         */
        BIT7Z_NODISCARD bool getBool() const;

        /**
         * @return the string value of this variant
         * (it throws an exception if the variant is not a string).
         */
        BIT7Z_NODISCARD tstring getString() const;

        /**
         * @return the 8-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8-bit unsigned integer).
         */
        BIT7Z_NODISCARD uint8_t getUInt8() const;

        /**
         * @return the 16-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit unsigned integer).
         */
        BIT7Z_NODISCARD uint16_t getUInt16() const;

        /**
         * @return the 32-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit unsigned integer).
         */
        BIT7Z_NODISCARD uint32_t getUInt32() const;

        /**
         * @return the 64-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit unsigned integer).
         */
        BIT7Z_NODISCARD uint64_t getUInt64() const;

        /**
         * @return the 8-bit integer value of this variant
         * (it throws an exception if the variant is not an 8-bit integer).
         */
        BIT7Z_NODISCARD int8_t getInt8() const;

        /**
         * @return the 16-bit integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit integer).
         */
        BIT7Z_NODISCARD int16_t getInt16() const;

        /**
         * @return the 32-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit integer).
         */
        BIT7Z_NODISCARD int32_t getInt32() const;

        /**
         * @return the 64-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit integer).
         */
        BIT7Z_NODISCARD int64_t getInt64() const;

        /**
         * @return the FILETIME value of this variant
         * (it throws an exception if the variant is not a filetime).
         */
        BIT7Z_NODISCARD FILETIME getFileTime() const;

        /**
         * @return the FILETIME value of this variant converted to std::time_point
         * (it throws an exception if the variant is not a filetime).
         */
        BIT7Z_NODISCARD time_type getTimePoint() const;

        /**
         * @return the value of this variant converted from any supported type to std::wstring.
         */
        BIT7Z_NODISCARD tstring toString() const;

        /**
         * @return true if this variant is empty, false otherwise.
         */
        BIT7Z_NODISCARD bool isEmpty() const noexcept;

        /**
         * @return true if this variant is a boolean, false otherwise.
         */
        BIT7Z_NODISCARD bool isBool() const noexcept;

        /**
         * @return true if this variant is a string, false otherwise.
         */
        BIT7Z_NODISCARD bool isString() const noexcept;

        /**
         * @return true if this variant is an 8-bit unsigned integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isUInt8() const noexcept;

        /**
         * @return true if this variant is an 8 or 16-bit unsigned integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isUInt16() const noexcept;

        /**
         * @return true if this variant is an 8, 16 or 32-bit unsigned integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isUInt32() const noexcept;

        /**
         * @return true if this variant is an 8, 16, 32 or 64-bit unsigned integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isUInt64() const noexcept;

        /**
         * @return true if this variant is an 8-bit integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isInt8() const noexcept;

        /**
         * @return true if this variant is an 8 or 16-bit integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isInt16() const noexcept;

        /**
         * @return true if this variant is an 8, 16 or 32-bit integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isInt32() const noexcept;

        /**
         * @return true if this variant is an 8, 16, 32 or 64-bit integer, false otherwise.
         */
        BIT7Z_NODISCARD bool isInt64() const noexcept;

        /**
         * @return true if this variant is a FILETIME structure, false otherwise.
         */
        BIT7Z_NODISCARD bool isFileTime() const noexcept;

        /**
         * @return the BitPropVariantType of this variant.
         */
        BIT7Z_NODISCARD BitPropVariantType type() const;

        /**
         * @brief Clears the current value of the variant object
         */
        void clear() noexcept;

    private:
        void internalClear() noexcept;

        friend bool operator==( const BitPropVariant& a, const BitPropVariant& b ) noexcept;

        friend bool operator!=( const BitPropVariant& a, const BitPropVariant& b ) noexcept;
};

bool operator==( const BitPropVariant& a, const BitPropVariant& b ) noexcept;

bool operator!=( const BitPropVariant& a, const BitPropVariant& b ) noexcept;
}  // namespace bit7z

#endif // BITPROPVARIANT_HPP
