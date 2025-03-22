// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "archive.hpp"
#include "crc.hpp"
#include "filesystem.hpp"

#include <catch2/catch.hpp>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

void require_archive_item( const BitInFormat& format,
                           const BitArchiveItem& item,
                           const ExpectedItem& expectedItem,
                           const SourceLocation& location ) {
    INFO( "Failed while checking archive item " << Catch::StringMaker< tstring >::convert( item.name() ) )
    INFO( "  from " << location.file_name() << ":" << location.line() )
    REQUIRE( item.isDir() == ( expectedItem.fileInfo.type == fs::file_type::directory ) );

    if ( !item.isDir() ) {
        REQUIRE( item.isEncrypted() == expectedItem.isEncrypted );
    }

    if ( format_has_path_metadata( format ) ) {
        REQUIRE( item.extension() == expectedItem.fileInfo.ext );
        REQUIRE( item.name() == expectedItem.fileInfo.name );
        REQUIRE( item.path() == expectedItem.inArchivePath );
    }

    if ( format_has_size_metadata( format ) ) {
        /* Note: some archive formats (e.g. BZip2) do not provide the size metadata! */
        REQUIRE( item.size() == expectedItem.fileInfo.size );
    }

    const auto itemCrc = item.crc();
    if ( itemCrc != 0 && ( ( format != BitFormat::Rar5 ) || !item.isEncrypted() ) ) {
        /* Encrypted Rar5 archives have random CRCs values. */
        if ( format_has_crc32( format ) ) {
            REQUIRE( itemCrc == expectedItem.fileInfo.crc32 );
        } else if ( format_has_crc16( format ) ) {
            REQUIRE( itemCrc == expectedItem.fileInfo.crc16 );
        }
    }
}

void require_archive_content( const BitArchiveReader& info,
                              const TestArchiveContent& input,
                              const SourceLocation& location ) {
    INFO( "Failed while checking content of " << Catch::StringMaker< tstring >::convert( info.archivePath() ) )
    INFO( "  from " << location.file_name() << ":" << location.line() )
    REQUIRE_FALSE( info.archiveProperties().empty() );

    const auto& expectedArchiveContent = input.content();
    REQUIRE( info.itemsCount() == expectedArchiveContent.items.size() );
    REQUIRE( info.filesCount() == expectedArchiveContent.fileCount );
    REQUIRE( info.foldersCount() == ( expectedArchiveContent.items.size() - expectedArchiveContent.fileCount ) );

    const auto& format = info.format();
    if ( format_has_size_metadata( format ) ) {
        REQUIRE( info.size() == expectedArchiveContent.size );
        REQUIRE( info.packSize() == input.packedSize() );
    }

    REQUIRE_FALSE( info.isMultiVolume() );
    REQUIRE( info.volumesCount() == 1 );

    std::vector< BitArchiveItemInfo > items;
    REQUIRE_NOTHROW( items = info.items() );
    REQUIRE( items.size() == info.itemsCount() );

    const bool archiveStoresPaths = format_has_path_metadata( format );
    const bool fromFilesystem = !info.archivePath().empty();
    std::size_t found_items = 0;
    for ( const auto& expectedItem : expectedArchiveContent.items ) {
        for ( const auto& item : items ) {
            if ( archiveStoresPaths || fromFilesystem ) {
                if ( item.name() != expectedItem.fileInfo.name ) {
                    continue;
                }
                REQUIRE( info.find( item.path() ) != info.cend() );
                REQUIRE( info.contains( item.path() ) );
            }
            REQUIRE( info.isItemEncrypted( item.index() ) == expectedItem.isEncrypted );
            REQUIRE( info.isItemFolder( item.index() ) == ( expectedItem.fileInfo.type == fs::file_type::directory ) );
            require_archive_item( format, item, expectedItem, location );
            found_items++;
            break;
        }
    }
    REQUIRE( items.size() == found_items );
}

void require_filesystem_item( const ExpectedItem& expectedItem, const SourceLocation& location ) {
    INFO( "From " << location.file_name() << ":" << location.line() )
    INFO( "Failed while checking expected item: " << filesystem::to_utf8string( expectedItem.inArchivePath ) )

    const auto fileStatus = fs::symlink_status( expectedItem.inArchivePath );
    REQUIRE( fs::exists( fileStatus ) );
    REQUIRE( fileStatus.type() == expectedItem.fileInfo.type );
    if ( fs::is_regular_file( fileStatus ) ) {
        REQUIRE( crc32( filesystem::load_file( expectedItem.inArchivePath ) ) == expectedItem.fileInfo.crc32 );
    }
#ifndef _WIN32
    if ( fs::is_symlink( fileStatus ) ) {
        const auto symlink = fs::read_symlink( expectedItem.inArchivePath );
        REQUIRE( crc32( symlink.u8string() ) == expectedItem.fileInfo.crc32 );
    }
#endif
    if ( !fs::is_directory( fileStatus ) || fs::is_empty( expectedItem.inArchivePath ) ) {
#ifdef _WIN32
        // Some versions of Windows/MSVC fail to remove read-only files, so we make them writable before removing them.
        fs::perms permissions = fileStatus.permissions();
        if ( ( permissions & fs::perms::owner_write ) == fs::perms::none ) {
            std::error_code error;
            fs::permissions( expectedItem.inArchivePath, permissions | fs::perms::owner_write, error );
            INFO( "Could not set new permissions for the read-only file: " << error.message() )
        }
#endif
        REQUIRE_NOTHROW( fs::remove( expectedItem.inArchivePath ) );
    }
}

} // namespace test
} // namespace bit7z