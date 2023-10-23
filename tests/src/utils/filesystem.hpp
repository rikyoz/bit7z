/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <bit7z/bitfs.hpp>
#include <bit7z/bittypes.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32
#include <array>
#include <Windows.h>
#elif defined( __APPLE__ )
#include <array>
#include <libproc.h> // for proc_pidpath and PROC_PIDPATHINFO_MAXSIZE
#include <unistd.h> // for getpid
#endif

#include <catch2/catch.hpp>
#include <internal/fs.hpp>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace filesystem {

inline auto exe_path() -> fs::path {
#ifdef _WIN32
    std::array< wchar_t, MAX_PATH > path{ 0 };
    GetModuleFileNameW( nullptr, path.data(), MAX_PATH );
    return fs::path{ path.data() };
#elif defined( __APPLE__ )
    std::array< char, PROC_PIDPATHINFO_MAXSIZE > result{ 0 };
    ssize_t result_size = proc_pidpath( getpid(), result.data(), result.size() );
    return ( result_size > 0 ) ? std::string( result.data(), result_size ) : "";
#else
    std::error_code error;
    const fs::path result = fs::read_symlink( "/proc/self/exe", error );
    return error ? "" : result;
#endif
}

#ifdef BIT7Z_TESTS_DATA_DIR

constexpr auto test_data_dir = BIT7Z_TESTS_DATA_DIR;
constexpr auto test_filesystem_dir = BIT7Z_TESTS_DATA_DIR "/test_filesystem";
constexpr auto test_archives_dir = BIT7Z_TESTS_DATA_DIR "/test_archives";

inline auto current_dir() -> fs::path {
    std::error_code error;
    return fs::current_path( error );
}

inline auto set_current_dir( const fs::path& dir ) -> bool {
    std::error_code error;
    fs::current_path( dir, error );
    return !error;
}

inline auto load_file( fs::path const& inFile ) -> std::vector< bit7z::byte_t > {
    fs::ifstream ifs{ inFile, fs::ifstream::binary };
    if ( !ifs.is_open() ) {
        return {};
    }
    noskipws( ifs ); //no skip spaces!
    auto size = fs::file_size( inFile );
    std::vector< bit7z::byte_t > result( size );
    // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
    ifs.read( reinterpret_cast<char*>( result.data() ), result.cend() - result.cbegin() );
    //note: using basic_ifstream with istreambuf_iterator<std::byte> would be cleaner, but it is 10x slower.
    return result;
}

#define REQUIRE_LOAD_FILE( var, in_file ) \
    const auto (var) = load_file( in_file ); \
    REQUIRE_FALSE( (var).empty() )

#define REQUIRE_OPEN_IFSTREAM( var, in_file ) \
    fs::ifstream (var){ in_file, std::ios::binary }; \
    REQUIRE( (var).is_open() )

struct FilesystemItemInfo {
    const tchar* name; // path inside the test_filesystem folder
    const tchar* ext;
    bool isDir;
    std::size_t size;
    uint32_t crc32;
};

extern const FilesystemItemInfo italy;
extern const FilesystemItemInfo lorem_ipsum;
extern const FilesystemItemInfo noext;
extern const FilesystemItemInfo dot_folder;
extern const FilesystemItemInfo hello_json;
extern const FilesystemItemInfo empty_folder;
extern const FilesystemItemInfo folder;
extern const FilesystemItemInfo subfolder;
extern const FilesystemItemInfo subfolder2;
extern const FilesystemItemInfo homework;
extern const FilesystemItemInfo quick_brown;
extern const FilesystemItemInfo frequency;
extern const FilesystemItemInfo clouds;

struct ArchivedItem {
    const FilesystemItemInfo& fileInfo;
    fs::path inArchivePath;
    bool isEncrypted;
};

struct ArchiveContent {
    std::size_t fileCount;
    std::size_t size;
    std::vector< ArchivedItem > items;
};

auto single_file_content() -> const ArchiveContent&;

auto multiple_files_content() -> const ArchiveContent&;

auto multiple_items_content() -> const ArchiveContent&;

auto encrypted_content() -> const ArchiveContent&;

auto empty_content() -> const ArchiveContent&;

class TestDirectory {
        fs::path mOldCurrentDirectory;
    public:
        explicit TestDirectory( const fs::path& testDir );

        explicit TestDirectory( const TestDirectory& ) = delete;

        explicit TestDirectory( TestDirectory&& ) = delete;

        auto operator=( const TestDirectory& ) -> TestDirectory& = delete;

        auto operator=( TestDirectory&& ) -> TestDirectory& = delete;

        ~TestDirectory();
};

#endif

} // namespace filesystem
} // namespace test
} // namespace bit7z

#endif //FILESYSTEM_HPP
