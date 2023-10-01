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

#include <bit7z/bitformat.hpp>
#include <internal/fsutil.hpp>

#include <array>
#include <vector>
#include <map>

#include "utils/filesystem.hpp"

using std::vector;
using std::map;
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
    //       directory, hence we must declare the vector inside the test case and not outside!
    const std::array< TestItem, 28 > testItems{ {
        { ".",                                                 "" },
        { "./",                                                "" },
        { "..",                                                "" },
        { "../",                                               "" },
        { "italy.svg",                                         "italy.svg" },
        { "folder",                                            "folder" },
        { "folder/",                                           "folder/" },
        { "folder/clouds.jpg",                                 "folder/clouds.jpg" },
        { "folder/subfolder2",                                 "folder/subfolder2" },
        { "folder/subfolder2/",                                "folder/subfolder2/" },
        { "folder/subfolder2/homework.doc",                   "folder/subfolder2/homework.doc" },
        { "./italy.svg",                                       "italy.svg" },
        { "./folder",                                          "folder" },
        { "./folder/",                                         "folder" },
        { "./folder/clouds.jpg",                               "clouds.jpg" },
        { "./folder/subfolder2",                               "subfolder2" },
        { "./folder/subfolder2/homework.doc",                 "homework.doc" },
        { "./../test_filesystem/",                             "test_filesystem" },
        { "./../test_filesystem/folder/",                      "folder" },
        { fs::absolute( "." ),                                 "test_filesystem" },
        { fs::absolute( "../" ),                               "data" },
        { fs::absolute( "./italy.svg" ),                       "italy.svg" },
        { fs::absolute( "./folder" ),                          "folder" },
        { fs::absolute( "./folder/" ),                         "folder" },
        { fs::absolute( "./folder/clouds.jpg" ),               "clouds.jpg" },
        { fs::absolute( "./folder/subfolder2" ),               "subfolder2" },
        { fs::absolute( "./folder/subfolder2/" ),              "subfolder2" },
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

#if defined( _WIN32 ) && defined( BIT7Z_PATH_SANITIZATION )
TEST_CASE( "fsutil: Sanitizing Windows paths", "[fsutil][sanitize_path]" ) {
    REQUIRE( sanitize_path( L"hello world.txt" ) == L"hello world.txt" );
    REQUIRE( sanitize_path( L"hello?world<" ) == L"hello_world_" );
    REQUIRE( sanitize_path( L":hello world|" ) == L"_hello world_" );
    REQUIRE( sanitize_path( L"hello?world<.txt" ) == L"hello_world_.txt" );
    REQUIRE( sanitize_path( L":hello world|.txt" ) == L"_hello world_.txt" );

    REQUIRE( sanitize_path( L"COM0" ) == L"_COM0" );
    REQUIRE( sanitize_path( L"COM0.txt" ) == L"COM0.txt" );
    REQUIRE( sanitize_path( L"LPT9" ) == L"_LPT9" );
    REQUIRE( sanitize_path( L"LPT9.txt" ) == L"LPT9.txt" );
    REQUIRE( sanitize_path( L"COM42" ) == L"COM42" );
    REQUIRE( sanitize_path( L"LPT42" ) == L"LPT42" );

    REQUIRE( sanitize_path( L"CON" ) == L"_CON" );
    REQUIRE( sanitize_path( L"PRN" ) == L"_PRN" );
    REQUIRE( sanitize_path( L"AUX" ) == L"_AUX" );
    REQUIRE( sanitize_path( L"NUL" ) == L"_NUL" );
    REQUIRE( sanitize_path( L"CONO" ) == L"CONO" );
    REQUIRE( sanitize_path( L"PRNG" ) == L"PRNG" );
    REQUIRE( sanitize_path( L"AUXI" ) == L"AUXI" );
    REQUIRE( sanitize_path( L"NULL" ) == L"NULL" );

    REQUIRE( sanitize_path( L"C:/abc/NUL/def" ) == L"C:\\abc\\_NUL\\def" );
    REQUIRE( sanitize_path( L"C:\\abc\\NUL\\def" ) == L"C:\\abc\\_NUL\\def" );

    REQUIRE( sanitize_path( L"C:/Test/COM0/hello?world<.txt" ) == L"C:\\Test\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"C:\\Test\\COM0\\hello?world<.txt" ) == L"C:\\Test\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"Test/COM0/hello?world<.txt" ) == L"Test\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"Test\\COM0\\hello?world<.txt" ) == L"Test\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"../COM0/hello?world<.txt" ) == L"..\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"..\\COM0\\hello?world<.txt" ) == L"..\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"./COM0/hello?world<.txt" ) == L".\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L".\\COM0\\hello?world<.txt" ) == L".\\_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"COM0/hello?world<.txt" ) == L"_COM0\\hello_world_.txt" );
    REQUIRE( sanitize_path( L"COM0\\hello?world<.txt" ) == L"_COM0\\hello_world_.txt" );

    REQUIRE( sanitize_path( L"C:/Test/:hello world|/LPT5" ) == L"C:\\Test\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"C:\\Test\\:hello world|\\LPT5" ) == L"C:\\Test\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"Test/:hello world|/LPT5" ) == L"Test\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"Test\\:hello world|\\LPT5" ) == L"Test\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"../:hello world|/LPT5" ) == L"..\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"..\\:hello world|\\LPT5" ) == L"..\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L"./:hello world|/LPT5" ) == L".\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L".\\:hello world|\\LPT5" ) == L".\\_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L":hello world|/LPT5" ) == L"_hello world_\\_LPT5" );
    REQUIRE( sanitize_path( L":hello world|\\LPT5" ) == L"_hello world_\\_LPT5" );
}
#endif