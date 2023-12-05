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

#include <catch2/catch_get_random_seed.hpp>

#include <internal/stringutil.hpp>

#include "filesystem.hpp"

#include <random>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace filesystem {

const FilesystemItemInfo italy{ BIT7Z_STRING( "italy.svg" ),
                                BIT7Z_STRING( "svg" ),
                                fs::file_type::regular,
                                267,
                                0x2214F4E9,
                                0xE94D };

const FilesystemItemInfo loremIpsum{ BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                                     BIT7Z_STRING( "pdf" ),
                                     fs::file_type::regular,
                                     38170,
                                     0xC0629B8E,
                                     0x4D7D };

const FilesystemItemInfo noext{ BIT7Z_STRING( "noext" ),
                                BIT7Z_STRING( "" ),
                                fs::file_type::regular,
                                34,
                                0xDE536DF9,
                                0xEB1D };

const FilesystemItemInfo dotFolder{ BIT7Z_STRING( "dot.folder" ),
                                    BIT7Z_STRING( "" ),
                                    fs::file_type::directory,
                                    0,
                                    0,
                                    0 };

const FilesystemItemInfo helloJson{ BIT7Z_STRING( "hello.json" ),
                                    BIT7Z_STRING( "json" ),
                                    fs::file_type::regular,
                                    29,
                                    0x794FDB10,
                                    0x226B };

const FilesystemItemInfo emptyFolder{ BIT7Z_STRING( "empty" ),
                                      BIT7Z_STRING( "" ),
                                      fs::file_type::directory,
                                      0,
                                      0,
                                      0 };

const FilesystemItemInfo folder{ BIT7Z_STRING( "folder" ),
                                 BIT7Z_STRING( "" ),
                                 fs::file_type::directory,
                                 0,
                                 0,
                                 0 };

const FilesystemItemInfo subfolder{ BIT7Z_STRING( "subfolder" ),
                                    BIT7Z_STRING( "" ),
                                    fs::file_type::directory,
                                    0,
                                    0,
                                    0 };

const FilesystemItemInfo subfolder2{ BIT7Z_STRING( "subfolder2" ),
                                     BIT7Z_STRING( "" ),
                                     fs::file_type::directory,
                                     0,
                                     0,
                                     0 };

const FilesystemItemInfo homework{ BIT7Z_STRING( "homework.doc" ),
                                   BIT7Z_STRING( "doc" ),
                                   fs::file_type::regular,
                                   31232,
                                   0x1734526A,
                                   0x8E04 };

const FilesystemItemInfo quickBrown{ BIT7Z_STRING( "The quick brown fox.pdf" ),
                                     BIT7Z_STRING( "pdf" ),
                                     fs::file_type::regular,
                                     45933,
                                     0x3A4DCE2A,
                                     0xF922 };

const FilesystemItemInfo frequency{ BIT7Z_STRING( "frequency.xlsx" ),
                                    BIT7Z_STRING( "xlsx" ),
                                    fs::file_type::regular,
                                    20803,
                                    0xA140E471,
                                    0x1F5A };

const FilesystemItemInfo clouds{ BIT7Z_STRING( "clouds.jpg" ),
                                 BIT7Z_STRING( "jpg" ),
                                 fs::file_type::regular,
                                 478883,
                                 0x515D4B66,
                                 0x784E };

const FilesystemItemInfo dir{ BIT7Z_STRING( "dir" ),
                              BIT7Z_STRING( "" ),
                              fs::file_type::directory,
                              0,
                              0,
                              0 };

const FilesystemItemInfo hidden{ BIT7Z_STRING( "hidden" ),
                                 BIT7Z_STRING( "" ),
                                 fs::file_type::regular,
                                 6,
                                 0x885DE9BD,
                                 0 };

const FilesystemItemInfo readOnly{ BIT7Z_STRING( "read_only" ),
                                   BIT7Z_STRING( "" ),
                                   fs::file_type::regular,
                                   9,
                                   0x1EAF9877,
                                   0 };

const FilesystemItemInfo regular{ BIT7Z_STRING( "regular" ),
                                  BIT7Z_STRING( "" ),
                                  fs::file_type::regular,
                                  13,
                                  0x01D7AFB4,
                                  0 };

const FilesystemItemInfo symlink{ BIT7Z_STRING( "symlink" ),
                                  BIT7Z_STRING( "" ),
#ifdef _WIN32
                                  fs::file_type::regular,
#else
                                  fs::file_type::symlink,
#endif
                                  7,
                                  0x4A7103D4,
                                  0 };

auto single_file_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 1, clouds.size, { { clouds, clouds.name, false } } };
    return instance;
}

auto multiple_files_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 2,
                                          italy.size + loremIpsum.size,
                                          { { italy, italy.name, false },
                                            { loremIpsum, loremIpsum.name, false } } };
    return instance;
}

auto multiple_items_content() -> const ArchiveContent& {
    static const ArchiveContent instance{
        8,
        615351,
        { { italy, "italy.svg", false },
          { loremIpsum, "Lorem Ipsum.pdf", false },
          { noext, "noext", false },
          { helloJson, fs::path{ "dot.folder" } / "hello.json", false },
          { dotFolder, "dot.folder", false },
          { emptyFolder, "empty", false },
          { clouds, fs::path{ "folder" } / "clouds.jpg", false },
          { subfolder, fs::path{ "folder" } / "subfolder", false },
          { frequency, fs::path{ "folder" } / "subfolder2" / "frequency.xlsx", false },
          { homework, fs::path{ "folder" } / "subfolder2" / "homework.doc", false },
          { quickBrown, fs::path{ "folder" } / "subfolder2" / "The quick brown fox.pdf", false },
          { subfolder2, fs::path{ "folder" } / "subfolder2", false },
          { folder, "folder", false } }
    };
    return instance;
}

auto encrypted_content() -> const ArchiveContent& {
    static const ArchiveContent instance{
        8,
        615351,
        { { italy, "italy.svg", true },
          { loremIpsum, "Lorem Ipsum.pdf", true },
          { noext, "noext", true },
          { helloJson, fs::path{ "dot.folder" } / "hello.json", true },
          { dotFolder, "dot.folder", false },
          { emptyFolder, "empty", false },
          { clouds, fs::path{ "folder" } / "clouds.jpg", true },
          { subfolder, fs::path{ "folder" } / "subfolder", false },
          { frequency, fs::path{ "folder" } / "subfolder2" / "frequency.xlsx", true },
          { homework, fs::path{ "folder" } / "subfolder2" / "homework.doc", true },
          { quickBrown, fs::path{ "folder" } / "subfolder2" / "The quick brown fox.pdf", true },
          { subfolder2, fs::path{ "folder" } / "subfolder2", false },
          { folder, "folder", false } }
    };
    return instance;
}

auto empty_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 0, 0, {} };
    return instance;
}

auto unicode_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 4,
                                          italy.size + loremIpsum.size + clouds.size + homework.size,
                                          { { italy, BIT7Z_NATIVE_STRING( "юнікод.svg" ), false },
                                            { loremIpsum, BIT7Z_NATIVE_STRING( "ユニコード.pdf" ), false },
                                            { clouds, BIT7Z_NATIVE_STRING( "σύννεφα.jpg" ), false },
                                            { homework, BIT7Z_NATIVE_STRING( "¡Porque sí!.doc" ), false } } };
    return instance;
}

auto file_type_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 4,
                                          hidden.size + readOnly.size + regular.size + symlink.size,
                                          { { dir, "dir", false },
                                            { hidden, "hidden", false },
                                            { readOnly, "read_only", false },
                                            { regular, "regular", false },
                                            { symlink, "symlink", false } } };
    return instance;
}

auto no_path_content() -> const ArchiveContent& {
    static const ArchiveContent instance{ 1,
                                          clouds.size,
                                          { { clouds, "[Content]", false } } };
    return instance;
}

auto flat_items_content() -> const ArchiveContent& {
    static const ArchiveContent instance{
        8,
        615351,
        { { italy, "italy.svg", false },
          { loremIpsum, "Lorem Ipsum.pdf", false },
          { noext, "noext", false },
          { helloJson, "hello.json", false },
          { clouds, "clouds.jpg", false },
          { frequency, "frequency.xlsx", false },
          { homework, "homework.doc", false },
          { quickBrown, "The quick brown fox.pdf", false } }
    };
    return instance;
}

TestDirectory::TestDirectory( const fs::path& testDir ) : mOldCurrentDirectory{ current_dir() } {
    set_current_dir( testDir );
}

TestDirectory::~TestDirectory() {
    set_current_dir( mOldCurrentDirectory );
}

auto random_test_id() -> std::string {
    static constexpr auto hex_digits = "0123456789abcdef";
    static constexpr auto hex_count = 16;

    thread_local static std::default_random_engine random_engine{ Catch::getSeed() };
    thread_local static std::uniform_int_distribution<> distribution{ 0, hex_count - 1 };

    std::string str( 8, '\0' );
    for ( char& str_char : str ) {
        str_char = hex_digits[ distribution( random_engine ) ]; // NOLINT(*-pro-bounds-pointer-arithmetic)
    }
    return str;
}

auto create_temp_directory( const std::string& name ) -> fs::path {
    const auto tempDir = fs::temp_directory_path() / name;
    if ( !fs::exists( tempDir ) ) { // Creating the temp directory since it doesn't exist.
        fs::create_directory( tempDir );
    } else if ( !fs::is_empty( tempDir ) ) { // The temp directory already exists, but it contains some files.
        for ( const auto& entry : fs::directory_iterator( tempDir ) ) {
            fs::remove_all( entry );
        }
    }
    return tempDir;
}

TempDirectory::TempDirectory( const std::string& dirName )
    : mDirectory{ create_temp_directory( dirName + "_" + random_test_id() ) } {}

TempDirectory::~TempDirectory() {
    if ( fs::is_empty( mDirectory ) ) {
        fs::remove( mDirectory );
    }
}

auto TempDirectory::path() const -> const fs::path& {
    return mDirectory;
}

TempDirectory::operator tstring() const {
    return path_to_tstring( mDirectory );
}

TempTestDirectory::TempTestDirectory( const std::string& dirName )
    : TempDirectory{ dirName }, TestDirectory{ path() } {}

} // namespace filesystem
} // namespace test
} // namespace bit7z