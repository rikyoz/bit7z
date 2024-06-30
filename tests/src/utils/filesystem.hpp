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

#ifdef _WIN32
#include <array>
#include <windows.h>
#elif defined( __APPLE__ )
#include <array>
#include <libproc.h> // for proc_pidpath and PROC_PIDPATHINFO_MAXSIZE
#include <unistd.h> // for getpid
#elif defined( __FreeBSD__ ) || defined( __DragonFly__ )
constexpr auto self_exe_path = "/proc/curproc/file";
#elif defined( __NetBSD__ )
constexpr auto self_exe_path = "/proc/curproc/exe";
#elif defined( __sun )
constexpr auto self_exe_path = "/proc/self/path/a.out";
#else
constexpr auto self_exe_path = "/proc/self/exe";
#endif

#include <internal/fs.hpp>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace filesystem {

inline auto to_utf8string( const fs::path& path ) -> std::string {
#ifdef __cpp_lib_char8_t
    const auto u8str = path.u8string();
    return std::string{ u8str.cbegin(), u8str.cend() };
#else
    return path.u8string();
#endif
}

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
    const fs::path result = fs::read_symlink( self_exe_path, error );
    return error ? "unknown" : result;
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

#ifdef _WIN32
BIT7Z_NODISCARD auto get_file_comment( const fs::path& filePath ) -> std::wstring;
#endif

inline auto load_file( fs::path const& inFile ) -> buffer_t {
    fs::ifstream ifs{ inFile, fs::ifstream::binary };
    if ( !ifs.is_open() ) {
        return {};
    }
    noskipws( ifs ); //no skip spaces!
    auto size = fs::file_size( inFile );
    buffer_t result( size );
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
    fs::file_type type;
    std::size_t size;
    uint32_t crc32;
    uint16_t crc16;
};

extern const FilesystemItemInfo italy;
extern const FilesystemItemInfo loremIpsum;
extern const FilesystemItemInfo noext;
extern const FilesystemItemInfo dotFolder;
extern const FilesystemItemInfo helloJson;
extern const FilesystemItemInfo emptyFolder;
extern const FilesystemItemInfo folder;
extern const FilesystemItemInfo subfolder;
extern const FilesystemItemInfo subfolder2;
extern const FilesystemItemInfo homework;
extern const FilesystemItemInfo quickBrown;
extern const FilesystemItemInfo frequency;
extern const FilesystemItemInfo clouds;
extern const FilesystemItemInfo dir;
extern const FilesystemItemInfo hidden;
extern const FilesystemItemInfo readOnly;
extern const FilesystemItemInfo regular;
extern const FilesystemItemInfo symlink;

struct ExpectedItem {
    const FilesystemItemInfo& fileInfo;
    fs::path inArchivePath;
    bool isEncrypted;
};

struct ArchiveContent {
    std::size_t fileCount;
    std::size_t size;
    std::vector< ExpectedItem > items;
};

auto single_file_content() -> const ArchiveContent&;

auto multiple_files_content() -> const ArchiveContent&;

auto multiple_items_content() -> const ArchiveContent&;

auto encrypted_content() -> const ArchiveContent&;

auto empty_content() -> const ArchiveContent&;

auto unicode_content() -> const ArchiveContent&;

auto file_type_content() -> const ArchiveContent&;

auto no_path_content() -> const ArchiveContent&;

auto flat_items_content() -> const ArchiveContent&;

class TestDirectory {
    public:
        explicit TestDirectory( const fs::path& testDir );

        explicit TestDirectory( const TestDirectory& ) = delete;

        explicit TestDirectory( TestDirectory&& ) = delete;

        auto operator=( const TestDirectory& ) -> TestDirectory& = delete;

        auto operator=( TestDirectory&& ) -> TestDirectory& = delete;

        ~TestDirectory();

    private:
        fs::path mOldCurrentDirectory;
};

class TempDirectory {
    public:
        explicit TempDirectory( const std::string& dirName );

        explicit TempDirectory( const TempDirectory& ) = delete;

        explicit TempDirectory( TempDirectory&& ) = delete;

        auto operator=( const TempDirectory& ) -> TempDirectory& = delete;

        auto operator=( TempDirectory&& ) -> TempDirectory& = delete;

        ~TempDirectory();

        BIT7Z_NODISCARD
        auto path() const -> const fs::path&;

        operator tstring() const; // NOLINT(*-explicit-constructor, *-explicit-conversions)

    private:
        fs::path mDirectory;
};

inline auto operator<<( std::ostream& stream, const TempDirectory& dir ) -> std::ostream& {
    return stream << to_utf8string( dir.path() );
}

struct TempTestDirectory : TempDirectory, TestDirectory {
    explicit TempTestDirectory( const std::string& dirName );
};

#endif

} // namespace filesystem
} // namespace test
} // namespace bit7z

#endif //FILESYSTEM_HPP
