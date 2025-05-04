// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/archive.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitformat.hpp>
#include <bit7z/bitfileextractor.hpp>

#include "utils/crc.hpp"

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

namespace {
auto to_string( FilterPolicy policy ) -> std::string {
    switch ( policy ) {
        case FilterPolicy::Exclude:
            return "FilterPolicy::Exclude";
        default:
        case FilterPolicy::Include:
            return "FilterPolicy::Include";
    }
}
} // namespace

TEST_CASE( "BitFileExtractor: using an empty wildcard pattern should throw (filesystem output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitfileextractor" };
        REQUIRE_THROWS( extractor.extractMatching( to_tstring( arcFileName ), tstring{}, testOutDir, policy ) );
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: using an empty wildcard pattern should throw (buffer output)", "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        buffer_t outBuffer;
        REQUIRE_THROWS( extractor.extractMatching( to_tstring( arcFileName ), tstring{}, outBuffer, policy ) );
        REQUIRE( outBuffer.empty() );
    }
}

TEST_CASE( "BitFileExtractor: non-matching wildcard pattern (filesystem output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const auto *const pattern = BIT7Z_STRING( "non-matching" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const auto arcFileNameStr = to_tstring( arcFileName );
        const TempDirectory outDir{ "test_bitfileextractor" };
        REQUIRE_THROWS( extractor.extractMatching( arcFileNameStr, pattern, outDir, FilterPolicy::Include ) );
        REQUIRE( fs::is_empty( outDir.path() ) );

        REQUIRE_NOTHROW( extractor.extractMatching( arcFileNameStr, pattern, outDir, FilterPolicy::Exclude ) );
        REQUIRE_FALSE( fs::is_empty( outDir.path() ) );

        for ( const auto& item : multiple_items_content().items ) {
            const auto expected = outDir.path() / item.inArchivePath;
            REQUIRE( fs::exists( expected ) );
            if ( item.fileInfo.type != fs::file_type::directory ) {
                REQUIRE( fs::is_regular_file( expected ) );
                REQUIRE( crc32( load_file( expected ) ) == item.fileInfo.crc32 );
            } else {
                REQUIRE( fs::is_directory( expected ) );
            }
            REQUIRE( fs::remove( expected ) );
        }

        REQUIRE( fs::is_empty( outDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: non-matching wildcard pattern (buffer output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const auto *const pattern = BIT7Z_STRING( "non-matching" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const auto arcFileNameStr = to_tstring( arcFileName );
        buffer_t outBuffer;
        REQUIRE_THROWS( extractor.extractMatching( arcFileNameStr, pattern, outBuffer, FilterPolicy::Include ) );
        REQUIRE( outBuffer.empty() );

        REQUIRE_NOTHROW( extractor.extractMatching( arcFileNameStr, pattern, outBuffer, FilterPolicy::Exclude ) );
        REQUIRE_FALSE( outBuffer.empty() );
    }
}

TEST_CASE( "BitFileExtractor: extracting only pdf files using a wildcard pattern (filesystem output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const auto arcFileNameStr = to_tstring( arcFileName );
        const TempDirectory outDir{ "test_bitfileextractor" };
        REQUIRE_NOTHROW( extractor.extractMatching( arcFileNameStr, BIT7Z_STRING( "*.pdf" ), outDir, policy ) );
        REQUIRE_FALSE( fs::is_empty( outDir.path() ) );

        const auto expected1 = outDir.path() / loremIpsum.name;
        const auto expected2 = outDir.path() / folder.name / subfolder2.name / quickBrown.name;
        if ( policy == FilterPolicy::Include ) {
            REQUIRE( fs::exists( expected1 ) );
            REQUIRE( crc32( load_file( expected1 ) ) == loremIpsum.crc32 );
            REQUIRE( fs::remove( expected1 ) );

            REQUIRE( fs::exists( expected2 ) );
            REQUIRE( crc32( load_file( expected2 ) ) == quickBrown.crc32 );
            REQUIRE( fs::remove( expected2 ) );
            REQUIRE( fs::remove( expected2.parent_path() ) );
            REQUIRE( fs::remove( expected2.parent_path().parent_path() ) );
        } else {
            REQUIRE_FALSE( fs::exists( expected1 ) );
            REQUIRE_FALSE( fs::exists( expected2 ) );

            for ( const auto& item : multiple_items_content().items ) {
                if ( item.fileInfo.name == loremIpsum.name || item.fileInfo.name == quickBrown.name ) {
                    continue;
                }
                const auto expected = outDir.path() / item.inArchivePath;
                INFO( expected )
                REQUIRE( fs::exists( expected ) );
                if ( item.fileInfo.type != fs::file_type::directory ) {
                    REQUIRE( fs::is_regular_file( expected ) );
                    REQUIRE( crc32( load_file( expected ) ) == item.fileInfo.crc32 );
                } else {
                    REQUIRE( fs::is_directory( expected ) );
                }
                REQUIRE( fs::remove( expected ) );
            }
        }

        REQUIRE( fs::is_empty( outDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: extracting only pdf files using a wildcard pattern (buffer output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;
        const auto wildcardPattern = BIT7Z_STRING( "*.pdf" );

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        buffer_t outBuffer;
        REQUIRE_NOTHROW( extractor.extractMatching( to_tstring( arcFileName ), wildcardPattern, outBuffer, policy ) );
        REQUIRE_FALSE( outBuffer.empty() );

        const auto bufferCrc = crc32( outBuffer );
        if ( policy == FilterPolicy::Include ) {
            // Note: bit7z does not guarantee the order of extraction,
            // so in the buffer there might be either of the two PDFs.
            // Catch2 doesn't support OR expressions inside REQUIRE,
            // so we store the result in a variable and check that.
            const auto isValidPdf = bufferCrc == loremIpsum.crc32 || bufferCrc == quickBrown.crc32;
            REQUIRE( isValidPdf );
        } else {
            REQUIRE( bufferCrc != loremIpsum.crc32 );
            REQUIRE( bufferCrc != quickBrown.crc32 );
        }
    }
}

#ifdef BIT7Z_REGEX_MATCHING

TEST_CASE( "BitFileExtractor: using an empty regex pattern should throw (filesystem output)", "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const TempTestDirectory testOutDir{ "test_bitfileextractor" };
        REQUIRE_THROWS( extractor.extractMatchingRegex( to_tstring( arcFileName ), tstring{}, testOutDir, policy ) );
        REQUIRE( fs::is_empty( testOutDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: using an empty regex pattern should throw (buffer output)", "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        buffer_t outBuffer;
        REQUIRE_THROWS( extractor.extractMatchingRegex( to_tstring( arcFileName ), tstring{}, outBuffer, policy ) );
        REQUIRE( outBuffer.empty() );
    }
}

TEST_CASE( "BitFileExtractor: non-matching regex pattern (filesystem output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const auto *const pattern = BIT7Z_STRING( "^[0-9]+$" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const auto arcFileNameStr = to_tstring( arcFileName );
        const TempDirectory outDir{ "test_bitfileextractor" };
        REQUIRE_THROWS( extractor.extractMatchingRegex( arcFileNameStr, pattern, outDir, FilterPolicy::Include ) );
        REQUIRE( fs::is_empty( outDir.path() ) );

        REQUIRE_NOTHROW( extractor.extractMatchingRegex( arcFileNameStr, pattern, outDir, FilterPolicy::Exclude ) );
        REQUIRE_FALSE( fs::is_empty( outDir.path() ) );

        for ( const auto& item : multiple_items_content().items ) {
            const auto expected = outDir.path() / item.inArchivePath;
            REQUIRE( fs::exists( expected ) );
            if ( item.fileInfo.type != fs::file_type::directory ) {
                REQUIRE( crc32( load_file( expected ) ) == item.fileInfo.crc32 );
            } else {
                REQUIRE( fs::is_directory( expected ) );
            }
            REQUIRE( fs::remove( expected ) );
        }

        REQUIRE( fs::is_empty( outDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: non-matching regex pattern (buffer output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const auto *const pattern = BIT7Z_STRING( "^[0-9]+$" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const auto arcFileNameStr = to_tstring( arcFileName );
        buffer_t outBuffer;
        REQUIRE_THROWS( extractor.extractMatchingRegex( arcFileNameStr, pattern, outBuffer, FilterPolicy::Include ) );
        REQUIRE( outBuffer.empty() );

        REQUIRE_NOTHROW( extractor.extractMatchingRegex( arcFileNameStr, pattern, outBuffer, FilterPolicy::Exclude ) );
        REQUIRE_FALSE( outBuffer.empty() );
    }
}

TEST_CASE( "BitFileExtractor: extracting only pdf files using a regex pattern (filesystem output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const tstring pattern = BIT7Z_STRING( R"(^(.*)\.pdf$)" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        const TempDirectory outDir{ "test_bitfileextractor" };
        REQUIRE_NOTHROW( extractor.extractMatchingRegex( to_tstring( arcFileName ), pattern, outDir, policy ) );
        REQUIRE_FALSE( fs::is_empty( outDir.path() ) );

        const auto expected1 = outDir.path() / loremIpsum.name;
        const auto expected2 = outDir.path() / folder.name / subfolder2.name / quickBrown.name;
        if ( policy == FilterPolicy::Include ) {
            REQUIRE( fs::exists( expected1 ) );
            REQUIRE( crc32( load_file( expected1 ) ) == loremIpsum.crc32 );
            REQUIRE( fs::remove( expected1 ) );

            REQUIRE( fs::exists( expected2 ) );
            REQUIRE( crc32( load_file( expected2 ) ) == quickBrown.crc32 );
            REQUIRE( fs::remove( expected2 ) );
            REQUIRE( fs::remove( expected2.parent_path() ) );
            REQUIRE( fs::remove( expected2.parent_path().parent_path() ) );
        } else {
            REQUIRE_FALSE( fs::exists( expected1 ) );
            REQUIRE_FALSE( fs::exists( expected2 ) );

            for ( const auto& item : multiple_items_content().items ) {
                if ( item.fileInfo.name == loremIpsum.name || item.fileInfo.name == quickBrown.name ) {
                    continue;
                }
                const auto expected = outDir.path() / item.inArchivePath;
                INFO( expected )
                REQUIRE( fs::exists( expected ) );
                if ( item.fileInfo.type != fs::file_type::directory ) {
                    REQUIRE( fs::is_regular_file( expected ) );
                    REQUIRE( crc32( load_file( expected ) ) == item.fileInfo.crc32 );
                } else {
                    REQUIRE( fs::is_directory( expected ) );
                }
                REQUIRE( fs::remove( expected ) );
            }
        }

        REQUIRE( fs::is_empty( outDir.path() ) );
    }
}

TEST_CASE( "BitFileExtractor: extracting only pdf files using a regex pattern (buffer output)",
           "[bitfileextractor]" ) {
    const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    const auto policy = GENERATE( as< FilterPolicy >(), FilterPolicy::Include, FilterPolicy::Exclude );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension << ", policy: " << to_string( policy ) ) {
        const tstring pattern = BIT7Z_STRING( R"(^(.*)\.pdf$)" );
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        const BitFileExtractor extractor( test::sevenzip_lib(), testArchive.format );

        buffer_t outBuffer;
        REQUIRE_NOTHROW( extractor.extractMatchingRegex( to_tstring( arcFileName ), pattern, outBuffer, policy ) );
        REQUIRE_FALSE( outBuffer.empty() );

        const auto bufferCrc = crc32( outBuffer );
        if ( policy == FilterPolicy::Include ) {
            // Note: bit7z does not guarantee the order of extraction,
            // so in the buffer there might be either of the two PDFs.
            // Catch2 doesn't support OR expressions inside REQUIRE,
            // so we store the result in a variable and check that.
            const auto isValidPdf = bufferCrc == loremIpsum.crc32 || bufferCrc == quickBrown.crc32;
            REQUIRE( isValidPdf );
        } else {
            REQUIRE( bufferCrc != loremIpsum.crc32 );
            REQUIRE( bufferCrc != quickBrown.crc32 );
        }
    }
}

#endif