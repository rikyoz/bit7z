// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "bitinputitem.hpp"

#include "bitexception.hpp"
#include "internal/cbufferinstream.hpp"
#include "internal/cfileinstream.hpp"
#include "internal/csymlinkinstream.hpp"
#include "internal/dateutil.hpp"
#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

namespace bit7z {
namespace {
BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto fileSize( const fs::path& filePath,
               const FileMetadata& data,
               SymlinkPolicy policy,
               bool isSymLink ) -> std::uint64_t {
    if ( policy == SymlinkPolicy::DoNotFollow && isSymLink ) {
        std::error_code error;
        return fs::read_symlink( filePath, error ).u8string().size();
    }
#ifdef _WIN32
    return ( static_cast< std::uint64_t >( data.nFileSizeHigh ) << 32 ) | data.nFileSizeLow;
#else
    return static_cast< std::uint64_t >( data.st_size );
#endif
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto streamSize( std::istream& stream ) -> std::uint64_t {
    const auto originalPos = stream.tellg();
    stream.seekg( 0, std::ios::end ); // seeking to the end of the stream
    const auto result = static_cast< std::uint64_t >( stream.tellg() - originalPos ); // size of the stream
    stream.seekg( originalPos ); // seeking back to the original position in the stream
    return result;
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto getFileTime( const BitInputArchive& inputArchive, std::uint32_t index, BitProperty property ) -> FILETIME {
    const BitPropVariant creationTime = inputArchive.itemProperty( index, property );
    return creationTime.isFileTime() ? creationTime.getFileTime() : current_file_time();
}

#define HAS_FLAG( attributes, x ) \
    ( ( ( attributes ) & static_cast< decltype( attributes ) >( x ) ) == static_cast< decltype( attributes ) >( x ) )

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto fileProperties( const fs::path& itemPath, SymlinkPolicy policy ) -> InputItemProperties {
    const FileMetadata fileMetadata = filesystem::fsutil::get_file_metadata( itemPath, policy );
#ifdef _WIN32
    const bool isSymLink = HAS_FLAG( fileMetadata.dwFileAttributes, FILE_ATTRIBUTE_REPARSE_POINT );
    return {
        fileSize( itemPath, fileMetadata, policy, isSymLink ),
        fileMetadata.ftLastWriteTime,
        fileMetadata.ftLastAccessTime,
        fileMetadata.ftCreationTime,
        static_cast< std::uint32_t >( fileMetadata.dwFileAttributes ),
        InputItemType::Filesystem
    };
#else
    std::uint32_t fileAttributes = S_ISDIR( fileMetadata.st_mode ) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_ARCHIVE;
    if ( ( fileMetadata.st_mode & S_IWUSR ) == 0 ) {
        fileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    const bool isSymLink = S_ISLNK( fileMetadata.st_mode );
    if ( isSymLink ) {
        fileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
    }
    constexpr auto kMask = 0xFFFFu;
    const std::uint32_t unixAttributes = ( ( fileMetadata.st_mode & kMask ) << 16u );
    fileAttributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + unixAttributes;
    return {
        fileSize( itemPath, fileMetadata, policy, isSymLink ),
        time_to_FILETIME( fileMetadata.st_mtime ),
        time_to_FILETIME( fileMetadata.st_atime ),
        time_to_FILETIME( fileMetadata.st_ctime ),
        fileAttributes,
        InputItemType::Filesystem
    };
#endif
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto bufferProperties( const buffer_t& buffer ) -> InputItemProperties {
    const auto currentTime = current_file_time();
    return {
        sizeof( byte_t ) * static_cast< std::uint64_t >( buffer.size() ),
        currentTime,
        currentTime,
        currentTime,
        static_cast< std::uint32_t >( FILE_ATTRIBUTE_NORMAL ),
        InputItemType::Buffer
    };
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto streamProperties( std::istream& stream ) -> InputItemProperties {
    const auto currentTime = current_file_time();
    return {
        streamSize( stream ),
        currentTime,
        currentTime,
        currentTime,
        static_cast< std::uint32_t >( FILE_ATTRIBUTE_NORMAL ),
        InputItemType::StdStream
    };
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto renamedItemProperties( const BitInputArchive& inputArchive, std::uint32_t index ) -> InputItemProperties {
    return {
        inputArchive.itemProperty( index, BitProperty::Size ).getUInt64(),
        getFileTime( inputArchive, index, BitProperty::MTime ),
        getFileTime( inputArchive, index, BitProperty::ATime ),
        getFileTime( inputArchive, index, BitProperty::CTime ),
        inputArchive.itemProperty( index, BitProperty::Attrib ).getUInt32(),
        InputItemType::RenamedItem
    };
}
} // namespace

using filesystem::fsutil::in_archive_path;

BitInputItem::BitInputItem( const fs::path& itemPath, SymlinkPolicy symlinkPolicy )
    : BitInputItem{ itemPath, fs::path{}, symlinkPolicy } {}

BitInputItem::BitInputItem( const fs::path& itemPath, const fs::path& inArchivePath, SymlinkPolicy symlinkPolicy )
    : mProperties{ fileProperties( itemPath, symlinkPolicy ) },
      mPath{ itemPath.native() },
      mInArchivePath{ path_to_wide_string( !inArchivePath.empty() ? inArchivePath : in_archive_path( itemPath ) ) },
      mFilesystemItem{ symlinkPolicy } {}

BitInputItem::BitInputItem( const fs::path& searchPath, const fs::directory_entry& entry, SymlinkPolicy symlinkPolicy )
    : mProperties{ fileProperties( entry.path(), symlinkPolicy ) },
      mPath{ entry.path().native() },
      mInArchivePath{ path_to_wide_string( in_archive_path( entry.path(), searchPath ) ) },
      mFilesystemItem{ symlinkPolicy } {}

BitInputItem::BitInputItem( const buffer_t& buffer, const tstring& path )
    : mProperties{ bufferProperties( buffer ) },
      mPath{ NATIVE( path ) },
      mInArchivePath{ WIDEN( path ) },
      mBufferItem{ buffer } {}

BitInputItem::BitInputItem( std::istream& stream, const tstring& path )
    : mProperties{ streamProperties( stream ) },
      mPath{ NATIVE( path ) },
      mInArchivePath{ WIDEN( path ) },
      mStdItem{ stream } {}

BitInputItem::BitInputItem( const BitInputArchive& inputArchive, std::uint32_t index, const tstring& newPath )
    : mProperties{ renamedItemProperties( inputArchive, index ) },
      mPath{ NATIVE( newPath ) },
      mInArchivePath{ WIDEN( newPath ) },
      mRenamedItem{ RenamedInputItemInitTag{} } {}

auto BitInputItem::isDir() const noexcept -> bool {
    return HAS_FLAG( mProperties.attributes, FILE_ATTRIBUTE_DIRECTORY );
}

auto BitInputItem::isSymLink() const noexcept -> bool {
    return HAS_FLAG( mProperties.attributes, FILE_ATTRIBUTE_REPARSE_POINT );
}

auto BitInputItem::size() const noexcept -> std::uint64_t {
    return mProperties.size;
}

auto BitInputItem::path() const -> const native_string& {
    return mPath;
}

auto BitInputItem::attributes() const noexcept -> std::uint32_t {
    return mProperties.attributes;
}

auto BitInputItem::inArchivePath() const -> const sevenzip_string& {
    return mInArchivePath;
}

auto BitInputItem::itemProperty( BitProperty property ) const -> BitPropVariant {
    BitPropVariant prop;
    switch ( property ) {
        case BitProperty::Path:
            prop = mInArchivePath;
            break;
        case BitProperty::IsDir:
            prop = isDir();
            break;
        case BitProperty::Size:
            prop = mProperties.size;
            break;
        case BitProperty::Attrib:
            prop = mProperties.attributes;
            break;
        case BitProperty::CTime:
            prop = mProperties.creationTime;
            break;
        case BitProperty::ATime:
            prop = mProperties.lastAccessTime;
            break;
        case BitProperty::MTime:
            prop = mProperties.lastWriteTime;
            break;
        default: //empty prop
            break;
    }
    return prop;
}

auto BitInputItem::getStream( ISequentialInStream** inStream ) const -> HRESULT try {
    if ( isDir() ) {
        return S_OK;
    }

    CMyComPtr< ISequentialInStream > inStreamLoc;
    if ( mProperties.inputType == InputItemType::Filesystem ) {
        // NOLINTNEXTLINE(*-pro-type-union-access)
        if ( mFilesystemItem.symlinkPolicy == SymlinkPolicy::DoNotFollow && isSymLink() ) {
            inStreamLoc = bit7z::make_com< CSymlinkInStream >( mPath );
        } else {
            inStreamLoc = bit7z::make_com< CFileInStream >( mPath );
        }
    } else if ( mProperties.inputType == InputItemType::Buffer ) {
        // NOLINTNEXTLINE(*-pro-type-union-access)
        inStreamLoc = bit7z::make_com< CBufferInStream, ISequentialInStream >( mBufferItem );
    } else if ( mProperties.inputType == InputItemType::StdStream ) {
        // NOLINTNEXTLINE(*-pro-type-union-access)
        inStreamLoc = bit7z::make_com< CStdInStream, ISequentialInStream >( mStdItem );
    }
    *inStream = inStreamLoc.Detach();
    return S_OK;
} catch ( const BitException& ex ) {
    return ex.hresultCode();
}

auto BitInputItem::hasNewData() const noexcept -> bool {
    return mProperties.inputType != InputItemType::RenamedItem;
}

} // namespace bit7z