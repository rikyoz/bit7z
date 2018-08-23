#ifndef BITPROPVARIANT_HPP
#define BITPROPVARIANT_HPP

#include <cstdint>
#include <string>
#include <Windows.h>

namespace bit7z {

    enum class BitProperty : uint32_t {
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

    static const wstring propertyNames[] = {
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
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Int8,
        Int16,
        Int32,
        Int64,
        Filetime
    };

    static const wstring typeNames[] = {
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

    struct BitPropVariant : public PROPVARIANT {
        BitPropVariant();
        BitPropVariant( const BitPropVariant& other );
        BitPropVariant( BitPropVariant&& other ) noexcept;
        virtual ~BitPropVariant();
        BitPropVariant& operator=( const BitPropVariant& other ) noexcept;
        BitPropVariant& operator=( BitPropVariant&& other ) noexcept;

        bool getBool() const;
        wstring getString() const;
        uint8_t getUint8() const;
        uint16_t getUint16() const;
        uint32_t getUint32() const;
        uint64_t getUint64() const;
        int8_t getInt8() const;
        int16_t getInt16() const;
        int32_t getInt32() const;
        int64_t getInt64() const;
        FILETIME getFiletime() const;

        wstring toString() const;
        bool isEmpty() const;
        BitPropVariantType type() const;
    };
}

#endif // BITPROPVARIANT_HPP
