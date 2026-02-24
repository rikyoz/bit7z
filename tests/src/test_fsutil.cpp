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

#include <catch2/catch.hpp>

#include "utils/filesystem.hpp"
#include "utils/test.hpp"

#include <bit7z/biterror.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/fsutil.hpp>

#include <array>

using namespace bit7z;
using namespace bit7z::filesystem::fsutil;

//-V::2008 (Suppressing warnings for cyclomatic complexity in PVS-Studio)

TEST_CASE( "fsutil: Wildcard matching without special characters", "[fsutil][wildcard_match]" ) {
    REQUIRE( wildcard_match( BIT7Z_STRING( "" ), BIT7Z_STRING( "" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "" ), BIT7Z_STRING( "*" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "" ), BIT7Z_STRING( "?" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a" ), BIT7Z_STRING( "" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a" ), BIT7Z_STRING( "b" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "acb" ), BIT7Z_STRING( "abc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a42b" ), BIT7Z_STRING( "a42b42" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc" ), BIT7Z_STRING( "****a****b****c****" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc" ), BIT7Z_STRING( "*a*b*c*" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abcdef" ), BIT7Z_STRING( "abc*def" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abcdef" ), BIT7Z_STRING( "abc def" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc def" ), BIT7Z_STRING( "abcdef" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc def" ), BIT7Z_STRING( "abc def" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "bLah" ), BIT7Z_STRING( "bLah" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "bLaH" ), BIT7Z_STRING( "bLah" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lorem " ), BIT7Z_STRING( "lorem " ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lorem " ), BIT7Z_STRING( "lorem" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lorem ipsum" ), BIT7Z_STRING( "lorem ipsum" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lorem ipsum" ), BIT7Z_STRING( "lorem-ipsum" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( " lorem" ), BIT7Z_STRING( "lorem " ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( " lorem" ), BIT7Z_STRING( "lorem" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( " lorem" ), BIT7Z_STRING( " lorem" ) ) == true );
}

TEST_CASE( "fsutil: Wildcard matching with question mark special character", "[fsutil][wildcard_match]" ) {
    REQUIRE( wildcard_match( BIT7Z_STRING( "?" ), BIT7Z_STRING( "" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?" ), BIT7Z_STRING( "?" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?" ), BIT7Z_STRING( "*" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "??" ), BIT7Z_STRING( "a" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "??" ), BIT7Z_STRING( "aa" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "??" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "??" ), BIT7Z_STRING( "az" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "??a?b" ), BIT7Z_STRING( "caaab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?a?b" ), BIT7Z_STRING( "caaab" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?z" ), BIT7Z_STRING( "z" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?z" ), BIT7Z_STRING( "az" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?Lah" ), BIT7Z_STRING( "bLaH" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?LaH" ), BIT7Z_STRING( "bLaH" ) ) == true );

    REQUIRE( wildcard_match( BIT7Z_STRING( "a?" ), BIT7Z_STRING( "a" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a?" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a?" ), BIT7Z_STRING( "az" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a?c" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc?" ), BIT7Z_STRING( "abc" ) ) == false );

    REQUIRE( wildcard_match( BIT7Z_STRING( "bL?h" ), BIT7Z_STRING( "bLah" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "bLa?" ), BIT7Z_STRING( "bLaaa" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "bLa?" ), BIT7Z_STRING( "bLah" ) ) == true );
}

TEST_CASE( "fsutil: Wildcard matching with star special character", "[fsutil][wildcard_match]" ) {
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*" ), BIT7Z_STRING( "a*r" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*a*a*a*a*a*a*a*" ), BIT7Z_STRING( "a*a*a*a*a*a*a*a*" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*aar" ), BIT7Z_STRING( "a*ar" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b" ), BIT7Z_STRING( "a*abab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*zz*" ), BIT7Z_STRING( "aAazz" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b" ), BIT7Z_STRING( "acb" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b" ), BIT7Z_STRING( "abc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "acb" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "acbd" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "acdb" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "ac" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*b*" ), BIT7Z_STRING( "bc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "a*zz*" ), BIT7Z_STRING( "aaazz" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "ab*d" ), BIT7Z_STRING( "abc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc*" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc*abc*abc*abc" ), BIT7Z_STRING( "abc*abcd*abc*abcd" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc*abc*abc*abcd" ), BIT7Z_STRING( "abc*abcd*abc*abcd" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "abc**" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lo*ips*" ), BIT7Z_STRING( "lorem ipsum" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lo*ips*" ), BIT7Z_STRING( "lorem-ipsum" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lo*ips*" ), BIT7Z_STRING( "loreM ipsum" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "lo*Ips*" ), BIT7Z_STRING( "loreM ipsum" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xxxx*yzz*aaaaa" ), BIT7Z_STRING( "xxxx*yyyyyyyzz*a" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xxx*yzz*a" ), BIT7Z_STRING( "xxxx*yyyyyyyzz*a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xxxx*yzz*aaaaa" ), BIT7Z_STRING( "xxxxyyyyyyyzza" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xxxx*yzz*a" ), BIT7Z_STRING( "xxxxyyyyyyyzza" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xy*z*xyz" ), BIT7Z_STRING( "xyxyxyzyxyz" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xy*xyz" ), BIT7Z_STRING( "xyxyxyxyz" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "XY*Z*XYz" ), BIT7Z_STRING( "XYXYXYZYXYz" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "xy*xyz" ), BIT7Z_STRING( "xyxyxyxyz" ) ) == true );

    REQUIRE( wildcard_match( BIT7Z_STRING( "*" ), BIT7Z_STRING( "" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*" ), BIT7Z_STRING( "*" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "**" ), BIT7Z_STRING( "" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "**" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "***a*b*c***" ), BIT7Z_STRING( "*abc*" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "********a********b********c********" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "********a********b********b********" ), BIT7Z_STRING( "abc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "**a**b**c" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "**a**b**c**" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "**b**c" ), BIT7Z_STRING( "bc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*" ), BIT7Z_STRING( "b" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a**b***c****" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*a*a*a*a*a*a*a*" ), BIT7Z_STRING( "aaaaaaaaaaaa" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*a*a*a*a*a*a*a*" ), BIT7Z_STRING( "aaaaaaa" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*b*c" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a*b*c*" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*aabbaa*a*" ), BIT7Z_STRING( "aaabbaabbaab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*abac*" ), BIT7Z_STRING( "ababac" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*abac*" ), BIT7Z_STRING( "ababac" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*abc*" ), BIT7Z_STRING( "aBc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*aBc*" ), BIT7Z_STRING( "aBc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*" ), BIT7Z_STRING( "a" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*" ), BIT7Z_STRING( "ba" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*" ), BIT7Z_STRING( "" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*c" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*c" ), BIT7Z_STRING( "bac" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*c" ), BIT7Z_STRING( "bc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*c" ), BIT7Z_STRING( "bd" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*b*c" ), BIT7Z_STRING( "db" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*ccd" ), BIT7Z_STRING( "abcccd" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*ipsum*ips*" ), BIT7Z_STRING( "lorem ipsum ipsu" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*ips*" ), BIT7Z_STRING( "lorem ipsum" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*orem*IPS*" ), BIT7Z_STRING( "lorem IPSUM" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*orem*IPS" ), BIT7Z_STRING( "lorem IPSUM" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*Abac*" ), BIT7Z_STRING( "abAbac" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*Abac*" ), BIT7Z_STRING( "abAbac" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*ORE*" ), BIT7Z_STRING( "lOREm" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*42*21" ), BIT7Z_STRING( "a42b42" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*42*42*" ), BIT7Z_STRING( "a42b42" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*42*23" ), BIT7Z_STRING( "A42b42" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*42*42*" ), BIT7Z_STRING( "a42B42" ) ) == true );
}

TEST_CASE( "fsutil: Wildcard matching with both question mark and star", "[fsutil][wildcard_match]" ) {
    REQUIRE( wildcard_match( BIT7Z_STRING( "*a?b" ), BIT7Z_STRING( "caaab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*?" ), BIT7Z_STRING( "a" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*?" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*?" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "*?*?*" ), BIT7Z_STRING( "ab" ) ) == true );

    REQUIRE( wildcard_match( BIT7Z_STRING( "?*?" ), BIT7Z_STRING( "ab" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?**?*?" ), BIT7Z_STRING( "abc" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?**?*&?" ), BIT7Z_STRING( "abc" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?b*??" ), BIT7Z_STRING( "abcd" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?a*??" ), BIT7Z_STRING( "abcd" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?**?c?" ), BIT7Z_STRING( "abcd" ) ) == true );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?**?d?" ), BIT7Z_STRING( "abcd" ) ) == false );
    REQUIRE( wildcard_match( BIT7Z_STRING( "?*b*?*d*?" ), BIT7Z_STRING( "abcde" ) ) == true );
}

#ifdef BIT7Z_TESTS_FILESYSTEM

struct TestItem {
    fs::path path;
    fs::path inArchivePath;
};

TEST_CASE( "fsutil: In-archive path computation", "[fsutil][in_archive_path]" ) {
    using namespace test::filesystem;

    const fs::path oldCurrentDir = current_dir();
    REQUIRE ( set_current_dir( test_filesystem_dir ) );

    // Note: since we are using the function fs::absolute(...), the content of this vector depends on the current
    //       directory, hence we must declare the vector inside the test case and not outside.
    const std::array< TestItem, 36 > testItems{ {
        { ".",                                                "" },
        { "./",                                               "" },
        { "..",                                               "" },
        { "../",                                              "" },
        { "italy.svg",                                        "italy.svg" },
        { "folder",                                           "folder" },
        { "folder/",                                          "folder/" },
        { "folder/..",                                        "" },
        { "folder/../",                                       "" },
        { "folder/.",                                         "folder" },
        { "folder/./",                                        "folder" },
        { "folder/clouds.jpg",                                "folder/clouds.jpg" },
        { "folder/subfolder2",                                "folder/subfolder2" },
        { "folder/subfolder2/",                               "folder/subfolder2/" },
        { "folder/subfolder2/..",                             "folder" },
        { "folder/subfolder2/../",                            "folder" },
        { "folder/subfolder2/.",                              "subfolder2" },
        { "folder/subfolder2/./",                             "subfolder2" },
        { "folder/subfolder2/homework.doc",                   "folder/subfolder2/homework.doc" },
        { "./italy.svg",                                      "italy.svg" },
        { "./folder",                                         "folder" },
        { "./folder/",                                        "folder" },
        { "./folder/clouds.jpg",                              "clouds.jpg" },
        { "./folder/subfolder2",                              "subfolder2" },
        { "./folder/subfolder2/homework.doc",                 "homework.doc" },
        { "./../test_filesystem/",                            "test_filesystem" },
        { "./../test_filesystem/folder/",                     "folder" },
        { fs::absolute( "." ),                                "test_filesystem" },
        { fs::absolute( "../" ),                              fs::path{ test_data_dir }.filename() },
        { fs::absolute( "./italy.svg" ),                      "italy.svg" },
        { fs::absolute( "./folder" ),                         "folder" },
        { fs::absolute( "./folder/" ),                        "folder" },
        { fs::absolute( "./folder/clouds.jpg" ),              "clouds.jpg" },
        { fs::absolute( "./folder/subfolder2" ),              "subfolder2" },
        { fs::absolute( "./folder/subfolder2/" ),             "subfolder2" },
        { fs::absolute( "./folder/subfolder2/homework.doc" ), "homework.doc" }
    } };

    for ( const auto& testItem : testItems ) {
        DYNAMIC_SECTION( "Path: " << testItem.path ) {
            REQUIRE( in_archive_path( testItem.path ) == testItem.inArchivePath );
        }
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

#endif

#if defined( _WIN32 ) && defined( BIT7Z_AUTO_PREFIX_LONG_PATHS )
TEST_CASE( "fsutil: Format long Windows paths", "[fsutil][format_long_path]" ) {
    SECTION( "Short paths should not be formatted" ) {
        REQUIRE_FALSE( should_format_long_path( L"short_path\\file.txt" ) );
        REQUIRE_FALSE( should_format_long_path( L"C:\\short_path\\file.txt" ) );
        REQUIRE_FALSE( should_format_long_path( L"\\\\server\\share\\file.txt" ) );
    }

    constexpr auto kLongPathPrefix = LR"(\\?\)";
    constexpr auto kVeryLongPath = LR"(very\long\dummy\path\)"
                                   LR"(ABCDEFGHIJKLMNOPQRSTUVWXYZ\abcdefghijklmnopqrstuvwxyz\0123456789\)"
                                   LR"(Lorem ipsum dolor sit amet\consectetur adipiscing elit\)"
                                   LR"(Mauris ac leo dui\Morbi non elit lacus\)"
                                   LR"(Ut ullamcorper sapien eget commodo eleifend\Curabitur varius magna sit\)"
                                   LR"(Hello_World.txt)";

    // Note: the paths passed to the long path formatting functions are absolute.
    SECTION( "Long relative paths should not be formatted" ) {
        REQUIRE_FALSE( should_format_long_path( kVeryLongPath ) );
    }

    SECTION( "Long absolute paths should be formatted" ) {
        // C:\<long path>
        const auto kVeryLongAbsolutePath = std::wstring{ L"C:\\" } + kVeryLongPath;
        // \\?\C:\<long path>
        const auto kPrefixedVeryLongPath = kLongPathPrefix + kVeryLongAbsolutePath;

        REQUIRE_FALSE( should_format_long_path( kPrefixedVeryLongPath ) );

#ifdef BIT7Z_USE_STANDARD_FILESYSTEM
        REQUIRE( should_format_long_path( kVeryLongAbsolutePath ) );
        REQUIRE( format_long_path( kVeryLongAbsolutePath ).native() == kPrefixedVeryLongPath );
#else
        // The GHC library already formats long paths!
        REQUIRE_FALSE( should_format_long_path( kVeryLongAbsolutePath ) );
#endif
    }

    SECTION( "Long UNC paths should be formatted" ) {
        // \\server\share
        const auto kVeryLongUncPath = std::wstring{ L"\\\\" } + kVeryLongPath;
        // \\?\UNC\server\share
        const auto kPrefixedVeryLongUncPath = std::wstring{ kLongPathPrefix } + L"UNC\\" + kVeryLongPath;

        REQUIRE_FALSE( should_format_long_path( kPrefixedVeryLongUncPath ) );

#ifdef BIT7Z_USE_STANDARD_FILESYSTEM
        REQUIRE( should_format_long_path( kVeryLongUncPath ) );
        REQUIRE( format_long_path( kVeryLongUncPath ).native() == kPrefixedVeryLongUncPath );
#else
        REQUIRE_FALSE( should_format_long_path( kVeryLongUncPath ) );
#endif
    }
}
#endif

namespace {
BIT7Z_ALWAYS_INLINE
auto quoted( const fs::path& path ) -> std::string {
    return bit7z::test::quoted( path.c_str() );
}
} // namespace

using bit7z::test::quoted;

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
namespace {
struct SanitizationTest {
    const native_char* path;
    const native_char* expectedPath;
};
} // namespace

TEST_CASE( "fsutil: Sanitizing Windows paths", "[fsutil][sanitize_path]" ) {
    SECTION( "Empty and whitespace inputs" ) {
        const auto test = GENERATE(
            SanitizationTest{ L"", L"" },
            SanitizationTest{ L" ", L"_" },
            SanitizationTest{ L"                  ", L"_" },
            SanitizationTest{ L"\t", L"_" },  // Tab character
            SanitizationTest{ L"   \t   ", L"   _   " }
        );

        DYNAMIC_SECTION( quoted( test.path ) << " -> " << quoted( test.expectedPath ) ) {
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );
        }
    }

    SECTION( "Control characters (ASCII 0-31) replacement" ) {
        const auto controlChar = GENERATE( range( 1, 32 ) );  // Skip null

        std::wstring input = L"test";
        input += static_cast< wchar_t >( controlChar );
        input += L"file.txt";

        INFO( "Control char: " << controlChar );
        const auto result = sanitize_path( input );
        REQUIRE( result == L"test_file.txt" );
    }

    SECTION( "Reserved characters replacement" ) {
        const auto test = GENERATE(
            SanitizationTest{ L"hello world.txt", L"hello world.txt" },
            SanitizationTest{ L"hello\tworld.txt", L"hello_world.txt" },
            SanitizationTest{ L"hello\t\rworld.txt", L"hello__world.txt" },
            SanitizationTest{ L"hello?world<", L"hello_world_" },
            SanitizationTest{ L":hello world|", L"_hello world_" },
            SanitizationTest{ L"hello?world<.txt", L"hello_world_.txt" },
            SanitizationTest{ L":hello world|.txt", L"_hello world_.txt" },
            SanitizationTest{ L"file<>:\"|?*.txt", L"file_______.txt" }
        );

        DYNAMIC_SECTION( quoted( test.path ) << " -> " << quoted( test.expectedPath ) ) {
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );
        }
    }

    SECTION( "COM and LPT reserved names and all their case variations" ) {
        const auto *const reservedName = GENERATE( as< const wchar_t* >(), L"COM", L"LPT" );
        const auto caseVariation = GENERATE_REF( test::casePermutations( reservedName ) );
        const auto digit = GENERATE( range( 0, 10 ) );

        const auto input = caseVariation + std::to_wstring( digit );
        const auto expected = L"_" + input;

        DYNAMIC_SECTION( quoted( input ) << " -> " << quoted( expected ) ) {
            REQUIRE( sanitize_path( input ) == expected );
        }
    }

    SECTION( "Superscript digits in COM/LPT reserved names" ) {
        const auto *const reservedName = GENERATE( as< const wchar_t* >(), L"COM", L"LPT" );
        const auto caseVariation = GENERATE_REF( test::casePermutations( reservedName ) );
        const auto *const superscriptDigit = GENERATE( as< const wchar_t* >(), L"¹", L"²", L"³" );

        const auto input = caseVariation + superscriptDigit;
        const auto expected = L"_" + input;

        DYNAMIC_SECTION( quoted( input ) << " -> " << quoted( expected ) ) {
            REQUIRE( sanitize_path( input ) == expected );
        }
    }

    SECTION( "Other reserved names" ) {
        const auto *const reservedName = GENERATE( as< const wchar_t* >(),
            L"CON",
            L"PRN",
            L"AUX",
            L"NUL",
            L"CONIN$",
            L"CONOUT$"
        );
        const auto caseVariation = GENERATE_REF( test::casePermutations( reservedName ) );

        const auto expected = L"_" + caseVariation;

        DYNAMIC_SECTION( quoted( caseVariation ) << " -> " << quoted( expected ) ) {
            REQUIRE( sanitize_path( caseVariation ) == expected );
            REQUIRE( sanitize_path( L" " + caseVariation ) == L" " + expected );
            REQUIRE( sanitize_path( L"      " + caseVariation ) == L"      " + expected );
        }
    }

    SECTION( "Names that look like reserved but aren't" ) {
        const auto *const testPath = GENERATE( as< const wchar_t* >(),
            L"COM0.txt",  // Has extension.
            L"LPT9.txt",
            L"COM42",     // Extra letter.
            L"LPT42",
            L"COMM",
            L"LPTT",
            L"CONO",
            L"PRNG",
            L"AUXI",
            L"NULL",
            L"COM",       // Missing final digit from reserved name.
            L"LPT",
            L"COM⁰",      // Superscript digits other than ¹, ², and ³ are valid names.
            L"LPT⁰",
            L"COM⁴",
            L"LPT⁴",
            L"COM⁵",
            L"LPT⁵",
            L"COM⁶",
            L"LPT⁶",
            L"COM⁷",
            L"LPT⁷",
            L"COM⁸",
            L"LPT⁸",
            L"COM⁹",
            L"LPT⁹",
            L"CON0"       // CON reserved name doesn't take digits.
        );

        DYNAMIC_SECTION( "The path " << quoted( testPath ) << " should NOT be treated as reserved" ) {
            REQUIRE( sanitize_path( testPath ) == testPath );
        }
    }

    const auto makeSpaces = []( int count ) -> std::wstring { return std::wstring( count, L' ' ); };

    SECTION( "Leading slashes are stripped, spaces are converted to underscores" ) {
        const auto slashPattern = GENERATE( as< std::wstring >(),
            L"/",
            L"\\",
            L"//",
            L"\\\\",
            L"//////",
            L"\\\\\\\\\\\\",
            L"/\\",
            L"\\/",
            L"//\\\\",
            L"\\\\//",
            L"////////\\\\\\\\",
            L"\\\\\\\\////////",
            L"/\\/\\/\\/\\/\\/\\",
            L"\\/\\/\\/\\/\\/\\/"
        );

        DYNAMIC_SECTION( "Basic slash pattern " << quoted( slashPattern ) ) {
            SECTION( "With no spaces" ) {
                // Component with only slashes becomes "_" (prevents creating empty paths).
                REQUIRE( sanitize_path( slashPattern ) == L"_" );

                // Leading slashes are stripped.
                REQUIRE( sanitize_path( slashPattern + L"abc" ) == L"abc" );

                // Trailing slashes create an empty component (e.g., "/abc" -> components: ["abc", ""] -> "abc\".
                REQUIRE( sanitize_path( L"abc" + slashPattern ) == L"abc\\" );

                // Leading slashes are stripped, trailing ones create an empty component -> ["abc", ""] -> "abc\".
                REQUIRE( sanitize_path( slashPattern + L"abc" + slashPattern ) == L"abc\\" );
            }

            SECTION( "With spaces" ) {
                const auto spaceCount = GENERATE( 1, 6 );

                INFO( spaceCount << " spaces" );
                const auto spaces = makeSpaces( spaceCount );

                // Leading spaces + slashes -> components: [spaces, ""] -> "_\".
                REQUIRE( sanitize_path( spaces + slashPattern ) == L"_\\" );

                // Two spaces-only components, both become "_".
                REQUIRE( sanitize_path( spaces + slashPattern + spaces ) == L"_\\_" );

                // Leading spaces in filenames are preserved (Windows allows this).
                REQUIRE( sanitize_path( spaces + L"abc" ) == spaces + L"abc" );

                // One spaces-only component -> "_", separator(s), filename with leading spaces (kept).
                REQUIRE( sanitize_path( spaces + slashPattern + spaces + L"abc" ) == L"_\\" + spaces + L"abc" );

                // Leading slash(es) stripped, filename with leading spaces preserved.
                REQUIRE( sanitize_path( slashPattern + spaces + L"abc" ) == spaces + L"abc" );

                // Leading slash(es) stripped, space-only component becomes "_".
                REQUIRE( sanitize_path( slashPattern + spaces ) == L"_" );

                // Trailing spaces in component are preserved (OS will ignore them).
                REQUIRE( sanitize_path( slashPattern + L"abc" + spaces ) == L"abc"  + spaces );

                // Trailing spaces-only component becomes "_".
                REQUIRE( sanitize_path( slashPattern + L"abc" + slashPattern + spaces ) == L"abc\\_" );

                if ( slashPattern.size() > 1 ) {
                    const auto midpoint = slashPattern.size() / 2;

                    const auto left = slashPattern.substr( 0, midpoint );
                    const auto right = slashPattern.substr( midpoint );

                    // All slashes and spaces combinations turn into "_\"
                    const auto spacedSlashes = left + spaces + right;

                    REQUIRE( sanitize_path( spacedSlashes ) == L"_\\" );
                    REQUIRE( sanitize_path( spacedSlashes + L"abc" ) == L"_\\abc" );
                    REQUIRE( sanitize_path( spacedSlashes + L"abc" + spacedSlashes ) == L"_\\abc\\_\\" );
                    REQUIRE( sanitize_path( spacedSlashes + spaces ) == L"_\\_" );

                    // Leading slashes (without spaces) are stripped.
                    REQUIRE( sanitize_path( slashPattern + L"abc" + spacedSlashes ) == L"abc\\_\\" );

                    // Spaces-only components turn into "_", combinations of slashes and spaces turn into "_\".
                    REQUIRE( sanitize_path( spaces + spacedSlashes ) == L"_\\_\\" );
                    REQUIRE( sanitize_path( spaces + spacedSlashes + L"abc" ) == L"_\\_\\abc" );
                    REQUIRE( sanitize_path( spaces + spacedSlashes + spaces ) == L"_\\_\\_" );

                    // Leading spaces preserved in filename component
                    REQUIRE( sanitize_path( spaces + L"abc" + spacedSlashes ) == spaces + L"abc\\_\\" );
                    REQUIRE( sanitize_path( spaces + spacedSlashes + spaces + L"abc" ) == L"_\\_\\" + spaces + L"abc" );
                }
            }
        }
    }

    SECTION( "Complex paths with drive letters" ) {
        const auto test = GENERATE(
            // Drive-relative paths
            SanitizationTest{ L"C:", L"C_" },
            SanitizationTest{ L"C:file.txt", L"C_file.txt" },
            SanitizationTest{ L"C:abc/def/", L"C_abc/def/" },
            SanitizationTest{ L"C:../abc/def", L"C_../abc/def"},
            SanitizationTest{ L"C:Test/COM0/hello?world<.txt", L"C_Test\\_COM0\\hello_world_.txt" },
            // Absolute paths
            SanitizationTest{ L"C:/", L"C_\\" },
            SanitizationTest{ L"C://", L"C_\\" },
            SanitizationTest{ L"C:/\\", L"C_\\" },
            SanitizationTest{ L"C:\\", L"C_\\" },
            SanitizationTest{ L"C:\\\\", L"C_\\" },
            SanitizationTest{ L"C:\\/", L"C_\\" },
            SanitizationTest{ L"C:/abc", L"C_\\abc" },
            SanitizationTest{ L"C://abc", L"C_\\abc" },
            SanitizationTest{ L"C:/\\abc", L"C_\\abc" },
            SanitizationTest{ L"C:\\abc", L"C_\\abc" },
            SanitizationTest{ L"C:\\\\abc", L"C_\\abc" },
            SanitizationTest{ L"C:\\/abc", L"C_\\abc" },
            SanitizationTest{ L"C:/abc/", L"C_\\abc\\" },
            SanitizationTest{ L"C:\\abc\\", L"C_\\abc\\" },
            SanitizationTest{ L"C:\\abc/", L"C_\\abc\\" },
            SanitizationTest{ L"C:/abc/NUL/def", L"C_\\abc\\_NUL\\def" },
            SanitizationTest{ L"C:\\abc\\NUL\\def", L"C_\\abc\\_NUL\\def" },
            SanitizationTest{ L"C:/Test/COM0/hello?world<.txt", L"C_\\Test\\_COM0\\hello_world_.txt" },
            SanitizationTest{ L"C:\\Test\\COM0\\hello?world<.txt", L"C_\\Test\\_COM0\\hello_world_.txt" },
            SanitizationTest{ L"C:/Test/:hello world|/LPT5", L"C_\\Test\\_hello world_\\_LPT5" },
            SanitizationTest{ L"C:\\Test\\:hello world|\\LPT5", L"C_\\Test\\_hello world_\\_LPT5" },
            SanitizationTest{ L"C:/Test/<\"?*>:|/LPT5", L"C_\\Test\\_______\\_LPT5" },
            SanitizationTest{ L"C:\\Test\\<\"?*>:|\\LPT5", L"C_\\Test\\_______\\_LPT5" }
        );

        DYNAMIC_SECTION( quoted( test.path ) << " -> " << quoted( test.expectedPath ) ) {
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );
        }
    }

    SECTION( "Relative paths with reserved names or with invalid characters" ) {
        const auto test = GENERATE(
            SanitizationTest{ L"Test/COM0/hello?world<.txt", L"Test\\_COM0\\hello_world_.txt" },
            SanitizationTest{ L"Test/:hello world|/LPT5", L"Test\\_hello world_\\_LPT5" },
            SanitizationTest{ L"Test/<\"?*>:|/LPT5", L"Test\\_______\\_LPT5" },
            SanitizationTest{ L"../:hello world|/LPT5", L"..\\_hello world_\\_LPT5" },
            SanitizationTest{ L"../COM0/hello?world<.txt", L"..\\_COM0\\hello_world_.txt" },
            SanitizationTest{ L"../<\"?*>:|/LPT5", L"..\\_______\\_LPT5" },
            SanitizationTest{ L"./:hello world|/LPT5", L".\\_hello world_\\_LPT5" },
            SanitizationTest{ L"./COM0/hello?world<.txt", L".\\_COM0\\hello_world_.txt" },
            SanitizationTest{ L"./<\"?*>:|/LPT5", L".\\_______\\_LPT5" },
            SanitizationTest{ L":hello world|/LPT5", L"_hello world_\\_LPT5" },
            SanitizationTest{ L"COM0/hello?world<.txt", L"_COM0\\hello_world_.txt" },
            SanitizationTest{ L"<\"?*>:|/LPT5", L"_______\\_LPT5" }
        );

        DYNAMIC_SECTION( quoted( test.path ) << " -> " << quoted( test.expectedPath ) ) {
            // Test forward slash version
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );

            // Test backslash version
            std::wstring backslash{ test.path };
            std::replace( backslash.begin(), backslash.end(), L'/', L'\\' );
            REQUIRE( sanitize_path( backslash ) == test.expectedPath );
        }
    }

    SECTION( "Non-reserved unicode filenames" ) {
        const auto *const testPath = GENERATE( as< const wchar_t* >(),
            L"文件.txt",
            L"файл.txt",
            L"αβγ.txt",
            L"emoji😀.txt",
            L"COM1文件"
        );

        DYNAMIC_SECTION( quoted( testPath ) ) {
            REQUIRE( sanitize_path( testPath ) == testPath );
        }
    }

    SECTION( "Edge cases" ) {
        // We do not sanitize trailing dots, as are already ignored by the C++ output file streams.
        const auto *const testPath = GENERATE( as< const wchar_t* >(),
            L"abc.",
            L"abc..",
            L"abc...",
            L"abc. . .",
            L".",
            L"..",
            L"...",
            L". . .",
            L"a",
            L"_"
        );

        DYNAMIC_SECTION( quoted( testPath ) ) {
            REQUIRE( sanitize_path( testPath ) == testPath );
        }
    }

    SECTION( "Normal paths without invalid characters or invalid names" ) {
        const auto test = GENERATE(
            SanitizationTest{ L"lorem ipsum.txt", L"lorem ipsum.txt" },
            SanitizationTest{ L"lorem     ipsum.txt", L"lorem     ipsum.txt" },
            SanitizationTest{ L"lorem/ipsum.txt", L"lorem/ipsum.txt" },
            SanitizationTest{ L"lorem\\ipsum.txt", L"lorem\\ipsum.txt" }
        );

        DYNAMIC_SECTION( quoted( test.path ) ) {
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );
        }
    }

    SECTION( "UNC paths" ) {
        const auto test = GENERATE(
            SanitizationTest{ L"\\\\abc\\def", L"abc\\def" },
            SanitizationTest{ L"\\\\?\\", L"_\\" },
            SanitizationTest{ L"\\\\?\\abc\\def", L"_\\abc\\def" },
            SanitizationTest{ L"\\\\?\\UNC\\server\\share", L"_\\UNC\\server\\share" }
        );

        DYNAMIC_SECTION( quoted( test.path ) ) {
            REQUIRE( sanitize_path( test.path ) == test.expectedPath );
        }
    }
}
#endif

namespace {
// Helper function to test base path normalization of SafeOutPathBuilder.
auto normalizedBasePath( const tstring& basePath ) -> fs::path {
    return SafeOutPathBuilder{ basePath }.basePath();
}
} // namespace

TEST_CASE( "fsutil: Base path normalization", "[fsutil][SafeOutPathBuilder]" ) {
    SECTION( "Path separators normalization" ) {
#ifdef _WIN32
        const auto separators = GENERATE( as< tstring >(),
            BIT7Z_STRING( "/" ),
            BIT7Z_STRING( "//" ),
            BIT7Z_STRING( "/////" ),
            BIT7Z_STRING( "\\" ),
            BIT7Z_STRING( "\\\\" ),
            BIT7Z_STRING( "\\\\\\" ),
            BIT7Z_STRING( "/\\" ),
            BIT7Z_STRING( "//\\\\" ),
            BIT7Z_STRING( "\\/" ),
            BIT7Z_STRING( "\\\\//" )
        );
#else
        const auto separators = GENERATE( as< tstring >(),
            BIT7Z_STRING( "/" ),
            BIT7Z_STRING( "//" ),
            BIT7Z_STRING( "/////" )
        );
#endif

        SECTION( "Trailing separators are normalized" ) {
            SECTION( "Relative paths" ) {
                const auto expectedPath = fs::absolute( "out/" );
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "out" ) + separators ) == expectedPath );
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "out/dir" ) + separators ) == expectedPath / "dir" / "" );
            }

#ifdef _WIN32
            SECTION( "Drive-relative paths" ) {
                // Paths starting with "/" are relative to current drive's root on Windows
#else
            SECTION( "Absolute paths" ) {
#endif
                const auto expectedPath = fs::absolute( "/out/" );
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "/out" ) + separators ) == expectedPath );
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "/out/dir" ) + separators ) == expectedPath / "dir" / "" );
            }

#ifdef _WIN32
            SECTION( "Windows drive paths" ) {
                const fs::path expectedPath = L"D:\\";
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "D:" ) + separators ) == expectedPath );
                REQUIRE( normalizedBasePath( BIT7Z_STRING( "D:/out" ) + separators ) == expectedPath / "out" / "" );
            }
#endif
        }

        SECTION( "Internal separators are normalized" ) {
            const auto pathSuffix = separators + BIT7Z_STRING( "dir/" );
            REQUIRE( normalizedBasePath( BIT7Z_STRING( "out" ) + pathSuffix ) == fs::absolute( "out/dir/" ) );
            REQUIRE( normalizedBasePath( BIT7Z_STRING( "/out" ) + pathSuffix ) == fs::absolute( "/out/dir/" ) );
#ifdef _WIN32
            REQUIRE( normalizedBasePath( BIT7Z_STRING( "D:/out" ) + pathSuffix ) == fs::absolute( "D:/out/dir/" ) );
#endif
        }
    }

    SECTION( "Empty and dot paths resolve to current directory" ) {
        // Using fs::equivalent to handle trailing separator differences across implementations.
        const auto currentPath = fs::current_path();
        REQUIRE( fs::equivalent( normalizedBasePath( BIT7Z_STRING( "" ) ), currentPath ) );
        REQUIRE( fs::equivalent( normalizedBasePath( BIT7Z_STRING( "." ) ), currentPath ) );
        REQUIRE( fs::equivalent( normalizedBasePath( BIT7Z_STRING( "./" ) ), currentPath ) );
    }

#ifdef _WIN32
    SECTION( "UNC paths should be preserved" ) {
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\server\\path" ) ) == L"\\\\server\\path" );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\?\\abc\\def" ) ) == L"\\\\?\\abc\\def" );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\?\\UNC\\server\\path" ) ) == L"\\\\?\\UNC\\server\\path" );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\?\\UNC\\\\server\\path" ) ) == L"\\\\?\\UNC\\server\\path" );
    }
#endif

#if defined( _WIN32 ) && !defined( GHC_FILESYSTEM_VERSION )
    SECTION( "Invalid base paths (UNC paths without server)" ) {
        // "//" and "\\\\" are invalid UNC paths on Windows (at least for MSVC's std::filesystem).
        REQUIRE_THROWS( normalizedBasePath( BIT7Z_STRING( "//" ) ) );
        REQUIRE_THROWS( normalizedBasePath( BIT7Z_STRING( "///////" ) ) );
        REQUIRE_THROWS( normalizedBasePath( BIT7Z_STRING( "\\\\" ) ) );
        REQUIRE_THROWS( normalizedBasePath( BIT7Z_STRING( "\\\\\\\\\\" ) ) );
    }
#else
    SECTION( "Unix absolute or Windows drive root base paths" ) {
        // ghc::filesystem treats these paths as root paths rather than UNC paths.
        const auto expectedPath = fs::absolute( "/" );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "//" ) ) == expectedPath );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "///////" ) ) == expectedPath );
#ifdef _WIN32
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\" ) ) == expectedPath );
        REQUIRE( normalizedBasePath( BIT7Z_STRING( "\\\\\\\\\\" ) ) == expectedPath );
#endif
    }
#endif
}

TEST_CASE( "fsutil: Basic path building tests", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "./" ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir"),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        // Note: these paths have different meaning on Windows and on Unix.
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

    const auto testItemPath = GENERATE( as< fs::path >(),
        BIT7Z_NATIVE_STRING( "abc" ),
        BIT7Z_NATIVE_STRING( "abc/" ),
        BIT7Z_NATIVE_STRING( "folder/subfolder" ),
        BIT7Z_NATIVE_STRING( "folder/subfolder/" ),
        BIT7Z_NATIVE_STRING( "file.txt" ),
        BIT7Z_NATIVE_STRING( "dir/file.txt" ),
        BIT7Z_NATIVE_STRING( "dir/subdir/file.txt" )
    );

    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath );
    }
}

#ifdef BIT7Z_PATH_SANITIZATION
#   ifdef _WIN32
TEST_CASE( "fsutil: Path building with invalid Windows item paths", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir" ),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

    const auto testItemPath = GENERATE( as< fs::path >(),
        L"/",
        L"/abc",
        L"/abc/def",
        L"\\\\server", // UNC Path.
        L"\\\\server\\share",
        L"\\\\?\\", // Prefixed long paths.
        L"\\\\?\\abc\\def",
        L"\\\\?\\UNC\\server\\share",
        L"C:abc", // Drive-relative paths (sanitized to C_<rest of the path>).
        L"C:..",
        L"C:../abc/def",
        L"Test/COM0/hello?world<.txt", // Path with invalid file names or characters.
        L"C:/abc", // Absolute paths.
        L"C:/abc/def"
    );

    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / sanitize_path( testItemPath ) );
    }
}
#   else
TEST_CASE( "fsutil: Path building with absolute paths", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< std::string >(),
        "",
        ".",
        "/",
        "/out",
        "/out/dir",
        "out",
        "out/dir"
    );

    const auto testItemPath = GENERATE( as< fs::path >(),
        "/",
        "/abc",
        "/abc/def"
    );

    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath.relative_path() );
    }
}
#   endif


TEST_CASE( "fsutil: Path building with paths with dot components", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir" ),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

#ifdef _WIN32
    const auto testItemPath = GENERATE( as< fs::path >(),
        L".",
        L"/.",
        L"./",
        L"/./",
        L"\\\\.\\"
    );
#else
    const auto testItemPath = GENERATE( as< fs::path >(),
        L".",
        L"/.",
        L"./",
        L"/./"
    );
#endif

    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / "" );
        REQUIRE( builder.buildPath( testItemPath / "abc" ) == builder.basePath() / "abc" );
        REQUIRE( builder.buildPath( testItemPath / "subdir/file.txt" ) == builder.basePath() / "subdir/file.txt" );
    }
}

TEST_CASE( "fsutil: Path building with an empty path should return the base path", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir" ),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

    DYNAMIC_SECTION( "empty path item inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( "" ) == builder.basePath() );
    }
}
#else
TEST_CASE( "fsutil: Path building with absolute paths should fail", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir" ),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        // Note: the following paths have different meaning on Windows and on Unix.
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

#   ifdef _WIN32
    const auto testItemPath = GENERATE( as< fs::path >(),
        L"C:\\", // Absolute paths.
        L"C:\\abc",
        L"C:\\abc\\def",
        L"\\\\abc", // UNC Paths.
        L"\\\\abc\\",
        L"\\\\abc\\def",
        L"\\\\.", // DOS device path.
        L"\\\\.\\",
        L"\\\\.\\abc\\def",
        L"\\\\.\\UNC\\server\\share",
        L"\\\\?", // Prefixed long paths.
        L"\\\\?\\",
        L"\\\\?\\abc\\def",
        L"\\\\?\\UNC\\server\\share"
    );
#   else
    const auto testItemPath = GENERATE( as< fs::path >(),
        "/",
        "/abc",
        "/abc/def"
    );
#   endif

    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE_THROWS_MATCHES(
            builder.buildPath( testItemPath ),
            BitException,
            Catch::Matchers::Predicate< BitException >(
                []( const BitException& exception ) -> bool {
                    return exception.code() == BitError::ItemHasAbsolutePath;
                },
                "Error code should be BitError::ItemHasAbsolutePath"
            )
        );
    }
}

TEST_CASE( "fsutil: Path building with relative paths", "[fsutil][SafeOutPathBuilder]" ) {
    const auto testBasePath = GENERATE( as< tstring >(),
        BIT7Z_STRING( "" ),
        BIT7Z_STRING( "." ),
        BIT7Z_STRING( "/" ),
        BIT7Z_STRING( "/out" ),
        BIT7Z_STRING( "/out/dir" ),
        BIT7Z_STRING( "out" ),
        BIT7Z_STRING( "out/dir" ),
        // Note: the following paths have different meaning on Windows and on Unix.
        BIT7Z_STRING( "C:" ),
        BIT7Z_STRING( "C:/" ),
        BIT7Z_STRING( "C:/out" ),
        BIT7Z_STRING( "D:" ),
        BIT7Z_STRING( "D:/" ),
        BIT7Z_STRING( "D:/out" )
    );

#   ifdef _WIN32
    const auto testItemPath = GENERATE( as< fs::path >(),
        L"/",
        L"/abc",
        L"/abc/def",
        L"abc/",
        L"abc/def/"
    );
#   else
    const auto testItemPath = GENERATE( as< fs::path >(),
        "abc/",
        "abc/def/"
    );
#   endif
    DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath.relative_path() );
    }
}

#   ifdef _WIN32
TEST_CASE( "fsutil: Path building with Windows' drive-relative paths", "[fsutil][SafeOutPathBuilder]" ) {
    SECTION ( "Simple drive-relative paths (same root as base path)" ) {
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "" ),
            BIT7Z_STRING( "." ),
            BIT7Z_STRING( "/" ),
            BIT7Z_STRING( "/out" ),
            BIT7Z_STRING( "/out/dir" ),
            BIT7Z_STRING( "out" ),
            BIT7Z_STRING( "out/dir" ),
            BIT7Z_STRING( "C:" ),
            BIT7Z_STRING( "C:/" ),
            BIT7Z_STRING( "C:/out" ),
            BIT7Z_STRING( "D:" ),
            BIT7Z_STRING( "D:/" ),
            BIT7Z_STRING( "D:/out" )
        );

        const auto testItemPath = GENERATE( as< fs::path >(),
            L"C:",
            L"C:abc",
            L"C:abc/def/file.txt",
            L"C:file.txt",
            L"D:",
            L"D:abc",
            L"D:abc/def/file.txt",
            L"D:file.txt"
        );

        DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
            const SafeOutPathBuilder builder{ testBasePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            if ( testItemPath.root_name() != builder.basePath().root_name() ) {
                REQUIRE_THROWS( builder.buildPath( testItemPath ) );
            } else {
                REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath.relative_path() );
            }
        }
    }
}
#   endif
#endif

namespace {
struct PathBuildTest {
    tstring basePath;
    fs::path itemPath;
    fs::path expectedPath;
};
} // namespace

TEST_CASE( "fsutil: Check if extracted path is outside base path", "[fsutil][SafeOutPathBuilder]" ) {
    SECTION( "Basic ZipSlip attacks" ) {
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "" ),
            BIT7Z_STRING( "." ),
            BIT7Z_STRING( ".." ),
            BIT7Z_STRING( "out" ),
            BIT7Z_STRING( "/out" ),
            BIT7Z_STRING( "out/dir" ),
            BIT7Z_STRING( "/out/dir" ),
            // Note: On Windows, C: is the current directory on the drive C, C:\\ is the root directory of the drive C.
            BIT7Z_STRING( "C:" ),
            // NOTE: On Windows, the following are absolute paths.
            BIT7Z_STRING( "C:/out" ),
            BIT7Z_STRING( "C:/out/dir" )
        );

        const auto slipPath = GENERATE( as< fs::path >(),
            BIT7Z_NATIVE_STRING( "../evil.txt" ),
            BIT7Z_NATIVE_STRING( "../../evil.txt" ),
            BIT7Z_NATIVE_STRING( "../../../../etc/passwd" ),
            BIT7Z_NATIVE_STRING( "../../../../tmp/pwned" ),
            BIT7Z_NATIVE_STRING( "../folder/" ),
            BIT7Z_NATIVE_STRING( "../folder/evil.txt" ),
            BIT7Z_NATIVE_STRING( "../folder/../../evil.txt" ),
            BIT7Z_NATIVE_STRING( "../../../../../etc/passwd" ),
            BIT7Z_NATIVE_STRING( "folder/../../evil.txt" ),
            BIT7Z_NATIVE_STRING( "folder/../../../../etc/passwd" ),
            BIT7Z_NATIVE_STRING( "a/b/c/../../../../evil.txt" ),
            BIT7Z_NATIVE_STRING( "C:abc/../../" ),
            BIT7Z_NATIVE_STRING( "C:../../../" )
        );

        const auto oldCurrentPath = fs::current_path();
        fs::current_path( test::filesystem::user_dir() );

        DYNAMIC_SECTION(
            "Building output path for " << quoted( slipPath ) << " "
            "inside base path " << quoted( testBasePath ) << " should fail"
        ) {
            const SafeOutPathBuilder builder{ testBasePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            REQUIRE_THROWS_MATCHES(
                builder.buildPath( slipPath ),
                BitException,
                Catch::Matchers::Predicate< BitException >(
                    []( const BitException& exception ) -> bool {
                        return exception.code() == BitError::ItemPathOutsideOutputDirectory;
                    },
                    "Error code should be BitError::ItemPathOutsideOutputDirectory"
                )
            );
        }

        fs::current_path( oldCurrentPath );
    }

#if defined( _WIN32 ) && !defined( BIT7Z_PATH_SANITIZATION )
    SECTION( "Basic ZipSlip attacks with drive-relative path" ) {
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "" ),
            BIT7Z_STRING( "." ),
            BIT7Z_STRING( ".." ),
            BIT7Z_STRING( "out" ),
            BIT7Z_STRING( "/out" ),
            BIT7Z_STRING( "out/dir" ),
            BIT7Z_STRING( "/out/dir" ),
            // Note: On Windows, C: is the current directory on the drive C, C:\\ is the root directory of the drive C.
            BIT7Z_STRING( "C:" ),
            // NOTE: On Windows, the following are absolute paths.
            BIT7Z_STRING( "C:/out" ),
            BIT7Z_STRING( "C:/out/dir" )
        );

        const auto oldCurrentPath = fs::current_path();
        fs::current_path( test::filesystem::user_dir() );

        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )

        const auto slipPath = GENERATE_REF( as< fs::path >(),
            // <base path drive letter>:..
            std::wstring{ builder.basePath().native()[0] } + BIT7Z_NATIVE_STRING( ":.." ),
            // Path with uncommon root drive letter different from the base path drive letter.
            L"A:.."
        );

        DYNAMIC_SECTION(
            "Building output path for " << quoted( slipPath ) << " "
            "inside base path " << quoted( testBasePath ) << " should fail"
        ) {
            REQUIRE_THROWS( builder.buildPath( slipPath ) );
        }

        fs::current_path( oldCurrentPath );
    }
#endif

    SECTION( "Near zip attacks" ) {
#ifdef _WIN32
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "/" ),
            BIT7Z_STRING( "\\" ),
            BIT7Z_STRING( "C:/" ),
            BIT7Z_STRING( "C:\\" )
        );
#else
        const tstring testBasePath = "/";
#endif

        const auto nearSlipPath = GENERATE( as< fs::path >(),
            BIT7Z_NATIVE_STRING( "out/dir/../../../../../../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "out/dir/../../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "out/../../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "../../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "../../../../../../notEvil.txt" )
        );

        // The base path is the root directory, and notEvil.txt is expected to be inside the root directory.
        const auto expectedPath = fs::absolute( testBasePath ) / "notEvil.txt";

        DYNAMIC_SECTION( nearSlipPath << " inside base path " << quoted( testBasePath ) ) {
            const SafeOutPathBuilder builder{ testBasePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            REQUIRE( builder.buildPath( nearSlipPath ) == expectedPath );
        }
    }

    // Note: we already tested throwing when the item path is an absolute path;
    // here, we are testing the specific case where the base path can be a substring of the item path.
    SECTION( "Edge cases (substring)" ) {
        // https://www.sonarsource.com/blog/openrefine-zip-slip/
#ifdef _WIN32
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "C:/Users/john" ),
            BIT7Z_STRING( "C:/Users/john/" )
        );

        const auto testItemPath = GENERATE( as< fs::path >(),
            L"C:/Users/johnny",
            L"C:/Users/johnny/.ssh/id_rsa"
        );
#else
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "/home/john" ),
            BIT7Z_STRING( "/home/john/" )
        );

        const auto testItemPath = GENERATE( as< fs::path >(),
            "/home/johnny",
            "/home/johnny/.ssh/id_rsa"
        );
#endif
#ifndef BIT7Z_PATH_SANITIZATION
        const SafeOutPathBuilder builder{ testBasePath };
        INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
        REQUIRE_THROWS( builder.buildPath( testItemPath ) );
#else
        DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
            const SafeOutPathBuilder builder{ testBasePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
#ifdef _WIN32
            REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / sanitize_path( testItemPath ) );
#else
            REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath.relative_path() );
#endif
        }
#endif
    }

    SECTION( "Edge cases (inside)" ) {
        const auto testBasePath = GENERATE( as< tstring >(),
            BIT7Z_STRING( "" ),
            BIT7Z_STRING( "." ),
            BIT7Z_STRING( "/" ),
            BIT7Z_STRING( "/out" ),
            BIT7Z_STRING( "/out/dir" ),
            BIT7Z_STRING( "out" ),
            BIT7Z_STRING( "out/dir" ),
            BIT7Z_STRING( "C:" ),
            BIT7Z_STRING( "C:/" ),
            BIT7Z_STRING( "C:/out" ),
            BIT7Z_STRING( "D:" ),
            BIT7Z_STRING( "D:/" ),
            BIT7Z_STRING( "D:/out" )
        );

        const auto testItemPath = GENERATE( as< fs::path >(),
            BIT7Z_NATIVE_STRING( "subdir/../legal.txt" ),
            BIT7Z_NATIVE_STRING( "a/b/c/../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "a/b/c/../../notEvil.txt" ),
            BIT7Z_NATIVE_STRING( "a/b/c/../../../notEvil.txt" )
        );

        DYNAMIC_SECTION( quoted( testItemPath ) << " inside base path " << quoted( testBasePath ) ) {
            const SafeOutPathBuilder builder{ testBasePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            REQUIRE( builder.buildPath( testItemPath ) == builder.basePath() / testItemPath.lexically_normal() );
        }
    }

    SECTION( "Edge cases (path traversal that are actually inside the base path)" ) {
        const auto testValues = GENERATE(
            // Paths that traverse up then back into the same directory
            PathBuildTest{ BIT7Z_STRING( "out" ), "../out", "out" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "../out/", "out/" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "../out/folder/notEvil.txt", "out/folder/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "../out/notEvil.txt", "out/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "folder/../notEvil.txt", "out/notEvil.txt" },

            // Multiple traversals that end up inside
            PathBuildTest{ BIT7Z_STRING( "out" ), "folder/../../out/notEvil.txt", "out/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "../out/folder/../../out/notEvil.txt", "out/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out" ), "a/b/c/../../../../out/notEvil.txt", "out/notEvil.txt" },

            // Nested base paths
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "../../out/dir", "out/dir" },
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "../../out/dir/", "out/dir/" },
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "../dir/folder/notEvil.txt", "out/dir/folder/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "../dir/notEvil.txt", "out/dir/notEvil.txt" },

            // Multiple traversals of nested base paths that end up inside
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "folder/../../dir/notEvil.txt", "out/dir/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "../dir/folder/../../dir/notEvil.txt", "out/dir/notEvil.txt" },
            PathBuildTest{ BIT7Z_STRING( "out/dir" ), "a/b/c/../../../../dir/notEvil.txt", "out/dir/notEvil.txt" }
        );

        DYNAMIC_SECTION( quoted( testValues.itemPath ) << " inside base path " << quoted( testValues.basePath ) ) {
            const SafeOutPathBuilder builder{ testValues.basePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            REQUIRE( builder.buildPath( testValues.itemPath ) == fs::absolute( testValues.expectedPath ) );
        }
    }

    SECTION( "Edge cases (case insensitivity, valid on Windows, not on Unix)" ) {
#ifdef BIT7Z_USE_SYSTEM_CODEPAGE
        const auto testValues = GENERATE(
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../DIR" ),
                BIT7Z_NATIVE_STRING( "out/DIR" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../DIR/" ),
                BIT7Z_NATIVE_STRING( "out/DIR/" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../../Out/DIR" ),
                BIT7Z_NATIVE_STRING( "Out/DIR" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../../Out/DIR/" ),
                BIT7Z_NATIVE_STRING( "Out/DIR/" )
            }
        );
#else
        const auto testValues = GENERATE(
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../DIR" ),
                BIT7Z_NATIVE_STRING( "out/DIR" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../DIR/" ),
                BIT7Z_NATIVE_STRING( "out/DIR/" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../../Out/DIR" ),
                BIT7Z_NATIVE_STRING( "Out/DIR" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir" ),
                BIT7Z_NATIVE_STRING( "../../Out/DIR/" ),
                BIT7Z_NATIVE_STRING( "Out/DIR/" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir/testɐ" ),
                BIT7Z_NATIVE_STRING( "../testⱯ" ),
                BIT7Z_NATIVE_STRING( "out/dir/testⱯ" )
            },
            PathBuildTest{
                BIT7Z_STRING( "out/dir/testⱯ" ),
                BIT7Z_NATIVE_STRING( "../testɐ" ),
                BIT7Z_NATIVE_STRING( "out/dir/testɐ" )
            }
        );
#endif
        DYNAMIC_SECTION( quoted( testValues.itemPath ) << " inside base path " << quoted( testValues.basePath ) ) {
#ifdef _WIN32
            const SafeOutPathBuilder builder{ testValues.basePath };
            INFO( "Sanitized base path: " << quoted( builder.basePath() ) )
            REQUIRE( builder.buildPath( testValues.itemPath ) == fs::absolute( testValues.expectedPath ) );
#else
            REQUIRE_THROWS( SafeOutPathBuilder{ testValues.basePath }.buildPath( testValues.itemPath ) );
#endif
        }
    }

#ifdef _WIN32
    /* According to the Unicode folding and case conversion rules,
     * these characters should be treated the same in case-insensitive comparisons.
     * However, Windows does not follow these rules, primarily for compatibility reasons.*/
    SECTION( "Edge cases (Windows' case insensitivity quirks)" ) {
        // German's ß and its uppercase variants ẞ/SS.
        const SafeOutPathBuilder german{ BIT7Z_STRING( "out/dir/german/Straße" ) };
        REQUIRE_THROWS( german.buildPath( L"../STRAẞE" ) );
        REQUIRE_THROWS( german.buildPath( L"../STRASSE" ) );

        // Turkish's dotted i and its uppercase variant İ.
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/iki" ) }.buildPath( L"../İKİ" ) );
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/İki" ) }.buildPath( L"../iKİ" ) );
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/İKİ" ) }.buildPath( L"../iki" ) );

        // Turkish's dotless ı and its uppercase variant I.
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/ışık" ) }.buildPath( L"../IŞIK" ) );
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/Işık" ) }.buildPath( L"../ışIK" ) );
        REQUIRE_THROWS( SafeOutPathBuilder{ BIT7Z_STRING( "out/dir/turkish/IŞIK" ) }.buildPath( L"../ışık" ) );
    }
#endif
}