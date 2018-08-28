#ifndef BITPROPVARIANT_HPP
#define BITPROPVARIANT_HPP

#include <cstdint>
#include <string>
#include <array>

#include <Windows.h>

#if _MSC_VER <= 1700
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace bit7z {

    enum class BitProperty : PROPID {
        NoProperty = 0,
        MainSubfile,
        HandlerItemIndex,
        Path,
        Name,
        Extension,
        IsDir,
        Size,
        PackSize,
        Attrib,
        CTime,
        ATime,
        MTime,
        Solid,
        Commented,
        Encrypted,
        SplitBefore,
        SplitAfter,
        DictionarySize,
        CRC,
        Type,
        IsAnti,
        Method,
        HostOS,
        FileSystem,
        User,
        Group,
        Block,
        Comment,
        Position,
        Prefix,
        NumSubDirs,
        NumSubFiles,
        UnpackVer,
        Volume,
        IsVolume,
        Offset,
        Links,
        NumBlocks,
        NumVolumes,
        TimeType,
        Bit64,
        BigEndian,
        Cpu,
        PhySize,
        HeadersSize,
        Checksum,
        Characts,
        Va,
        Id,
        ShortName,
        CreatorApp,
        SectorSize,
        PosixAttrib,
        SymLink,
        Error,
        TotalSize,
        FreeSpace,
        ClusterSize,
        VolumeName,
        LocalName,
        Provider,
        NtSecure,
        IsAltStream,
        IsAux,
        IsDeleted,
        IsTree,
        Sha1,
        Sha256,
        ErrorType,
        NumErrors,
        ErrorFlags,
        WarningFlags,
        Warning,
        NumStreams,
        NumAltStreams,
        AltStreamsSize,
        VirtualSize,
        UnpackSize,
        TotalPhySize,
        VolumeIndex,
        SubType,
        ShortComment,
        CodePage,
        IsNotArcType,
        PhySizeCantBeDetected,
        ZerosTailIsAllowed,
        TailSize,
        EmbeddedStubSize,
        NtReparse,
        HardLink,
        INode,
        StreamId,
        ReadOnly,
        OutName,
        CopyLink
    };

    using std::wstring;
    using std::array;

    static const array < wstring, static_cast< int >( BitProperty::CopyLink ) + 1 > propertyNames = {
        L"NoProperty",
        L"MainSubfile",
        L"HandlerItemIndex",
        L"Path",
        L"Name",
        L"Extension",
        L"IsDir",
        L"Size",
        L"PackSize",
        L"Attrib",
        L"CTime",
        L"ATime",
        L"MTime",
        L"Solid",
        L"Commented",
        L"Encrypted",
        L"SplitBefore",
        L"SplitAfter",
        L"DictionarySize",
        L"CRC",
        L"Type",
        L"IsAnti",
        L"Method",
        L"HostOS",
        L"FileSystem",
        L"User",
        L"Group",
        L"Block",
        L"Comment",
        L"Position",
        L"Prefix",
        L"NumSubDirs",
        L"NumSubFiles",
        L"UnpackVer",
        L"Volume",
        L"IsVolume",
        L"Offset",
        L"Links",
        L"NumBlocks",
        L"NumVolumes",
        L"TimeType",
        L"Bit64",
        L"BigEndian",
        L"Cpu",
        L"PhySize",
        L"HeadersSize",
        L"Checksum",
        L"Characts",
        L"Va",
        L"Id",
        L"ShortName",
        L"CreatorApp",
        L"SectorSize",
        L"PosixAttrib",
        L"SymLink",
        L"Error",
        L"TotalSize",
        L"FreeSpace",
        L"ClusterSize",
        L"VolumeName",
        L"LocalName",
        L"Provider",
        L"NtSecure",
        L"IsAltStream",
        L"IsAux",
        L"IsDeleted",
        L"IsTree",
        L"Sha1",
        L"Sha256",
        L"ErrorType",
        L"NumErrors",
        L"ErrorFlags",
        L"WarningFlags",
        L"Warning",
        L"NumStreams",
        L"NumAltStreams",
        L"AltStreamsSize",
        L"VirtualSize",
        L"UnpackSize",
        L"TotalPhySize",
        L"VolumeIndex",
        L"SubType",
        L"ShortComment",
        L"CodePage",
        L"IsNotArcType",
        L"PhySizeCantBeDetected",
        L"ZerosTailIsAllowed",
        L"TailSize",
        L"EmbeddedStubSize",
        L"NtReparse",
        L"HardLink",
        L"INode",
        L"StreamId",
        L"ReadOnly",
        L"OutName",
        L"CopyLink"
    };

    enum class BitPropVariantType : uint32_t {
        Empty,
        Bool,
        String,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Int8,
        Int16,
        Int32,
        Int64,
        Filetime
    };

    static const array < wstring, static_cast< uint32_t >( BitPropVariantType::Filetime ) + 1 > typeNames = {
        L"Empty",
        L"Bool",
        L"String",
        L"Uint8",
        L"Uint16",
        L"Uint32",
        L"Uint64",
        L"Int8",
        L"Int16",
        L"Int32",
        L"Int64",
        L"Filetime"
    };

    /**
     * @brief The BitPropVariant struct is a light extension to the WinAPI PROPVARIANT struct providing useful getters.
     */
    struct BitPropVariant : public PROPVARIANT {
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
        BitPropVariant( BitPropVariant&& other ) NOEXCEPT;

        /**
         * @brief BitPropVariant destructor.
         */
        virtual ~BitPropVariant();

        /**
         * @brief Copy assignment operator.
         *
         * @param other the variant to be copied.
         *
         * @return a reference to *this object (with the copied values from other).
         */
        BitPropVariant& operator=( const BitPropVariant& other ) NOEXCEPT;

        /**
         * @brief Move assignment operator.
         *
         * @param other the variant to be moved.
         *
         * @return a reference to *this object (with the moved values from other).
         */
        BitPropVariant& operator=( BitPropVariant&& other ) NOEXCEPT;

        /**
         * @return the boolean value of this variant
         * (it throws an expcetion if the variant is not a boolean).
         */
        bool getBool() const;

        /**
         * @return the string value of this variant
         * (it throws an exception if the variant is not a string).
         */
        wstring getString() const;

        /**
         * @return the 8-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8-bit unsigned integer).
         */
        uint8_t getUInt8() const;

        /**
         * @return the 16-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit unsigned integer).
         */
        uint16_t getUInt16() const;

        /**
         * @return the 32-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit unsigned integer).
         */
        uint32_t getUInt32() const;

        /**
         * @return the 64-bit unsigned integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit unsigned integer).
         */
        uint64_t getUInt64() const;

        /**
         * @return the 8-bit integer value of this variant
         * (it throws an exception if the variant is not an 8-bit integer).
         */
        int8_t getInt8() const;

        /**
         * @return the 16-bit integer value of this variant
         * (it throws an exception if the variant is not an 8 or 16-bit integer).
         */
        int16_t getInt16() const;

        /**
         * @return the 32-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16 or 32-bit integer).
         */
        int32_t getInt32() const;

        /**
         * @return the 64-bit integer value of this variant
         * (it throws an exception if the variant is not an 8, 16, 32 or 64-bit integer).
         */
        int64_t getInt64() const;

        /**
         * @return the FILETIME value of this variant
         * (it throws an exception if the variant is not a filetime).
         */
        FILETIME getFiletime() const;

        /**
         * @return the the value of this variant converted from any supported type to std::wstring.
         */
        wstring toString() const;

        /**
         * @return true if this variant is empty, false otherwise.
         */
        bool isEmpty() const;

        /**
         * @return the BitPropVariantType of this variant.
         */
        BitPropVariantType type() const;
    };
}

#endif // BITPROPVARIANT_HPP
