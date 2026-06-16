// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifdef BIT7Z_AUTO_FORMAT

#include <catch2/catch.hpp>

#include "utils/exception.hpp"
#include "utils/format.hpp"
#include "utils/filesystem.hpp"
#include "utils/shared_lib.hpp"

#include <bitarchivereader.hpp>
#include <bitexception.hpp>
#include <bitformat.hpp>
#include <internal/formatdetect.hpp>
#include <internal/operationresult.hpp>

using bit7z::BitInFormat;
using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

#ifdef BIT7Z_DETECT_FROM_EXTENSION
// Note: format detection by extension doesn't actually require the file to exist.
TEST_CASE( "formatdetect: Format detection by extension", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    const auto test = GENERATE(
        TestInputFormat{ "7z", BitFormat::SevenZip },
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
        TestInputFormat{ "zipx", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        REQUIRE( detectFormatFromExtension( "valid." + test.extension ) == test.format );
    }
}
#endif

TEST_CASE( "formatdetect: Format detection by signature", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    auto test = GENERATE(
        TestInputFormat{ "7z", BitFormat::SevenZip },
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
        TestInputFormat{ "zipx", BitFormat::Zip }
    );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        // We might directly test the detect_format_from_signature function, but it would require us to include
        // too many internal headers, and it easily gives compilation problems.
        // Hence, we use BitArchiveReader for reading the file from a buffer (to avoid detection via file extensions).

        REQUIRE_LOAD_FILE( fileBuffer, "valid." + test.extension );
        const BitArchiveReader reader{ test::sevenzipLib(), fileBuffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }
}

#ifdef _WIN32

// For some reason, 7-zip fails to open UDF files on Linux, so we test them only on Windows.
TEST_CASE( "formatdetect: Format detection by signature (UDF files)", "[formatdetect]" ) {
    const fs::path oldCurrentDir = currentDir();
    const auto testDir = fs::path{ test_archives_dir } / "detection" / "valid";
    REQUIRE( setCurrentDir( testDir ) );

    auto test = GENERATE(
        TestInputFormat{ "udf", BitFormat::Udf },
        TestInputFormat{ "udf.img", BitFormat::Udf },
        TestInputFormat{ "udf.iso", BitFormat::Udf }
    );

    DYNAMIC_SECTION( "Extension: " << test.extension ) {
        REQUIRE_LOAD_FILE( fileBuffer, "valid." + test.extension );
        const BitArchiveReader reader{ test::sevenzipLib(), fileBuffer };
        REQUIRE( reader.detectedFormat() == test.format );
    }

    REQUIRE( setCurrentDir( oldCurrentDir ) );
}

#endif

TEST_CASE( "formatdetect: Format detection of invalid archives", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "small" ) ), BitException );
    REQUIRE_THROWS_AS( BitArchiveReader( lib, BIT7Z_STRING( "invalid" ) ), BitException );
}

TEST_CASE( "formatdetect: Format detection of archive with a wrong extension (Issue #134)", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    const Bit7zLibrary lib{ test::sevenzipLibPath() };

    auto testFile = GENERATE(
        as< tstring >(),
        BIT7Z_STRING( "wrong_extension.rar" ),
        BIT7Z_STRING( "wrong_extension.bz2" )
    );

    DYNAMIC_SECTION( "Reading file with a wrong extension: " << Catch::StringMaker< tstring >::convert( testFile ) ) {
        // From file
        REQUIRE_THROWS( BitArchiveReader( lib, testFile, BitFormat::Rar ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, testFile, BitFormat::SevenZip ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, testFile ) );

        // From buffer
        auto fileBuffer = loadFile( testFile );
        REQUIRE_THROWS( BitArchiveReader( lib, fileBuffer, BitFormat::Rar ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, fileBuffer, BitFormat::SevenZip ) );
        REQUIRE_NOTHROW( BitArchiveReader( lib, fileBuffer ) );
    }
}

#ifdef BIT7Z_DETECT_FROM_EXTENSION
TEST_CASE( "formatdetect: Format detection of an executable with a wrong extension", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

    // PE executables given a wrong, concrete (non-executable) extension. Extension-based detection
    // guesses the wrong format and fails to open, so bit7z falls back to signature detection and the
    // SFX scan: an SFX resolves to its embedded archive, a plain executable to the PE wrapper.
    struct WrongExtensionExe {
        tstring filename;
        const BitInFormat& format;
    };
    const auto test = GENERATE(
        WrongExtensionExe{ BIT7Z_STRING( "wrong_extension_sfx.gz" ), BitFormat::SevenZip }, // embedded archive
        WrongExtensionExe{ BIT7Z_STRING( "wrong_extension.7z" ), BitFormat::Pe }            // no archive -> wrapper
    );

    DYNAMIC_SECTION( Catch::StringMaker< tstring >::convert( test.filename ) ) {
        const BitArchiveReader reader{ test::sevenzipLib(), test.filename };
        REQUIRE( reader.detectedFormat() == test.format );
        REQUIRE( reader.itemsCount() > 0 );
    }
}
#endif

TEST_CASE( "formatdetect: Format detection of an archive file without an extension", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" };

#ifdef BIT7Z_DETECT_FROM_EXTENSION
    REQUIRE( detectFormatFromExtension( "noextension" ) == BitFormat::Auto );
#endif

    const BitArchiveReader reader{ test::sevenzipLib(), BIT7Z_STRING( "noextension" ) };
    REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
    REQUIRE_NOTHROW( reader.test() );
}

TEST_CASE( "formatdetect: Format detection of PE SFX archives", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "sfx" / "exe" };

    const auto test = GENERATE(
        TestInputFormat{ "7z.exe",  BitFormat::SevenZip },
        TestInputFormat{ "rar.exe", BitFormat::Rar5 },
        TestInputFormat{ "cab.exe", BitFormat::Cab },
        TestInputFormat{ "zip.exe", BitFormat::Zip },
        TestInputFormat{ "rar.zip.exe", BitFormat::Zip }
    );

    DYNAMIC_SECTION( test.extension ) {
        REQUIRE_LOAD_FILE( sfxBuffer, "sfx." + test.extension );
        const BitArchiveReader reader{ test::sevenzipLib(), sfxBuffer };
        REQUIRE( reader.detectedFormat() == test.format );
        REQUIRE( reader.itemsCount() > 0 );
    }
}

TEST_CASE( "formatdetect: Format detection of encrypted SFX archives", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "sfx" / "exe" };

    const auto test = GENERATE(
        TestInputFormat{ "7z.exe",  BitFormat::SevenZip },
        TestInputFormat{ "rar.exe", BitFormat::Rar5 }
    );

    DYNAMIC_SECTION( test.extension ) {
        REQUIRE_LOAD_FILE( sfxBuffer, "encrypted_sfx." + test.extension );

        SECTION( "Without a password, opening reports an encrypted-archive error" ) {
            // The SFX scan finds the embedded header-encrypted archive; its handler asks for a password
            // during Open, so tryOpenSfxArchive stops and surfaces an encrypted-open error.
            REQUIRE_THROWS_CODE(
                BitArchiveReader( test::sevenzipLib(), sfxBuffer ),
                make_error_code( OperationResult::OpenErrorEncrypted )
            );
        }

        SECTION( "With the correct password, the embedded archive is detected" ) {
            const BitArchiveReader reader{
                test::sevenzipLib(),
                sfxBuffer,
                BitFormat::Auto,
                BIT7Z_STRING( "password" )
            };
            REQUIRE( reader.detectedFormat() == test.format );
            REQUIRE( reader.itemsCount() > 0 );
        }
    }
}

TEST_CASE( "formatdetect: SFX detection is suppressed with the FileStart offset", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "sfx" / "exe" };

    const auto filename = GENERATE(
        as< std::string >(),
        "sfx.7z.exe",
        "sfx.rar.exe",
        "sfx.cab.exe",
        "sfx.zip.exe",
        "sfx.rar.zip.exe"
    );

    DYNAMIC_SECTION( filename ) {
        REQUIRE_LOAD_FILE( sfxBuffer, filename );
        // With the FileStart offset, bit7z must not scan for an embedded archive, so the executable
        // wrapper is detected instead of the SFX payload (the opposite of the default None behavior).
        const BitArchiveReader reader{ test::sevenzipLib(), sfxBuffer, ArchiveStartOffset::FileStart };
        REQUIRE( reader.detectedFormat() == BitFormat::Pe );
        REQUIRE( reader.itemsCount() > 0 );
    }
}

TEST_CASE( "formatdetect: Format detection of ELF SFX archives", "[formatdetect]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "sfx" / "elf" };

    const auto filename = GENERATE(
        as< std::string >(),
        "sfx.7z.elf",
        "sfx.p7zip.7z.elf",
        "sfx.peazip.7z.elf"
    );

    DYNAMIC_SECTION( filename ) {
        REQUIRE_LOAD_FILE( sfxBuffer, filename );
        const BitArchiveReader reader{ test::sevenzipLib(), sfxBuffer };
        REQUIRE( reader.detectedFormat() == BitFormat::SevenZip );
        REQUIRE( reader.itemsCount() > 0 );
    }
}

#endif // BIT7Z_AUTO_FORMAT
