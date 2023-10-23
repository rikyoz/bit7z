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

#ifdef BIT7Z_AUTO_FORMAT

#include <catch2/catch.hpp>

#include "utils/format.hpp"
#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"

#include <bitarchivereader.hpp>
#include <bitexception.hpp>
#include <bitformat.hpp>
#include <internal/formatdetect.hpp>

using bit7z::BitInFormat;
using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

// Note: format detection by extension doesn't actually require the file to exist.
TEST_CASE( "formatdetect: Format detection by extension", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    auto test = GENERATE( TestInputFormat{ "7z", BitFormat::SevenZip },
                          TestInputFormat{ "7z.001", BitFormat::Split },
                          TestInputFormat{ "ar", BitFormat::Deb },
                          TestInputFormat{ "arj", BitFormat::Arj },
                          TestInputFormat{ "bz2", BitFormat::BZip2 },
                          TestInputFormat{ "bzip2", BitFormat::BZip2 },
                          TestInputFormat{ "cab", BitFormat::Cab },
                          TestInputFormat{ "chi", BitFormat::Chm },
                          TestInputFormat{ "chm", BitFormat::Chm },
                          TestInputFormat{ "cpio", BitFormat::Cpio },
                          TestInputFormat{ "cramfs", BitFormat::CramFS },
                          TestInputFormat{ "deb", BitFormat::Deb },
                          TestInputFormat{ "deflate.swfc", BitFormat::Auto },
                          TestInputFormat{ "dmg", BitFormat::Dmg },
                          TestInputFormat{ "doc", BitFormat::Compound },
                          TestInputFormat{ "docx", BitFormat::Zip },
                          TestInputFormat{ "dylib", BitFormat::Macho },
                          TestInputFormat{ "elf", BitFormat::Auto }, // The elf extension doesn't really exist.
                          TestInputFormat{ "exe", BitFormat::Pe }, // We don't consider SFX executables!
                          TestInputFormat{ "ext2", BitFormat::Ext },
                          TestInputFormat{ "ext3", BitFormat::Ext },
                          TestInputFormat{ "ext4", BitFormat::Ext },
                          TestInputFormat{ "ext4.img", BitFormat::Auto }, // The img extension is used for many formats
                          TestInputFormat{ "fat", BitFormat::Fat },
                          TestInputFormat{ "fat12.img", BitFormat::Auto },
                          TestInputFormat{ "fat16.img", BitFormat::Auto },
                          TestInputFormat{ "flv", BitFormat::Flv },
                          TestInputFormat{ "gpt", BitFormat::GPT },
                          TestInputFormat{ "gz", BitFormat::GZip },
                          TestInputFormat{ "gzip", BitFormat::GZip },
                          TestInputFormat{ "hfs", BitFormat::Hfs },
                          TestInputFormat{ "hxs", BitFormat::Hxs },
                          TestInputFormat{ "iso", BitFormat::Auto }, // An iso file can be either Iso or Udf
                          TestInputFormat{ "iso.img", BitFormat::Auto },
                          TestInputFormat{ "lha", BitFormat::Lzh },
                          TestInputFormat{ "lzh", BitFormat::Lzh },
                          TestInputFormat{ "lzma", BitFormat::Lzma },
                          TestInputFormat{ "lzma86", BitFormat::Lzma86 },
                          TestInputFormat{ "lzma.swfc", BitFormat::Auto },
                          TestInputFormat{ "macho", BitFormat::Auto }, // The macho extension doesn't really exist.
                          TestInputFormat{ "mbr", BitFormat::Mbr },
                          TestInputFormat{ "mslz", BitFormat::Mslz },
                          TestInputFormat{ "nsis", BitFormat::Nsis },
                          TestInputFormat{ "ntfs", BitFormat::Ntfs },
                          TestInputFormat{ "ntfs.img", BitFormat::Auto },
                          TestInputFormat{ "obj", BitFormat::COFF },
                          TestInputFormat{ "odp", BitFormat::Zip },
                          TestInputFormat{ "ods", BitFormat::Zip },
                          TestInputFormat{ "odt", BitFormat::Zip },
                          TestInputFormat{ "ova", BitFormat::Tar },
                          TestInputFormat{ "part1.rar", BitFormat::Auto }, // Rar files can be Rar4 or Rar5
                          TestInputFormat{ "part2.rar", BitFormat::Auto },
                          TestInputFormat{ "part3.rar", BitFormat::Auto },
                          TestInputFormat{ "pkg", BitFormat::Xar },
                          TestInputFormat{ "pmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppt", BitFormat::Compound },
                          TestInputFormat{ "pptx", BitFormat::Zip },
                          TestInputFormat{ "qcow", BitFormat::QCow },
                          TestInputFormat{ "qcow2", BitFormat::QCow },
                          TestInputFormat{ "r00", BitFormat::Rar }, // Old-style multi-volume RAR4 extension
                          TestInputFormat{ "r01", BitFormat::Rar },
                          TestInputFormat{ "r42", BitFormat::Rar },
                          TestInputFormat{ "rar4.rar", BitFormat::Auto }, // Rar files can be Rar4 or Rar5
                          TestInputFormat{ "rar5.rar", BitFormat::Auto },
                          TestInputFormat{ "rpm", BitFormat::Rpm },
                          TestInputFormat{ "squashfs", BitFormat::SquashFS },
                          TestInputFormat{ "swf", BitFormat::Swf },
                          TestInputFormat{ "swm", BitFormat::Wim },
                          TestInputFormat{ "tar", BitFormat::Tar },
                          TestInputFormat{ "taz", BitFormat::Z },
                          TestInputFormat{ "tbz", BitFormat::BZip2 },
                          TestInputFormat{ "tbz2", BitFormat::BZip2 },
                          TestInputFormat{ "tgz", BitFormat::GZip },
                          TestInputFormat{ "txz", BitFormat::Xz },
                          TestInputFormat{ "udf", BitFormat::Udf },
                          TestInputFormat{ "udf.img", BitFormat::Auto },
                          TestInputFormat{ "udf.iso", BitFormat::Auto },
                          TestInputFormat{ "vdi", BitFormat::VDI },
                          TestInputFormat{ "vhd", BitFormat::Vhd },
                          TestInputFormat{ "vmdk", BitFormat::VMDK },
                          TestInputFormat{ "wim", BitFormat::Wim },
                          TestInputFormat{ "xar", BitFormat::Xar },
                          TestInputFormat{ "xls", BitFormat::Compound },
                          TestInputFormat{ "xlsx", BitFormat::Zip },
                          TestInputFormat{ "xz", BitFormat::Xz },
                          TestInputFormat{ "z", BitFormat::Z },
                          TestInputFormat{ "z00", BitFormat::Zip }, // Old-style multi-volume ZIP extension
                          TestInputFormat{ "z01", BitFormat::Zip },
                          TestInputFormat{ "z21", BitFormat::Zip },
                          TestInputFormat{ "zip", BitFormat::Zip },
                          TestInputFormat{ "zipx", BitFormat::Zip } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        REQUIRE( detect_format_from_extension( "valid." + test.extension ) == test.format );
    }
}

TEST_CASE( "formatdetect: Format detection by signature", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    auto test = GENERATE( TestInputFormat{ "7z", BitFormat::SevenZip },
                          TestInputFormat{ "ar", BitFormat::Deb },
                          TestInputFormat{ "arj", BitFormat::Arj },
                          TestInputFormat{ "bz2", BitFormat::BZip2 },
                          TestInputFormat{ "bzip2", BitFormat::BZip2 },
                          TestInputFormat{ "cab", BitFormat::Cab },
                          TestInputFormat{ "chi", BitFormat::Chm },
                          TestInputFormat{ "chm", BitFormat::Chm },
                          TestInputFormat{ "cpio", BitFormat::Cpio },
                          TestInputFormat{ "cramfs", BitFormat::CramFS },
                          TestInputFormat{ "deb", BitFormat::Deb },
                          TestInputFormat{ "deflate.swfc", BitFormat::Swfc },
                          TestInputFormat{ "doc", BitFormat::Compound },
                          TestInputFormat{ "docx", BitFormat::Zip },
                          TestInputFormat{ "dylib", BitFormat::Macho },
                          TestInputFormat{ "elf", BitFormat::Elf },
                          TestInputFormat{ "exe", BitFormat::Pe }, // We don't consider SFX executables!
                          TestInputFormat{ "ext2", BitFormat::Ext },
                          TestInputFormat{ "ext3", BitFormat::Ext },
                          TestInputFormat{ "ext4", BitFormat::Ext },
                          TestInputFormat{ "ext4.img", BitFormat::Ext },
                          TestInputFormat{ "fat", BitFormat::Fat },
                          TestInputFormat{ "fat12.img", BitFormat::Fat },
                          TestInputFormat{ "fat16.img", BitFormat::Fat },
                          TestInputFormat{ "flv", BitFormat::Flv },
                          TestInputFormat{ "gpt", BitFormat::GPT },
                          TestInputFormat{ "gz", BitFormat::GZip },
                          TestInputFormat{ "gzip", BitFormat::GZip },
                          TestInputFormat{ "hfs", BitFormat::Hfs },
                          TestInputFormat{ "iso", BitFormat::Iso },
                          TestInputFormat{ "iso.img", BitFormat::Iso },
                          TestInputFormat{ "lha", BitFormat::Lzh },
                          TestInputFormat{ "lzh", BitFormat::Lzh },
                          TestInputFormat{ "lzma", BitFormat::Lzma },
                          TestInputFormat{ "lzma86", BitFormat::Lzma86 },
                          TestInputFormat{ "lzma.swfc", BitFormat::Swfc },
                          TestInputFormat{ "macho", BitFormat::Macho },
                          TestInputFormat{ "mslz", BitFormat::Mslz },
                          TestInputFormat{ "nsis", BitFormat::Nsis },
                          TestInputFormat{ "ntfs", BitFormat::Ntfs },
                          TestInputFormat{ "ntfs.img", BitFormat::Ntfs },
                          TestInputFormat{ "odp", BitFormat::Zip },
                          TestInputFormat{ "ods", BitFormat::Zip },
                          TestInputFormat{ "odt", BitFormat::Zip },
                          TestInputFormat{ "ova", BitFormat::Tar },
                          TestInputFormat{ "part1.rar", BitFormat::Rar5 },
                          TestInputFormat{ "part2.rar", BitFormat::Rar5 },
                          TestInputFormat{ "part3.rar", BitFormat::Rar5 },
                          TestInputFormat{ "pkg", BitFormat::Xar },
                          TestInputFormat{ "pmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppmd", BitFormat::Ppmd },
                          TestInputFormat{ "ppt", BitFormat::Compound },
                          TestInputFormat{ "pptx", BitFormat::Zip },
                          TestInputFormat{ "qcow", BitFormat::QCow },
                          TestInputFormat{ "qcow2", BitFormat::QCow },
                          TestInputFormat{ "rar4.rar", BitFormat::Rar },
                          TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                          TestInputFormat{ "rpm", BitFormat::Rpm },
                          TestInputFormat{ "sqsh", BitFormat::SquashFS },
                          TestInputFormat{ "squashfs", BitFormat::SquashFS },
                          TestInputFormat{ "swf", BitFormat::Swf },
                          TestInputFormat{ "swm", BitFormat::Wim },
                          TestInputFormat{ "tar", BitFormat::Tar },
                          TestInputFormat{ "taz", BitFormat::Z },
                          TestInputFormat{ "tbz", BitFormat::BZip2 },
                          TestInputFormat{ "tbz2", BitFormat::BZip2 },
                          TestInputFormat{ "tgz", BitFormat::GZip },
                          TestInputFormat{ "txz", BitFormat::Xz },
                          TestInputFormat{ "vdi", BitFormat::VDI },
                          TestInputFormat{ "vhd", BitFormat::Vhd },
                          TestInputFormat{ "vmdk", BitFormat::VMDK },
                          TestInputFormat{ "wim", BitFormat::Wim },
                          TestInputFormat{ "xar", BitFormat::Xar },
                          TestInputFormat{ "xls", BitFormat::Compound },
                          TestInputFormat{ "xlsx", BitFormat::Zip },
                          TestInputFormat{ "xz", BitFormat::Xz },
                          TestInputFormat{ "z", BitFormat::Z },
                          TestInputFormat{ "zip", BitFormat::Zip },
                          TestInputFormat{ "zipx", BitFormat::Zip } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        // We might directly test the detect_format_from_signature function, but it would require us to include
        // too many internal headers, and it easily gives compilation problems.
        // Hence, we use BitArchiveReader for reading the file from a buffer (to avoid detection via file extensions).

        REQUIRE_LOAD_FILE( fileBuffer, "valid." + test.extension );
        const BitArchiveReader reader{ lib, fileBuffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }
}

#ifdef _WIN32

// For some reason, 7-zip fails to open UDF files on Linux, so we test them only on Windows.
TEST_CASE( "formatdetect: Format detection by signature (UDF files)", "[formatdetect]" ) {
    const fs::path oldCurrentDir = current_dir();
    const auto testDir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( testDir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    auto test = GENERATE( TestInputFormat{ "udf", BitFormat::Udf },
                          TestInputFormat{ "udf.img", BitFormat::Udf },
                          TestInputFormat{ "udf.iso", BitFormat::Udf } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        REQUIRE_LOAD_FILE( fileBuffer, "valid." + test.extension );
        const BitArchiveReader reader{ lib, fileBuffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

#endif

TEST_CASE( "formatdetect: Format detection of invalid archives", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "small" ) ), BitException );
    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "invalid" ) ), BitException );
}

TEST_CASE( "formatdetect: Format detection of archive with a wrong extension (Issue #134)", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    auto testFile = GENERATE( as< tstring >(),
                              BIT7Z_STRING( "wrong_extension.rar" ),
                              BIT7Z_STRING( "wrong_extension.bz2" ) );

    DYNAMIC_SECTION( "Reading file with a wrong extension: " << Catch::StringMaker< tstring >::convert( testFile ) ) {
        // From file
        REQUIRE_THROWS( BitArchiveReader( lib, testFile, BitFormat::Rar ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, testFile, BitFormat::SevenZip ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, testFile ) );

        // From buffer
        auto fileBuffer = load_file( testFile );
        REQUIRE_THROWS( BitArchiveReader( lib, fileBuffer, BitFormat::Rar ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, fileBuffer, BitFormat::SevenZip ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, fileBuffer ) );
    }
}

TEST_CASE( "formatdetect: Format detection of an archive file without an extension", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    REQUIRE( detect_format_from_extension( "noextension" ) == BitFormat::Auto );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };
    const BitArchiveReader reader{ lib, BIT7Z_STRING( "noextension" ) };
    REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
    REQUIRE_NOTHROW( reader.test() );
}

#endif // BIT7Z_AUTO_FORMAT