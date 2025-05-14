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
auto fileSize( const fs::path& filePath, SymlinkPolicy policy, bool isSymLink ) -> std::uint64_t {
    std::error_code error;
    if ( policy == SymlinkPolicy::DoNotFollow && isSymLink ) {
        return fs::read_symlink( filePath, error ).u8string().size();
    }
    const auto res = fs::file_size( filePath, error );
    return !error ? res : 0;
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

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto getFileAttributes( const fs::path& itemPath, SymlinkPolicy symlinkPolicy ) -> WIN32_FILE_ATTRIBUTE_DATA {
    WIN32_FILE_ATTRIBUTE_DATA fileAttributeData; // NOLINT(*-member-init)
    if ( !filesystem::fsutil::get_file_attributes_ex( itemPath, symlinkPolicy, fileAttributeData ) ) {
        //should not happen, but anyway...
        const auto error = last_error_code();
        throw BitException( "Could not retrieve file attributes", error, path_to_tstring( itemPath ) );
    }
    return fileAttributeData;
}
} // namespace

using filesystem::fsutil::in_archive_path;

namespace {
BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto fileProperties( const fs::path& itemPath, SymlinkPolicy policy ) -> InputItemProperties {
    std::error_code error;
    const auto fileStatus = fs::status( itemPath, error );
    if ( !fs::exists( fileStatus ) ) {
        error = std::make_error_code( std::errc::no_such_file_or_directory );
    }
    if ( error ) {
        throw BitException( "Invalid file path", error, path_to_tstring( itemPath ) );
    }
    const auto attributes = getFileAttributes( itemPath, policy );
    return {
        fileSize( itemPath, policy, fs::is_symlink( fileStatus ) ),
        attributes.ftLastWriteTime,
        attributes.ftLastAccessTime,
        attributes.ftCreationTime,
        static_cast< std::uint32_t >( attributes.dwFileAttributes ),
        fs::is_directory( fileStatus ),
        fs::is_symlink( fileStatus ),
        InputItemType::Filesystem
    };
}

BIT7Z_NODISCARD
BIT7Z_ALWAYS_INLINE
auto entryProperties( const fs::directory_entry& entry, SymlinkPolicy policy ) -> InputItemProperties {
    std::error_code error;
    // Note: there's no need to check if the entry exists, as it is obtained while indexing a directory.
    const auto isSymlink = entry.is_symlink( error );
    const auto attributes = getFileAttributes( entry.path(), policy );
    return {
        fileSize( entry, policy, isSymlink ),
        attributes.ftLastWriteTime,
        attributes.ftLastAccessTime,
        attributes.ftCreationTime,
        static_cast< std::uint32_t >( attributes.dwFileAttributes ),
        entry.is_directory( error ),
        isSymlink,
        InputItemType::Filesystem
    };
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
        false,
        false,
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
        false,
        false,
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
        inputArchive.itemProperty( index, BitProperty::IsDir ).getBool(),
        inputArchive.itemAt( index ).isSymLink(),
        InputItemType::RenamedItem
    };
}
} // namespace

BitInputItem::BitInputItem( const fs::path& itemPath, SymlinkPolicy symlinkPolicy )
    : BitInputItem{ itemPath, fs::path{}, symlinkPolicy } {}

BitInputItem::BitInputItem( const fs::path& itemPath, const fs::path& inArchivePath, SymlinkPolicy symlinkPolicy )
    : mProperties{ fileProperties( itemPath, symlinkPolicy ) },
      mPath{ path_to_tstring( itemPath ) },
      mInArchivePath{ path_to_wide_string( !inArchivePath.empty() ? inArchivePath : in_archive_path( itemPath ) ) },
      mFilesystemItem{ symlinkPolicy } {}

BitInputItem::BitInputItem( const fs::path& searchPath, const fs::directory_entry& entry, SymlinkPolicy symlinkPolicy )
    : mProperties{ entryProperties( entry, symlinkPolicy ) },
      mPath{ path_to_tstring( entry.path() ) },
      mInArchivePath{ path_to_wide_string( in_archive_path( entry.path(), searchPath ) ) },
      mFilesystemItem{ symlinkPolicy } {}

BitInputItem::BitInputItem( const buffer_t& buffer, const tstring& path )
    : mProperties{ bufferProperties( buffer ) },
      mPath{ path },
      mInArchivePath{ WIDEN( path ) },
      mBufferItem{ buffer } {}

BitInputItem::BitInputItem( std::istream& stream, const tstring& path )
    : mProperties{ streamProperties( stream ) },
      mPath{ path },
      mInArchivePath{ WIDEN( path ) },
      mStdItem{ stream } {}

BitInputItem::BitInputItem( const BitInputArchive& inputArchive, std::uint32_t index, const tstring& newPath )
    : mProperties{ renamedItemProperties( inputArchive, index ) },
      mPath{ newPath },
      mInArchivePath{ WIDEN( newPath ) },
      mRenamedItem{ RenamedInputItemInitTag{} } {}

auto BitInputItem::isDir() const noexcept -> bool {
    return mProperties.isDir;
}

auto BitInputItem::isSymLink() const noexcept -> bool {
    return mProperties.isSymLink;
}

auto BitInputItem::size() const noexcept -> std::uint64_t {
    return mProperties.size;
}

auto BitInputItem::path() const -> const tstring& {
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
            prop = mProperties.isDir;
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
        const auto path = tstring_to_path( mPath );
        // NOLINTNEXTLINE(*-pro-type-union-access)
        if ( mFilesystemItem.symlinkPolicy == SymlinkPolicy::DoNotFollow && isSymLink() ) {
            inStreamLoc = bit7z::make_com< CSymlinkInStream >( path );
        } else {
            inStreamLoc = bit7z::make_com< CFileInStream >( path );
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