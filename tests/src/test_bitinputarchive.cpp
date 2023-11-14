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

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "utils/archive.hpp"
#include "utils/crc.hpp"
#include "utils/filesystem.hpp"
#include "utils/format.hpp"
#include "utils/shared_lib.hpp"
#include "utils/source_location.hpp"

#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/fs.hpp>

#include <cstddef>
#include <cstdint>
#include <sstream>

using namespace bit7z;
using namespace bit7z::test;
using namespace bit7z::test::filesystem;

void require_archive_extracts( const BitArchiveReader& info, const source_location& location ) {
    INFO( "Failed while extracting the archive ");
    INFO( "  from " << location.file_name() << ":" << location.line() );
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif

    SECTION( "Extracting to a map of buffers" ) {
        std::map< tstring, buffer_t > bufferMap;
        REQUIRE_NOTHROW( info.extractTo( bufferMap ) );
        REQUIRE( bufferMap.size() == info.filesCount() );
        if ( !format_has_crc( info.detectedFormat() ) ) {
            return;
        }
        for( const auto& entry : bufferMap ) {
            auto item = info.find( entry.first );
            REQUIRE( item != info.cend() );
            const auto item_crc = item->crc();
            if ( item_crc > 0 ) {
                if ( info.detectedFormat() == BitFormat::Lzh ) {
                    REQUIRE( crc16( entry.second ) == item_crc );
                } else if ( info.detectedFormat() != BitFormat::Rar5 || !item->isEncrypted() ) {
                    REQUIRE( crc32( entry.second ) == item_crc );
                }
            }
        }
    }

    SECTION( "Extracting each item to a buffer" ) {
        buffer_t outputBuffer;
        for ( const auto& item : info ) {
            INFO( "Item at the index " << item.index() << " named '" << tstring_to_path( item.name() ).u8string() << "'" );
            if ( item.isDir() ) {
                REQUIRE_THROWS( info.extractTo( outputBuffer, item.index() ) );
                REQUIRE( outputBuffer.empty() );
            } else {
                REQUIRE_NOTHROW( info.extractTo( outputBuffer, item.index() ) );
                const auto item_crc = item.crc();
                if ( item_crc > 0 ) {
                    if ( info.detectedFormat() == BitFormat::Lzh ) {
                        REQUIRE( crc16( outputBuffer ) == item_crc );
                    } else if ( info.detectedFormat() != BitFormat::Rar5 || !item.isEncrypted() ) {
                        REQUIRE( crc32( outputBuffer ) == item_crc );
                    }
                } else if ( info.detectedFormat() != BitFormat::Chm && info.detectedFormat() != BitFormat::Elf &&
                            info.detectedFormat() != BitFormat::Macho && info.detectedFormat() != BitFormat::Ntfs &&
                            info.detectedFormat() != BitFormat::Swf && (info.detectedFormat() != BitFormat::Wim || item.isSymLink()) ) {
                    // TODO: Check why these formats can't be extracted to a buffer
                    REQUIRE_FALSE( outputBuffer.empty() );
                }
            }
            outputBuffer.clear();
        }

        {
            buffer_t dummyBuffer;
            REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() ) );
            REQUIRE( dummyBuffer.empty() );

            REQUIRE_THROWS( info.extractTo( dummyBuffer, info.itemsCount() + 1 ) );
            REQUIRE( dummyBuffer.empty() );
        }
    }


    SECTION( "Extracting each item to a fixed size buffer" ) {
        // Note: this value must be different from any file size we can encounter inside the tested archives.
        constexpr size_t dummyBufferSize = 42;
        buffer_t dummyBuffer2( dummyBufferSize, static_cast< byte_t >( '\0' ) );
        buffer_t outputBuffer;
        for ( const auto& item : info ) {
            const auto itemIndex = item.index();
            REQUIRE_THROWS( info.extractTo( nullptr, 0, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( nullptr, dummyBufferSize, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), itemIndex ) );

            REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), 0, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), dummyBufferSize, itemIndex ) );
            REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), std::numeric_limits< std::size_t >::max(), itemIndex ) );

            if ( !item.isDir() ) {
                const auto itemSize = item.size();
                REQUIRE_THROWS( info.extractTo( nullptr, itemSize, itemIndex ) );

                if ( itemSize > 0 ) {
                    outputBuffer.resize( itemSize, static_cast< byte_t >( '\0' ) );
                    REQUIRE_NOTHROW( info.extractTo( outputBuffer.data(), itemSize, itemIndex ) );

                    const auto item_crc = item.crc();
                    if ( item_crc > 0 ) {
                        if ( info.detectedFormat() == BitFormat::Lzh ) {
                            REQUIRE( crc16( outputBuffer ) == item_crc );
                        } else if ( info.detectedFormat() != BitFormat::Rar5 || !item.isEncrypted() ) {
                            REQUIRE( crc32( outputBuffer ) == item_crc );
                        }
                    }
                } else {
                    REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), itemSize, itemIndex ) );
                }
                outputBuffer.clear();
            }
        }

        REQUIRE_THROWS( info.extractTo( nullptr, 0, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( nullptr, dummyBufferSize, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( nullptr, std::numeric_limits< std::size_t >::max(), info.itemsCount() ) );

        REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), 0, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(), dummyBufferSize, info.itemsCount() ) );
        REQUIRE_THROWS( info.extractTo( dummyBuffer2.data(),
                                        std::numeric_limits< std::size_t >::max(),
                                        info.itemsCount() ) );
    }

    SECTION( "Extracting each item to std::ostream" ) {
        for ( const auto& item : info ) {
            INFO( "Item at the index " << item.index() << " named '" << tstring_to_path( item.name() ).u8string() << "'" );
            std::ostringstream outputStream;
            if ( item.isDir() ) {
                REQUIRE_THROWS( info.extractTo( outputStream, item.index() ) );
                REQUIRE( outputStream.str().empty() );
            } else {
                REQUIRE_NOTHROW( info.extractTo( outputStream, item.index() ) );
                const auto item_crc = item.crc();
                if ( item_crc > 0 ) {
                    if ( info.detectedFormat() == BitFormat::Lzh ) {
                        REQUIRE( crc16( outputStream.str() ) == item_crc );
                    } else if ( info.detectedFormat() != BitFormat::Rar5 || !item.isEncrypted() ) {
                        REQUIRE( crc32( outputStream.str() ) == item_crc );
                    }
                } else if ( info.detectedFormat() != BitFormat::Wim || item.isSymLink() ) {
                    REQUIRE_FALSE( outputStream.str().empty() );
                }
            }
        }

        {
            std::ostringstream outputStream;
            REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() ) );
            REQUIRE( outputStream.str().empty() );

            REQUIRE_THROWS( info.extractTo( outputStream, info.itemsCount() + 1 ) );
            REQUIRE( outputStream.str().empty() );
        }
    }
}

#define REQUIRE_ARCHIVE_EXTRACTS( info ) \
    require_archive_extracts( info, BIT7Z_CURRENT_LOCATION )

void require_archive_tests( const BitArchiveReader& info, const source_location& location ) {
    INFO( "Failed while testing the archive ");
    INFO( "  from " << location.file_name() << ":" << location.line() );
#ifdef BIT7Z_BUILD_FOR_P7ZIP
    const auto& detectedFormat = (info).detectedFormat();
    if ( detectedFormat == BitFormat::Rar || detectedFormat == BitFormat::Rar5 ) {
        return;
    }
#endif
    REQUIRE_NOTHROW( info.test() );
    for ( uint32_t index = 0; index < info.itemsCount(); ++index ) {
        REQUIRE_NOTHROW( info.testItem( index ) );
    }
    REQUIRE_THROWS_AS( info.testItem( info.itemsCount() ), BitException );
}

#define REQUIRE_ARCHIVE_TESTS( info ) \
    require_archive_tests( info, BIT7Z_CURRENT_LOCATION )

/* Note: throughout this unit test we will use BitArchiveReader for testing BitInputArchive's specific methods. */

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing only a single file",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "bz2", BitFormat::BZip2 },
                                      TestInputFormat{ "gz", BitFormat::GZip },
                                      TestInputFormat{ "iso", BitFormat::Iso },
                                      TestInputFormat{ "lzh", BitFormat::Lzh },
                                      TestInputFormat{ "lzma", BitFormat::Lzma },
                                      TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                      TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "xz", BitFormat::Xz },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + testFormat.extension );

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing multiple files",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_files" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "iso", BitFormat::Iso },
                                      TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "multiple_files." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing multiple items (files and folders)",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "multiple_items" };

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "multiple_items." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting archives containing encrypted items",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                      TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                      TestInputFormat{ "aes256.zip", BitFormat::Zip },
                                      TestInputFormat{ "zipcrypto.zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password should allow make testing and extraction throw" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
            REQUIRE_THROWS( info.test() );

            std::map< tstring, buffer_t > dummyMap;
            REQUIRE_THROWS( info.extractTo( dummyMap ) );
            for ( const auto& entry : dummyMap ) {
                // TODO: Check if extractTo should not write or clear the map when the extraction fails
                REQUIRE( entry.second.empty() );
            }
        }

        SECTION( "Opening the archive with the correct password should allow testing and extraction without issues" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info );
        }
    }
}

/* Pull request #36 */
TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting header-encrypted archives",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "header_encrypted" };

    const auto* const password = BIT7Z_STRING( "helloworld" );

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                      TestInputFormat{ "rar5.rar", BitFormat::Rar5 } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "header_encrypted." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );

        SECTION( "Opening the archive with no password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testFormat.format ) );
        }

        SECTION( "Opening the archive with a wrong password should throw an exception" ) {
            REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, testFormat.format,
                                              BIT7Z_STRING( "wrong_password" ) ) );
        }

        SECTION( "Opening the archive with the correct password should pass the tests" ) {
            const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format, password );
            REQUIRE_ARCHIVE_TESTS( info );
            REQUIRE_ARCHIVE_EXTRACTS( info );
        }
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting multi-volume archives", "[bitinputarchive]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "split" };

    SECTION( "Split archive (non-RAR)" ) {
        const auto testFormat = GENERATE( as< TestInputFormat >(),
                                          TestInputFormat{ "7z", BitFormat::SevenZip },
                                          TestInputFormat{ "bz2", BitFormat::BZip2 },
                                          TestInputFormat{ "gz", BitFormat::GZip },
                                          TestInputFormat{ "tar", BitFormat::Tar },
                                          TestInputFormat{ "wim", BitFormat::Wim },
                                          TestInputFormat{ "xz", BitFormat::Xz },
                                          TestInputFormat{ "zip", BitFormat::Zip } );

        DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
            const fs::path arcFileName = "clouds.jpg." + testFormat.extension + ".001";

            INFO( "Archive file: " << arcFileName );

            SECTION( "Opening as a split archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Split );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info );
            }

            SECTION( "Opening as a whole archive" ) {
                const BitArchiveReader info( test::sevenzip_lib(),
                                             arcFileName.string< tchar >(),
                                             testFormat.format );
                REQUIRE_ARCHIVE_TESTS( info );
                REQUIRE_ARCHIVE_EXTRACTS( info );
            }
        }
    }

    SECTION( "Multi-volume RAR5" ) {
        const fs::path arcFileName = "clouds.jpg.part1.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar5 );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Multi-volume RAR4" ) {
        const fs::path arcFileName = "clouds.jpg.rar";
        const BitArchiveReader info( test::sevenzip_lib(), arcFileName.string< tchar >(), BitFormat::Rar );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an empty archive",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "empty" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
    // TestInputFormat{ "tar", BitFormat::Tar, 0 }, // TODO: Check why it fails opening
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "empty." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEST_CASE( "BitInputArchive: Testing and extracting solid archives", "[bitinputarchive]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "solid" };

    SECTION( "Solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "solid.rar" ), BitFormat::Rar5 );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Non solid 7z" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.7z" ), BitFormat::SevenZip );
        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }

    SECTION( "Non-solid RAR" ) {
        const BitArchiveReader info( test::sevenzip_lib(), BIT7Z_STRING( "non_solid.rar" ), BitFormat::Rar5 );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

/**
 * Tests opening an archive file using the RAR format
 * (or throws a BitException if it is not a RAR archive at all).
 */
auto test_open_rar_archive( const Bit7zLibrary& lib, const tstring& inFile ) -> const BitInFormat& {
    try {
        const BitArchiveReader info( lib, inFile, BitFormat::Rar );
        //if BitArchiveReader constructor did not throw an exception, the archive is RAR (< 5.0).
        return BitFormat::Rar;
    } catch ( const BitException& ) {
        /* the archive is not a RAR, and if it is not even a RAR5,
           the following line will throw an exception (which we do not catch). */
        const BitArchiveReader info( lib, inFile, BitFormat::Rar5 );
        return BitFormat::Rar5;
    }
}

TEST_CASE( "BitArchiveReader: Opening RAR archives using the correct RAR format version", "[bitarchivereader]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "detection" / "valid" };

    SECTION( "Valid RAR archives" ) {
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar4.rar" ) ) == BitFormat::Rar );
        REQUIRE( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.rar5.rar" ) ) == BitFormat::Rar5 );
    }

    SECTION( "Non-RAR archive" ) {
        REQUIRE_THROWS( test_open_rar_archive( test::sevenzip_lib(), BIT7Z_STRING( "valid.zip" ) ) );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting invalid archives should throw",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "testing" };

    // The italy.svg file in the ko_test archives is different from the one used for filesystem tests
    static constexpr auto italy_ko_crc32 = 0x2ADFB3AF;

    const auto testArchive = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "rar", BitFormat::Rar5 },
                                       //TestInputFormat{"tar", BitFormat::Tar},
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testArchive.extension ) {
        const fs::path arcFileName = "ko_test." + testArchive.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testArchive.format );
        REQUIRE_THROWS( info.test() );

        std::map< tstring, buffer_t > dummyMap;
        REQUIRE_THROWS( info.extractTo( dummyMap ) );
        // TODO: Check if extractTo should not write or clear the map when the extraction fails

        buffer_t outputBuffer;
        std::ostringstream outputStream;
        if ( info.itemsCount() == 1 ) {
            REQUIRE_THROWS( info.extractTo( outputBuffer, 0 ) );
            REQUIRE( outputBuffer.empty() );

            REQUIRE_THROWS( info.extractTo( outputStream, 0 ) );
            // Note: we might have written some data to the stream before 7-zip failed!
        } else if ( info.itemsCount() == 2 ) {
#ifndef BIT7Z_BUILD_FOR_P7ZIP
            REQUIRE_NOTHROW( info.extractTo( outputBuffer, 0 ) );
            REQUIRE( crc32( outputBuffer ) == italy_ko_crc32 );

            REQUIRE_NOTHROW( info.extractTo( outputStream, 0 ) );
            REQUIRE( crc32( outputStream.str() ) == italy_ko_crc32 );
#endif

            outputBuffer.clear();
            REQUIRE_THROWS( info.extractTo( outputBuffer, 1 ) );
            REQUIRE( outputBuffer.empty() );

            outputStream.str("");
            outputStream.clear();
            REQUIRE_THROWS( info.extractTo( outputStream, 1 ) );
            // Note: we might have written some data to the stream before 7-zip failed!
        }
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Reading archives using the wrong format should throw",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "extraction" / "single_file" };

    const auto correctFormat = GENERATE( as< TestInputFormat >(),
                                         TestInputFormat{ "7z", BitFormat::SevenZip },
                                         TestInputFormat{ "bz2", BitFormat::BZip2 },
                                         TestInputFormat{ "gz", BitFormat::GZip },
                                         TestInputFormat{ "iso", BitFormat::Iso },
                                         TestInputFormat{ "lzh", BitFormat::Lzh },
                                         TestInputFormat{ "lzma", BitFormat::Lzma },
                                         TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                         TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                         TestInputFormat{ "tar", BitFormat::Tar },
                                         TestInputFormat{ "wim", BitFormat::Wim },
                                         TestInputFormat{ "xz", BitFormat::Xz },
                                         TestInputFormat{ "zip", BitFormat::Zip } );

    const auto wrongFormat = GENERATE( as< TestInputFormat >(),
                                       TestInputFormat{ "7z", BitFormat::SevenZip },
                                       TestInputFormat{ "bz2", BitFormat::BZip2 },
                                       TestInputFormat{ "gz", BitFormat::GZip },
                                       TestInputFormat{ "iso", BitFormat::Iso },
                                       TestInputFormat{ "lzh", BitFormat::Lzh },
                                       TestInputFormat{ "lzma", BitFormat::Lzma },
                                       TestInputFormat{ "rar4.rar", BitFormat::Rar },
                                       TestInputFormat{ "rar5.rar", BitFormat::Rar5 },
                                       TestInputFormat{ "tar", BitFormat::Tar },
                                       TestInputFormat{ "wim", BitFormat::Wim },
                                       TestInputFormat{ "xz", BitFormat::Xz },
                                       TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << correctFormat.extension ) {
        const auto arcFileName = fs::path{ clouds.name }.concat( "." + correctFormat.extension );

        if ( correctFormat.extension != wrongFormat.extension ) {
            DYNAMIC_SECTION( "Wrong format: " << wrongFormat.extension ) {
                TestType inputArchive{};
                getInputArchive( arcFileName, inputArchive );
                REQUIRE_THROWS( BitArchiveReader( test::sevenzip_lib(), inputArchive, wrongFormat.format ) );
            }
        }
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with different file types inside",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "file_type" };

    // Note: for some reason, 7-Zip fails to test or extract Rar archives containing symbolic links.

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      //TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "file_type." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );

        REQUIRE_ARCHIVE_TESTS( info );
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with Unicode items",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const auto testFormat = GENERATE( as< TestInputFormat >(),
                                      TestInputFormat{ "7z", BitFormat::SevenZip },
                                      TestInputFormat{ "rar", BitFormat::Rar5 },
                                      TestInputFormat{ "tar", BitFormat::Tar },
                                      TestInputFormat{ "wim", BitFormat::Wim },
                                      TestInputFormat{ "zip", BitFormat::Zip } );

    DYNAMIC_SECTION( "Archive format: " << testFormat.extension ) {
        const fs::path arcFileName = "unicode." + testFormat.extension;

        TestType inputArchive{};
        getInputArchive( arcFileName, inputArchive );
        const BitArchiveReader info( test::sevenzip_lib(), inputArchive, testFormat.format );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
        REQUIRE_ARCHIVE_TESTS( info );
#endif
        REQUIRE_ARCHIVE_EXTRACTS( info );
    }
}

TEMPLATE_TEST_CASE( "BitInputArchive: Testing and extracting an archive with a Unicode file name",
                    "[bitinputarchive]", tstring, buffer_t, stream_t ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "αρχείο.7z" ) };

    TestType inputArchive{};
    getInputArchive( arcFileName, inputArchive );
    const BitArchiveReader info( test::sevenzip_lib(), inputArchive, BitFormat::SevenZip );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
    REQUIRE_ARCHIVE_TESTS( info );
#endif
    REQUIRE_ARCHIVE_EXTRACTS( info );
}

TEST_CASE( "BitInputArchive: Testing and extracting an archive with a Unicode file name (bzip2)",
           "[bitinputarchive]" ) {
    static const TestDirectory testDir{ fs::path{ test_archives_dir } / "metadata" / "unicode" };

    const fs::path arcFileName{ BIT7Z_NATIVE_STRING( "クラウド.jpg.bz2" ) };
    const BitArchiveReader info( test::sevenzip_lib(), path_to_tstring( arcFileName ), BitFormat::BZip2 );
#ifndef BIT7Z_BUILD_FOR_P7ZIP
    REQUIRE_ARCHIVE_TESTS( info );
#endif
    REQUIRE_ARCHIVE_EXTRACTS( info );
}
#endif