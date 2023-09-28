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

#include "filesystem.hpp"

#include <bit7z/bitexception.hpp>
#include <bit7z/bititemsvector.hpp>
#include <internal/genericinputitem.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <map>

using std::wcout;
using std::endl;
using std::vector;
using std::map;

using bit7z::tstring;
using bit7z::BitException;
using bit7z::BitItemsVector;
using bit7z::IndexingOptions;
using namespace bit7z;
using namespace bit7z::filesystem;
using namespace bit7z::test::filesystem;

TEST_CASE( "BitItemsVector: Default properties", "[bititemsvector]" ) {
    const BitItemsVector itemsVector{};

    REQUIRE( itemsVector.size() == 0 );
    REQUIRE( itemsVector.cbegin() == itemsVector.cend() );
    REQUIRE( itemsVector.begin() == itemsVector.end() );
}

TEST_CASE( "BitItemsVector: Indexing an invalid directory (non-existing)", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( itemsVector.indexDirectory( "not_existing_path" ) );
}

TEST_CASE( "BitItemsVector: Indexing an invalid directory (empty string)", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( itemsVector.indexDirectory( "" ) );
}

auto in_archive_paths( const BitItemsVector& vector ) -> std::vector< fs::path > {
    std::vector< fs::path > paths;
    std::transform( vector.cbegin(), vector.cend(), std::back_inserter( paths ),
                    []( const auto& item ) {
                        return item->inArchivePath();
                    } );
    return paths;
}

struct TestDirectory {
    fs::path path;
    vector< fs::path > expectedItems;
};

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
TEST_CASE( "BitItemsVector: Indexing a valid directory", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_STRING( "σαράντα δύο.txt" ),
                "dot.folder",
                "dot.folder/hello.json",
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "empty" } },
        TestDirectory{ "./empty", { "empty" } },
        TestDirectory{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (only files)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.onlyFiles = true;

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_STRING( "σαράντα δύο.txt" ),
                "dot.folder/hello.json",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "empty" } },
        TestDirectory{ "./empty", { "empty" } },
        TestDirectory{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (retaining folder structure)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.retainFolderStructure = true;

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                ".",
                "./italy.svg",
                "./Lorem Ipsum.pdf",
                "./noext",
                BIT7Z_STRING( "./σαράντα δύο.txt" ),
                "./dot.folder",
                "./dot.folder/hello.json",
                "./empty",
                "./folder",
                "./folder/clouds.jpg",
                "./folder/subfolder",
                "./folder/subfolder2",
                "./folder/subfolder2/homework.doc",
                "./folder/subfolder2/The quick brown fox.pdf",
                "./folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "empty" } },
        TestDirectory{ "./empty", { "./empty" } },
        TestDirectory{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "./folder",
                "./folder/clouds.jpg",
                "./folder/subfolder",
                "./folder/subfolder2",
                "./folder/subfolder2/homework.doc",
                "./folder/subfolder2/The quick brown fox.pdf",
                "./folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "./folder/subfolder2",
                "./folder/subfolder2/homework.doc",
                "./folder/subfolder2/The quick brown fox.pdf",
                "./folder/subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (filtered)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    SECTION( "Wildcard filter *" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "dot.folder",
                    "dot.folder/hello.json",
                    "empty",
                    "folder",
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder",
                    "subfolder2",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "dot.folder/hello.json",
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "dot.folder",
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.*" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.folder" ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( ".", BIT7Z_STRING( "*.folder" ) ) );

        const std::vector< fs::path > expectedItems{ "dot.folder" };

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( expectedItems ) );
    }

    SECTION( "Wildcard filter *.folder (only files)" ) {
        // Even if we are indexing non-recursively, the "dot.folder" matches the filter; hence, it must be indexed!
        IndexingOptions options{};
        options.onlyFiles = true;

        REQUIRE_NOTHROW( itemsVector.indexDirectory( ".", BIT7Z_STRING( "*.folder" ),
                                                      FilterPolicy::Include, options ) );
        REQUIRE( itemsVector.size() == 0 );
    }

    SECTION( "Only PDF files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "Lorem Ipsum.pdf", "folder/subfolder2/The quick brown fox.pdf" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestDirectory{ "./folder", { "subfolder2/The quick brown fox.pdf" } },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestDirectory{ "./folder/subfolder2", { "The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.pdf" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "italy.svg" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", {} },
            TestDirectory{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.svg" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "folder/clouds.jpg" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/clouds.jpg" } },
            TestDirectory{ "./folder", { "clouds.jpg" } },
            TestDirectory{ "folder/subfolder2", {} },
            TestDirectory{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.jpg" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "folder/subfolder2/homework.doc" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/subfolder2/homework.doc" } },
            TestDirectory{ "./folder", { "subfolder2/homework.doc" } },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/homework.doc" } },
            TestDirectory{ "./folder/subfolder2", { "homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.doc" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "folder/subfolder2/frequency.xlsx" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/subfolder2/frequency.xlsx" } },
            TestDirectory{ "./folder", { "subfolder2/frequency.xlsx" } },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/frequency.xlsx" } },
            TestDirectory{ "./folder/subfolder2", { "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.xlsx" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only PNG files (no matching file)" ) {
        const auto testDirectory = GENERATE( as < fs::path > {},
                                              ".",
                                              "empty",
                                              "./empty",
                                              "folder",
                                              "./folder",
                                              "folder/subfolder2",
                                              "./folder/subfolder2" );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory, BIT7Z_STRING( "*.png" ) ) );
            REQUIRE( itemsVector.size() == 0 );
        }
    }

    SECTION( "Only non-PDF files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "italy.svg",
                    "noext",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "dot.folder",
                    "dot.folder/hello.json",
                    "empty",
                    "folder",
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder",
                    "subfolder2",
                    "subfolder2/homework.doc",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.pdf" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (excluding)" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", {} },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", {} },
            TestDirectory{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Empty filter (excluding)" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", {} },
            TestDirectory{ "empty", { "empty" } },
            TestDirectory{ "./empty", { "empty" } },
            TestDirectory{ "folder", { "folder" } },
            TestDirectory{ "./folder", { "folder" } },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2" } },
            TestDirectory{ "./folder/subfolder2", { "subfolder2" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (non-recursively, filtered)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    SECTION( "Wildcard filter *" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                    "dot.folder",
                    "dot.folder/hello.json",
                    "empty",
                    "folder",
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder",
                    "subfolder2",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            },
            TestDirectory{
                "../test_filesystem/folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        options.onlyFiles = true;

        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{
                ".",
                {
                    "dot.folder",
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_STRING( "σαράντα δύο.txt" ),
                }
            },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{
                "folder",
                {
                    "folder/clouds.jpg"
                }
            },
            TestDirectory{
                "./folder",
                {
                    "clouds.jpg"
                }
            },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            },
            TestDirectory{
                "../test_filesystem/folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        options.onlyFiles = true;

        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "italy.svg", "Lorem Ipsum.pdf", BIT7Z_STRING( "σαράντα δύο.txt" ), } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/clouds.jpg" } },
            TestDirectory{ "./folder", { "clouds.jpg" } },
            TestDirectory{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestDirectory{ "./folder/subfolder2", { "homework.doc", "The quick brown fox.pdf", "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.folder" ) {
        // Even if we are indexing non-recursively, the "dot.folder" matches the filter; hence, it must be indexed!
        REQUIRE_NOTHROW( itemsVector.indexDirectory( ".", BIT7Z_STRING( "*.folder" ),
                                                      FilterPolicy::Include, options ) );

        const std::vector< fs::path > expectedItems{ "dot.folder" };

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( expectedItems ) );
    }

    SECTION( "Wildcard filter *.folder (only files)" ) {
        options.onlyFiles = true;

        REQUIRE_NOTHROW( itemsVector.indexDirectory( ".", BIT7Z_STRING( "*.folder" ),
                                                      FilterPolicy::Include, options ) );
        REQUIRE( itemsVector.size() == 0 );
    }

    SECTION( "Only PDF files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "Lorem Ipsum.pdf" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestDirectory{ "./folder/subfolder2", { "The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.pdf" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", { "italy.svg" } },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", {} },
            TestDirectory{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.svg" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", {} },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", { "folder/clouds.jpg" } },
            TestDirectory{ "./folder", { "clouds.jpg" } },
            TestDirectory{ "folder/subfolder2", {} },
            TestDirectory{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.jpg" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", {} },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/homework.doc" } },
            TestDirectory{ "./folder/subfolder2", { "homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.doc" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testDirectory = GENERATE(
            TestDirectory{ ".", {} },
            TestDirectory{ "empty", {} },
            TestDirectory{ "./empty", {} },
            TestDirectory{ "folder", {} },
            TestDirectory{ "./folder", {} },
            TestDirectory{ "folder/subfolder2", { "folder/subfolder2/frequency.xlsx" } },
            TestDirectory{ "./folder/subfolder2", { "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "*.xlsx" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
        }
    }

    SECTION( "Only PNG files (no matching file)" ) {
        const auto testDirectory = GENERATE( as < fs::path > {},
                                              ".",
                                              "empty",
                                              "./empty",
                                              "folder",
                                              "./folder",
                                              "folder/subfolder2",
                                              "./folder/subfolder2" );

        DYNAMIC_SECTION ( "Indexing directory " << testDirectory ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory, BIT7Z_STRING( "*.png" ),
                                                          FilterPolicy::Include, options ) );
            REQUIRE( itemsVector.size() == 0 );
        }
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (non-recursively)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                "dot.folder",
                "dot.folder/hello.json",
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_STRING( "σαράντα δύο.txt" ),
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "empty" } },
        TestDirectory{ "./empty", { "empty" } },
        TestDirectory{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            "../test_filesystem",
            {
                "test_filesystem",
                "test_filesystem/italy.svg",
                "test_filesystem/Lorem Ipsum.pdf",
                "test_filesystem/noext",
                BIT7Z_STRING( "test_filesystem/σαράντα δύο.txt" ),
                "test_filesystem/dot.folder",
                "test_filesystem/dot.folder/hello.json",
                "test_filesystem/empty",
                "test_filesystem/folder",
                "test_filesystem/folder/clouds.jpg",
                "test_filesystem/folder/subfolder",
                "test_filesystem/folder/subfolder2",
                "test_filesystem/folder/subfolder2/homework.doc",
                "test_filesystem/folder/subfolder2/The quick brown fox.pdf",
                "test_filesystem/folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "../test_filesystem/empty", { "empty" } },
        TestDirectory{
            "../test_filesystem/folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path, non-recursively)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            "../test_filesystem",
            {
                "test_filesystem",
                "test_filesystem/italy.svg",
                "test_filesystem/Lorem Ipsum.pdf",
                "test_filesystem/noext",
                BIT7Z_STRING( "test_filesystem/σαράντα δύο.txt" ),
                "test_filesystem/dot.folder",
                "test_filesystem/dot.folder/hello.json",
                "test_filesystem/empty",
                "test_filesystem/folder",
                "test_filesystem/folder/clouds.jpg",
                "test_filesystem/folder/subfolder",
                "test_filesystem/folder/subfolder2",
                "test_filesystem/folder/subfolder2/homework.doc",
                "test_filesystem/folder/subfolder2/The quick brown fox.pdf",
                "test_filesystem/folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "../test_filesystem/empty", { "empty" } },
        TestDirectory{
            "../test_filesystem/folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testDirectory.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (custom path mapping)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                "custom_folder",
                "custom_folder/dot.folder",
                "custom_folder/dot.folder/hello.json",
                "custom_folder/italy.svg",
                "custom_folder/Lorem Ipsum.pdf",
                "custom_folder/noext",
                BIT7Z_STRING( "custom_folder/σαράντα δύο.txt" ),
                "custom_folder/empty",
                "custom_folder/folder",
                "custom_folder/folder/clouds.jpg",
                "custom_folder/folder/subfolder",
                "custom_folder/folder/subfolder2",
                "custom_folder/folder/subfolder2/homework.doc",
                "custom_folder/folder/subfolder2/The quick brown fox.pdf",
                "custom_folder/folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "custom_folder" } },
        TestDirectory{ "./empty", { "custom_folder" } },
        TestDirectory{
            "folder",
            {
                "custom_folder",
                "custom_folder/clouds.jpg",
                "custom_folder/subfolder",
                "custom_folder/subfolder2",
                "custom_folder/subfolder2/homework.doc",
                "custom_folder/subfolder2/The quick brown fox.pdf",
                "custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "custom_folder",
                "custom_folder/clouds.jpg",
                "custom_folder/subfolder",
                "custom_folder/subfolder2",
                "custom_folder/subfolder2/homework.doc",
                "custom_folder/subfolder2/The quick brown fox.pdf",
                "custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestDirectory{
            "../test_filesystem/folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestDirectory{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testDirectory.path.string< bit7z::tchar >(), BIT7Z_STRING( "custom_folder" ) }
        };
        REQUIRE_NOTHROW( itemsVector.indexPathsMap( pathMap ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (empty custom path mapping)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const auto testDirectory = GENERATE(
        TestDirectory{
            ".",
            {
                "dot.folder",
                "dot.folder/hello.json",
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_STRING( "σαράντα δύο.txt" ),
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{ "empty", { "empty" } },
        TestDirectory{ "./empty", { "empty" } },
        TestDirectory{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        },
        TestDirectory{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testDirectory.path ) {
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testDirectory.path.string< bit7z::tchar >(), BIT7Z_STRING( "" ) }
        };
        REQUIRE_NOTHROW( itemsVector.indexPathsMap( pathMap ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testDirectory.expectedItems ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}
#endif

struct TestPaths {
    vector< bit7z::tstring > inputPaths;
    vector< fs::path > expectedItems;
};

TEST_CASE( "BitItemsVector: Indexing a vector of paths", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestPaths{
            {
                BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                BIT7Z_STRING( "folder/clouds.jpg" ),
                BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" )
            },
            {
                "Lorem Ipsum.pdf",
                "folder/clouds.jpg",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                "italy.svg",
                "dot.folder/hello.json",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                "dot.folder",
                "dot.folder/hello.json",
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( itemsVector.indexPaths( testInput.inputPaths ) );

    const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (retaining folder structure)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.retainFolderStructure = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestPaths{
            {
                BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                BIT7Z_STRING( "folder/clouds.jpg" ),
                BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" )
            },
            {
                "Lorem Ipsum.pdf",
                "folder/clouds.jpg",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                "italy.svg",
                "dot.folder/hello.json",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                "dot.folder",
                "dot.folder/hello.json",
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( itemsVector.indexPaths( testInput.inputPaths, options ) );

    const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (only files)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.onlyFiles = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestPaths{
            {
                BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                BIT7Z_STRING( "folder/clouds.jpg" ),
                BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" )
            },
            {
                "Lorem Ipsum.pdf",
                "folder/clouds.jpg",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                "italy.svg",
                "dot.folder/hello.json",
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                "dot.folder",
                "dot.folder/hello.json",
                "empty",
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( itemsVector.indexPaths( testInput.inputPaths, options ) );

    const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (non-recursively)", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestPaths{
            {
                BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                BIT7Z_STRING( "folder/clouds.jpg" ),
                BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" )
            },
            {
                "Lorem Ipsum.pdf",
                "folder/clouds.jpg",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                "italy.svg",
                "dot.folder/hello.json"
            }
        },
        TestPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {}
        }
    );

    REQUIRE_NOTHROW( itemsVector.indexPaths( testInput.inputPaths, options ) );

    const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a directory as a file should fail", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "." ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "dot.folder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "empty" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder/subfolder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder/subfolder2" ) ) );
}

TEST_CASE( "BitItemsVector: Indexing a non-existing file should fail", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "non-existing.ext" ) ) );
}

struct TestFile {
    fs::path inputFile;
    fs::path expectedItem;
};

TEST_CASE( "BitItemsVector: Indexing a single file", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    // Catch2 doesn't support preprocessor ifdef inside GENERATE, at least on MSVC
#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", "italy.svg" },
            TestFile{ "noext", "noext" },
            TestFile{ BIT7Z_STRING( "σαράντα δύο.txt" ), BIT7Z_STRING( "σαράντα δύο.txt" ) },
            TestFile{ "folder/clouds.jpg", "folder/clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", "folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", "folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", "folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", "dot.folder/hello.json" }
        );
#else
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", "italy.svg" },
            TestFile{ "noext", "noext" },
            TestFile{ "folder/clouds.jpg", "folder/clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", "folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", "folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", "folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", "dot.folder/hello.json" }
        );
#endif

    DYNAMIC_SECTION ( "Indexing file " << testInput.inputFile.u8string() ) {
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.inputFile.string< bit7z::tchar >() ) );

        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == testInput.expectedItem );
        REQUIRE( itemsVector[ 0 ].path() == testInput.inputFile );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput.inputFile ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
#define UNICODE_TESTPATH(x) BIT7Z_STRING( x ),
#else
#define UNICODE_TESTPATH(x)
#endif

TEST_CASE( "BitItemsVector: Indexing a single file with a custom name", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
        "Lorem Ipsum.pdf",
        "italy.svg",
        "noext",
        UNICODE_TESTPATH( "σαράντα δύο.txt" )
        "folder/clouds.jpg",
        "folder/subfolder2/homework.doc",
        "folder/subfolder2/The quick brown fox.pdf",
        "folder/subfolder2/frequency.xlsx",
        "dot.folder/hello.json"
    );

    DYNAMIC_SECTION ( "Indexing file " << testInput.u8string() ) {
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.string< bit7z::tchar >(),
                                                 BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == testInput );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a single stream", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
        BIT7Z_STRING( "Lorem Ipsum.pdf" ),
        BIT7Z_STRING( "italy.svg" ),
        BIT7Z_STRING( "noext" ),
        UNICODE_TESTPATH( "σαράντα δύο.txt" )
        BIT7Z_STRING( "folder/clouds.jpg" ),
        BIT7Z_STRING( "folder/subfolder2/homework.doc" ),
        BIT7Z_STRING( "folder/subfolder2/The quick brown fox.pdf" ),
        BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" ),
        BIT7Z_STRING( "dot.folder/hello.json" )
    );

    DYNAMIC_SECTION ( "Indexing file " << testInput.u8string() << " as a stream" ) {
        REQUIRE_OPEN_IFSTREAM( input_stream, testInput );
        REQUIRE_NOTHROW( itemsVector.indexStream( input_stream, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}

TEST_CASE( "BitItemsVector: Indexing a single buffer", "[bititemsvector]" ) {
    const fs::path oldCurrentDir = current_dir();
    REQUIRE( set_current_dir( test_filesystem_dir ) );

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
        BIT7Z_STRING( "Lorem Ipsum.pdf" ),
        BIT7Z_STRING( "italy.svg" ),
        BIT7Z_STRING( "noext" ),
        UNICODE_TESTPATH( "σαράντα δύο.txt" )
        BIT7Z_STRING( "folder/clouds.jpg" ),
        BIT7Z_STRING( "folder/subfolder2/homework.doc" ),
        BIT7Z_STRING( "folder/subfolder2/The quick brown fox.pdf" ),
        BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" ),
        BIT7Z_STRING( "dot.folder/hello.json" )
    );

    DYNAMIC_SECTION ( "Indexing file " << testInput.u8string() << " as a buffer" ) {
        REQUIRE_LOAD_FILE( input_buffer, testInput );
        REQUIRE_NOTHROW( itemsVector.indexBuffer( input_buffer, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }

    REQUIRE( set_current_dir( oldCurrentDir ) );
}