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

#include "filesystem.hpp"
#include "shared_lib.hpp"

#include <bitarchivereader.hpp>
#include <bitexception.hpp>
#include <bitformat.hpp>
#include <internal/formatdetect.hpp>

using bit7z::BitInFormat;
using namespace bit7z;
using namespace bit7z::test::filesystem;

struct TestDetection {
    std::string extension;
    const BitInFormat& format;
};

// Note: format detection by extension doesn't actually require the file to exist!
TEST_CASE( "formatdetect: Format detection by extension", "[formatdetect]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( test_dir ) );

    auto test = GENERATE( TestDetection{ "7z", BitFormat::SevenZip },
                          TestDetection{ "7z.001", BitFormat::Split },
                          TestDetection{ "ar", BitFormat::Deb },
                          TestDetection{ "arj", BitFormat::Arj },
                          TestDetection{ "bz2", BitFormat::BZip2 },
                          TestDetection{ "bzip2", BitFormat::BZip2 },
                          TestDetection{ "cab", BitFormat::Cab },
                          TestDetection{ "chi", BitFormat::Chm },
                          TestDetection{ "chm", BitFormat::Chm },
                          TestDetection{ "cpio", BitFormat::Cpio },
                          TestDetection{ "cramfs", BitFormat::CramFS },
                          TestDetection{ "deb", BitFormat::Deb },
                          TestDetection{ "dmg", BitFormat::Dmg },
                          TestDetection{ "doc", BitFormat::Compound },
                          TestDetection{ "docx", BitFormat::Zip },
                          TestDetection{ "dylib", BitFormat::Macho },
                          TestDetection{ "elf", BitFormat::Auto }, // The elf extension doesn't really exist.
                          TestDetection{ "exe", BitFormat::Pe }, // We don't consider SFX executables!
                          TestDetection{ "ext2", BitFormat::Ext },
                          TestDetection{ "ext3", BitFormat::Ext },
                          TestDetection{ "ext4", BitFormat::Ext },
                          TestDetection{ "ext4.img", BitFormat::Auto }, // The img extension is used for many formats
                          TestDetection{ "fat", BitFormat::Fat },
                          TestDetection{ "fat12.img", BitFormat::Auto },
                          TestDetection{ "fat16.img", BitFormat::Auto },
                          TestDetection{ "gpt", BitFormat::GPT },
                          TestDetection{ "gz", BitFormat::GZip },
                          TestDetection{ "gzip", BitFormat::GZip },
                          TestDetection{ "hfs", BitFormat::Hfs },
                          TestDetection{ "iso", BitFormat::Auto }, // An iso file can be either Iso or Udf
                          TestDetection{ "iso.img", BitFormat::Auto },
                          TestDetection{ "lha", BitFormat::Lzh },
                          TestDetection{ "lzh", BitFormat::Lzh },
                          TestDetection{ "lzma", BitFormat::Lzma },
                          TestDetection{ "lzma86", BitFormat::Lzma86 },
                          TestDetection{ "macho", BitFormat::Auto }, // The macho extension doesn't really exist.
                          TestDetection{ "mslz", BitFormat::Mslz },
                          TestDetection{ "nsis", BitFormat::Nsis },
                          TestDetection{ "ntfs", BitFormat::Ntfs },
                          TestDetection{ "ntfs.img", BitFormat::Auto },
                          TestDetection{ "obj", BitFormat::COFF },
                          TestDetection{ "odp", BitFormat::Zip },
                          TestDetection{ "ods", BitFormat::Zip },
                          TestDetection{ "odt", BitFormat::Zip },
                          TestDetection{ "ova", BitFormat::Tar },
                          TestDetection{ "part1.rar", BitFormat::Auto }, // Rar files can be Rar4 or Rar5
                          TestDetection{ "part2.rar", BitFormat::Auto },
                          TestDetection{ "part3.rar", BitFormat::Auto },
                          TestDetection{ "pkg", BitFormat::Xar },
                          TestDetection{ "pmd", BitFormat::Ppmd },
                          TestDetection{ "ppmd", BitFormat::Ppmd },
                          TestDetection{ "ppt", BitFormat::Compound },
                          TestDetection{ "pptx", BitFormat::Zip },
                          TestDetection{ "qcow", BitFormat::QCow },
                          TestDetection{ "qcow2", BitFormat::QCow },
                          TestDetection{ "r00", BitFormat::Rar }, // Old-style multi-volume RAR4 extension
                          TestDetection{ "r01", BitFormat::Rar },
                          TestDetection{ "r42", BitFormat::Rar },
                          TestDetection{ "rar4.rar", BitFormat::Auto }, // Rar files can be Rar4 or Rar5
                          TestDetection{ "rar5.rar", BitFormat::Auto },
                          TestDetection{ "rpm", BitFormat::Rpm },
                          TestDetection{ "squashfs", BitFormat::SquashFS },
                          TestDetection{ "swf", BitFormat::Swf },
                          TestDetection{ "swm", BitFormat::Wim },
                          TestDetection{ "tar", BitFormat::Tar },
                          TestDetection{ "taz", BitFormat::Z },
                          TestDetection{ "tbz", BitFormat::BZip2 },
                          TestDetection{ "tbz2", BitFormat::BZip2 },
                          TestDetection{ "tgz", BitFormat::GZip },
                          TestDetection{ "txz", BitFormat::Xz },
                          TestDetection{ "udf", BitFormat::Udf },
                          TestDetection{ "udf.img", BitFormat::Auto },
                          TestDetection{ "udf.iso", BitFormat::Auto },
                          TestDetection{ "vdi", BitFormat::VDI },
                          TestDetection{ "vhd", BitFormat::Vhd },
                          TestDetection{ "vmdk", BitFormat::VMDK },
                          TestDetection{ "wim", BitFormat::Wim },
                          TestDetection{ "xar", BitFormat::Xar },
                          TestDetection{ "xls", BitFormat::Compound },
                          TestDetection{ "xlsx", BitFormat::Zip },
                          TestDetection{ "xz", BitFormat::Xz },
                          TestDetection{ "z", BitFormat::Z },
                          TestDetection{ "z00", BitFormat::Zip }, // Old-style multi-volume ZIP extension
                          TestDetection{ "z01", BitFormat::Zip },
                          TestDetection{ "z21", BitFormat::Zip },
                          TestDetection{ "zip", BitFormat::Zip },
                          TestDetection{ "zipx", BitFormat::Zip } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        REQUIRE( detectFormatFromExt( "valid." + test.extension ) == test.format );
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

TEST_CASE( "formatdetect: Format detection by signature", "[formatdetect]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    auto test = GENERATE( TestDetection{ "7z", BitFormat::SevenZip },
                          TestDetection{ "ar", BitFormat::Deb },
                          TestDetection{ "arj", BitFormat::Arj },
                          TestDetection{ "bz2", BitFormat::BZip2 },
                          TestDetection{ "bzip2", BitFormat::BZip2 },
                          TestDetection{ "cab", BitFormat::Cab },
                          TestDetection{ "chi", BitFormat::Chm },
                          TestDetection{ "chm", BitFormat::Chm },
                          TestDetection{ "cpio", BitFormat::Cpio },
                          TestDetection{ "cramfs", BitFormat::CramFS },
                          TestDetection{ "deb", BitFormat::Deb },
                          TestDetection{ "doc", BitFormat::Compound },
                          TestDetection{ "docx", BitFormat::Zip },
                          TestDetection{ "dylib", BitFormat::Macho },
                          TestDetection{ "elf", BitFormat::Elf },
                          TestDetection{ "exe", BitFormat::Pe }, // We don't consider SFX executables!
                          TestDetection{ "ext2", BitFormat::Ext },
                          TestDetection{ "ext3", BitFormat::Ext },
                          TestDetection{ "ext4", BitFormat::Ext },
                          TestDetection{ "ext4.img", BitFormat::Ext },
                          TestDetection{ "fat", BitFormat::Fat },
                          TestDetection{ "fat12.img", BitFormat::Fat },
                          TestDetection{ "fat16.img", BitFormat::Fat },
                          TestDetection{ "gpt", BitFormat::GPT },
                          TestDetection{ "gz", BitFormat::GZip },
                          TestDetection{ "gzip", BitFormat::GZip },
                          TestDetection{ "hfs", BitFormat::Hfs },
                          TestDetection{ "iso", BitFormat::Iso },
                          TestDetection{ "iso.img", BitFormat::Iso },
                          TestDetection{ "lha", BitFormat::Lzh },
                          TestDetection{ "lzh", BitFormat::Lzh },
                          TestDetection{ "lzma", BitFormat::Lzma },
                          TestDetection{ "lzma86", BitFormat::Lzma86 },
                          TestDetection{ "macho", BitFormat::Macho },
                          TestDetection{ "mslz", BitFormat::Mslz },
                          TestDetection{ "nsis", BitFormat::Nsis },
                          TestDetection{ "ntfs", BitFormat::Ntfs },
                          TestDetection{ "ntfs.img", BitFormat::Ntfs },
                          TestDetection{ "odp", BitFormat::Zip },
                          TestDetection{ "ods", BitFormat::Zip },
                          TestDetection{ "odt", BitFormat::Zip },
                          TestDetection{ "ova", BitFormat::Tar },
                          TestDetection{ "part1.rar", BitFormat::Rar5 },
                          TestDetection{ "part2.rar", BitFormat::Rar5 },
                          TestDetection{ "part3.rar", BitFormat::Rar5 },
                          TestDetection{ "pkg", BitFormat::Xar },
                          TestDetection{ "pmd", BitFormat::Ppmd },
                          TestDetection{ "ppmd", BitFormat::Ppmd },
                          TestDetection{ "ppt", BitFormat::Compound },
                          TestDetection{ "pptx", BitFormat::Zip },
                          TestDetection{ "qcow", BitFormat::QCow },
                          TestDetection{ "qcow2", BitFormat::QCow },
                          TestDetection{ "rar4.rar", BitFormat::Rar },
                          TestDetection{ "rar5.rar", BitFormat::Rar5 },
                          TestDetection{ "rpm", BitFormat::Rpm },
                          TestDetection{ "sqsh", BitFormat::SquashFS },
                          TestDetection{ "squashfs", BitFormat::SquashFS },
                          TestDetection{ "swf", BitFormat::Swf },
                          TestDetection{ "swm", BitFormat::Wim },
                          TestDetection{ "tar", BitFormat::Tar },
                          TestDetection{ "taz", BitFormat::Z },
                          TestDetection{ "tbz", BitFormat::BZip2 },
                          TestDetection{ "tbz2", BitFormat::BZip2 },
                          TestDetection{ "tgz", BitFormat::GZip },
                          TestDetection{ "txz", BitFormat::Xz },
                          TestDetection{ "vdi", BitFormat::VDI },
                          TestDetection{ "vhd", BitFormat::Vhd },
                          TestDetection{ "vmdk", BitFormat::VMDK },
                          TestDetection{ "wim", BitFormat::Wim },
                          TestDetection{ "xar", BitFormat::Xar },
                          TestDetection{ "xls", BitFormat::Compound },
                          TestDetection{ "xlsx", BitFormat::Zip },
                          TestDetection{ "xz", BitFormat::Xz },
                          TestDetection{ "z", BitFormat::Z },
                          TestDetection{ "zip", BitFormat::Zip },
                          TestDetection{ "zipx", BitFormat::Zip } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        // We might directly test the detectFormatFromSig function, but it would require us to include
        // too many internal headers, and it easily gives compilation problems.
        // Hence, we use BitArchiveReader for reading the file from a buffer (to avoid signature detection).

        const auto file_buffer = load_file( "valid." + test.extension );
        const BitArchiveReader reader{ lib, file_buffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#ifdef _WIN32

// For some reasons, 7-zip fails to open UDF files on Linux, so we test them only on Windows.
TEST_CASE( "formatdetect: Format detection by signature (UDF files)", "[formatdetect]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    auto test = GENERATE( TestDetection{ "udf", BitFormat::Udf },
                          TestDetection{ "udf.img", BitFormat::Udf },
                          TestDetection{ "udf.iso", BitFormat::Udf } );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        const auto file_buffer = load_file( "valid." + test.extension );
        const BitArchiveReader reader{ lib, file_buffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }

    REQUIRE( set_current_dir( old_current_dir ) );
}

#endif

TEST_CASE( "formatdetect: Format detection of invalid archives", "[formatdetect]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "small" ) ), BitException );
    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "invalid" ) ), BitException );

    REQUIRE( set_current_dir( old_current_dir ) );
}

TEST_CASE( "formatdetect: Format detection of archive with a wrong extension (Issue #134)", "[formatdetect]" ) {
    const fs::path old_current_dir = current_dir();
    const auto test_dir = fs::path{ test_archives_dir } / "detection";
    REQUIRE( set_current_dir( test_dir ) );

    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    // From file
    REQUIRE_THROWS( BitArchiveReader( lib, BIT7Z_STRING( "wrong_extension.rar" ), BitFormat::Rar ) );
    REQUIRE_NOTHROW( BitArchiveReader( lib, BIT7Z_STRING( "wrong_extension.rar" ), BitFormat::SevenZip ) );
    REQUIRE_NOTHROW( BitArchiveReader( lib, BIT7Z_STRING( "wrong_extension.rar" ) ) );

    // From buffer
    auto file_buffer = load_file( "wrong_extension.rar" );
    REQUIRE_THROWS( BitArchiveReader( lib, file_buffer, BitFormat::Rar ) );
    REQUIRE_NOTHROW( BitArchiveReader( lib, file_buffer, BitFormat::SevenZip ) );
    REQUIRE_NOTHROW( BitArchiveReader( lib, file_buffer ) );

    REQUIRE( set_current_dir( old_current_dir ) );
}

#endif