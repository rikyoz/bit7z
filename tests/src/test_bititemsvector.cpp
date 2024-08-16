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

struct TestInputPath {
    fs::path path;
    vector< fs::path > expectedItems;
};

TEST_CASE( "BitItemsVector: Indexing a valid directory", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
        TestInputPath{ "empty", { "empty" } },
        TestInputPath{ "./empty", { "empty" } },
        TestInputPath{
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
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (only files)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.onlyFiles = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                "dot.folder/hello.json",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { "empty" } },
        TestInputPath{ "./empty", { "empty" } },
        TestInputPath{
            "folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                "folder",
                "folder/clouds.jpg",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (retaining folder structure)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    /* NOTE: BitItemsVector uses the retainFolderStructure option only to decide if it must include
     * the root folder in the in-archive paths; the actual folder structure is still kept here,
     * and will be discarded by the extract callback (e.g., see FileExtractCallback). */
    // TODO: Rationalize the handling of retainFolderStructure.
    IndexingOptions options{};
    options.retainFolderStructure = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                ".",
                "./italy.svg",
                "./Lorem Ipsum.pdf",
                "./noext",
                BIT7Z_NATIVE_STRING( "./σαράντα δύο.txt" ),
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
        TestInputPath{ "empty", { "empty" } },
        TestInputPath{ "./empty", { "./empty" } },
        TestInputPath{
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
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "./folder/subfolder2",
                "./folder/subfolder2/homework.doc",
                "./folder/subfolder2/The quick brown fox.pdf",
                "./folder/subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (filtered)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    SECTION( "Wildcard filter *" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
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
            TestInputPath{
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
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                    "dot.folder/hello.json",
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "dot.folder",
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.*" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder2/homework.doc",
                    "subfolder2/The quick brown fox.pdf",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
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
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "Lorem Ipsum.pdf", "folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder", { "subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder/subfolder2", { "The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.pdf" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "italy.svg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.svg" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "folder/clouds.jpg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/clouds.jpg" } },
            TestInputPath{ "./folder", { "clouds.jpg" } },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.jpg" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "folder/subfolder2/homework.doc" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder", { "subfolder2/homework.doc" } },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder/subfolder2", { "homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.doc" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder", { "subfolder2/frequency.xlsx" } },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder/subfolder2", { "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.xlsx" ) ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only PNG files (no matching file)" ) {
        const auto testInput = GENERATE( as < fs::path > {},
                                              ".",
                                              "empty",
                                              "./empty",
                                              "folder",
                                              "./folder",
                                              "folder/subfolder2",
                                              "./folder/subfolder2" );

        DYNAMIC_SECTION ( "Indexing directory " << testInput ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput, BIT7Z_STRING( "*.png" ) ) );
            REQUIRE( itemsVector.size() == 0 );
        }
    }

    SECTION( "Only non-PDF files" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "italy.svg",
                    "noext",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg",
                    "folder/subfolder",
                    "folder/subfolder2",
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg",
                    "subfolder",
                    "subfolder2",
                    "subfolder2/homework.doc",
                    "subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.pdf" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (excluding)" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Empty filter (excluding)" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", { "empty" } },
            TestInputPath{ "./empty", { "empty" } },
            TestInputPath{ "folder", { "folder" } },
            TestInputPath{ "./folder", { "folder" } },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2" } },
            TestInputPath{ "./folder/subfolder2", { "subfolder2" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "" ),
                                                          FilterPolicy::Exclude ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (non-recursively, filtered)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    SECTION( "Wildcard filter *" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
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
            TestInputPath{
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
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            },
            TestInputPath{
                "../test_filesystem/folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    "noext",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    "dot.folder",
                    "dot.folder/hello.json",
                    "italy.svg",
                    "Lorem Ipsum.pdf",
                    BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    "folder/clouds.jpg"
                }
            },
            TestInputPath{
                "./folder",
                {
                    "clouds.jpg"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            },
            TestInputPath{
                "../test_filesystem/folder/subfolder2",
                {
                    "homework.doc",
                    "The quick brown fox.pdf",
                    "frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{ ".", { "italy.svg", "Lorem Ipsum.pdf", BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ), } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/clouds.jpg" } },
            TestInputPath{ "./folder", { "clouds.jpg" } },
            TestInputPath{
                "folder/subfolder2",
                {
                    "folder/subfolder2/homework.doc",
                    "folder/subfolder2/The quick brown fox.pdf",
                    "folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "./folder/subfolder2", { "homework.doc", "The quick brown fox.pdf", "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.*" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
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
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "Lorem Ipsum.pdf" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder/subfolder2", { "The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.pdf" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { "italy.svg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.svg" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { "folder/clouds.jpg" } },
            TestInputPath{ "./folder", { "clouds.jpg" } },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.jpg" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder/subfolder2", { "homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.doc" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { "folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder/subfolder2", { "frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "*.xlsx" ),
                                                          FilterPolicy::Include, options ) );

            const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only PNG files (no matching file)" ) {
        const auto testInput = GENERATE( as < fs::path > {},
                                              ".",
                                              "empty",
                                              "./empty",
                                              "folder",
                                              "./folder",
                                              "folder/subfolder2",
                                              "./folder/subfolder2" );

        DYNAMIC_SECTION ( "Indexing directory " << testInput ) {
            REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput, BIT7Z_STRING( "*.png" ),
                                                          FilterPolicy::Include, options ) );
            REQUIRE( itemsVector.size() == 0 );
        }
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (non-recursively)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                "dot.folder",
                "dot.folder/hello.json",
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
        TestInputPath{ "empty", { "empty" } },
        TestInputPath{ "./empty", { "empty" } },
        TestInputPath{
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
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            "../test_filesystem",
            {
                "test_filesystem",
                "test_filesystem/italy.svg",
                "test_filesystem/Lorem Ipsum.pdf",
                "test_filesystem/noext",
                BIT7Z_NATIVE_STRING( "test_filesystem/σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/..",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/../dot.folder",
            {
                "dot.folder",
                "dot.folder/hello.json"
            }
        },
        TestInputPath{ "../test_filesystem/empty", { "empty" } },
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2/../",
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
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path, non-recursively)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            "../test_filesystem",
            {
                "test_filesystem",
                "test_filesystem/italy.svg",
                "test_filesystem/Lorem Ipsum.pdf",
                "test_filesystem/noext",
                BIT7Z_NATIVE_STRING( "test_filesystem/σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/..",
            {
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/../dot.folder",
            {
                "dot.folder",
                "dot.folder/hello.json"
            }
        },
        TestInputPath{ "../test_filesystem/empty", { "empty" } },
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2/../",
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
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        REQUIRE_NOTHROW( itemsVector.indexDirectory( testInput.path, BIT7Z_STRING( "" ),
                                                      FilterPolicy::Include, options ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (custom path mapping)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                "custom_folder",
                "custom_folder/dot.folder",
                "custom_folder/dot.folder/hello.json",
                "custom_folder/italy.svg",
                "custom_folder/Lorem Ipsum.pdf",
                "custom_folder/noext",
                BIT7Z_NATIVE_STRING( "custom_folder/σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/..",
            {
                "custom_folder",
                "custom_folder/dot.folder",
                "custom_folder/dot.folder/hello.json",
                "custom_folder/italy.svg",
                "custom_folder/Lorem Ipsum.pdf",
                "custom_folder/noext",
                BIT7Z_NATIVE_STRING( "custom_folder/σαράντα δύο.txt" ),
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
        TestInputPath{
            "folder/../",
            {
                "custom_folder",
                "custom_folder/dot.folder",
                "custom_folder/dot.folder/hello.json",
                "custom_folder/italy.svg",
                "custom_folder/Lorem Ipsum.pdf",
                "custom_folder/noext",
                BIT7Z_NATIVE_STRING( "custom_folder/σαράντα δύο.txt" ),
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
        TestInputPath{ "empty", { "custom_folder" } },
        TestInputPath{ "./empty", { "custom_folder" } },
        TestInputPath{
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
        TestInputPath{
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
        TestInputPath{
            "./folder/subfolder2/..",
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
        TestInputPath{
            "./folder/subfolder2/../",
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
        TestInputPath{
            "folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                "custom_folder",
                "custom_folder/homework.doc",
                "custom_folder/The quick brown fox.pdf",
                "custom_folder/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testInput.path.string< bit7z::tchar >(), BIT7Z_STRING( "custom_folder" ) }
        };
        REQUIRE_NOTHROW( itemsVector.indexPathsMap( pathMap ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (empty custom path mapping)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                "dot.folder",
                "dot.folder/hello.json",
                "italy.svg",
                "Lorem Ipsum.pdf",
                "noext",
                BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
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
        TestInputPath{ "empty", { "empty" } },
        TestInputPath{ "./empty", { "empty" } },
        TestInputPath{
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
        TestInputPath{
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
        TestInputPath{
            "folder/subfolder2",
            {
                "folder/subfolder2",
                "folder/subfolder2/homework.doc",
                "folder/subfolder2/The quick brown fox.pdf",
                "folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                "subfolder2",
                "subfolder2/homework.doc",
                "subfolder2/The quick brown fox.pdf",
                "subfolder2/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testInput.path.string< bit7z::tchar >(), BIT7Z_STRING( "" ) }
        };
        REQUIRE_NOTHROW( itemsVector.indexPathsMap( pathMap ) );

        const vector< fs::path > indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::UnorderedEquals( testInput.expectedItems ) );
    }
}

struct TestInputPaths {
    vector< bit7z::tstring > inputPaths;
    vector< fs::path > expectedItems;
};

TEST_CASE( "BitItemsVector: Indexing a vector of paths", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPaths{
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
        TestInputPaths{
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
        TestInputPaths{
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
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (retaining folder structure)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.retainFolderStructure = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPaths{
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
        TestInputPaths{
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
        TestInputPaths{
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
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (only files)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.onlyFiles = true;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPaths{
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
        TestInputPaths{
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
        TestInputPaths{
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
}

TEST_CASE( "BitItemsVector: Indexing a vector of paths (non-recursively)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    BitItemsVector itemsVector;

    const auto testInput = GENERATE(
        TestInputPaths{
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
        TestInputPaths{
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
        TestInputPaths{
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
}

TEST_CASE( "BitItemsVector: Indexing a directory as a file should fail", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "." ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "dot.folder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "dot.folder/../" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "empty" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder/subfolder" ) ) );
    REQUIRE_THROWS( itemsVector.indexFile( BIT7Z_STRING( "folder/subfolder/../.." ) ) );
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
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    // Catch2 doesn't support preprocessor ifdef inside GENERATE, at least on MSVC
#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "empty/../Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "folder/subfolder2/../../Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", "italy.svg" },
            TestFile{ "noext", "noext" },
            TestFile{ BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ), BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ) },
            TestFile{ "folder/clouds.jpg", "folder/clouds.jpg" },
            TestFile{ "dot.folder/../folder/clouds.jpg", "clouds.jpg" },
            TestFile{ "folder/subfolder2/../clouds.jpg", "clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", "folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", "folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", "folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", "dot.folder/hello.json" }
        );
#else
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "empty/../Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "folder/subfolder2/../../Lorem Ipsum.pdf", "Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", "italy.svg" },
            TestFile{ "noext", "noext" },
            TestFile{ "folder/clouds.jpg", "folder/clouds.jpg" },
            TestFile{ "dot.folder/../folder/clouds.jpg", "clouds.jpg" },
            TestFile{ "folder/subfolder2/../clouds.jpg", "clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", "folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", "folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", "folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", "dot.folder/hello.json" }
        );
#endif

    DYNAMIC_SECTION( "Indexing file " << testInput.inputFile.u8string() ) {
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.inputFile.string< bit7z::tchar >() ) );
#else
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.inputFile.u8string() ) );
#endif

        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == testInput.expectedItem );
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE( itemsVector[ 0 ].path() == testInput.inputFile );
#else
        REQUIRE( itemsVector[ 0 ].path() == testInput.inputFile.u8string() );
#endif
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput.inputFile ) );
    }
}

#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
#define UNICODE_TEST_PATH(x) BIT7Z_NATIVE_STRING( x ),
#else
#define UNICODE_TEST_PATH(x)
#endif

TEST_CASE( "BitItemsVector: Indexing a single file with a custom name", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
                                         "Lorem Ipsum.pdf",
                                         "italy.svg",
                                         "noext",
                                         UNICODE_TEST_PATH( "σαράντα δύο.txt" )
                                         "folder/clouds.jpg",
                                         "folder/subfolder2/homework.doc",
                                         "folder/subfolder2/The quick brown fox.pdf",
                                         "folder/subfolder2/frequency.xlsx",
                                         "dot.folder/hello.json" );

    DYNAMIC_SECTION( "Indexing file " << testInput.u8string() ) {
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.string< bit7z::tchar >(),
                                                BIT7Z_STRING( "custom_name.ext" ) ) );
#else
        REQUIRE_NOTHROW( itemsVector.indexFile( testInput.u8string(), BIT7Z_STRING( "custom_name.ext" ) ) );
#endif

        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE( itemsVector[ 0 ].path() == testInput );
#else
        REQUIRE( itemsVector[ 0 ].path() == testInput.u8string() );
#endif
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a single stream", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
                                         BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                                         BIT7Z_STRING( "italy.svg" ),
                                         BIT7Z_STRING( "noext" ),
                                         BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                                         BIT7Z_STRING( "folder/clouds.jpg" ),
                                         BIT7Z_STRING( "folder/subfolder2/homework.doc" ),
                                         BIT7Z_STRING( "folder/subfolder2/The quick brown fox.pdf" ),
                                         BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" ),
                                         BIT7Z_STRING( "dot.folder/hello.json" ) );

    DYNAMIC_SECTION( "Indexing file " << testInput.u8string() << " as a stream" ) {
        REQUIRE_OPEN_IFSTREAM( input_stream, testInput );
        REQUIRE_NOTHROW( itemsVector.indexStream( input_stream, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a single buffer", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    BitItemsVector itemsVector;

    const fs::path testInput = GENERATE( as< fs::path >(),
                                         BIT7Z_STRING( "Lorem Ipsum.pdf" ),
                                         BIT7Z_STRING( "italy.svg" ),
                                         BIT7Z_STRING( "noext" ),
                                         BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ),
                                         BIT7Z_STRING( "folder/clouds.jpg" ),
                                         BIT7Z_STRING( "folder/subfolder2/homework.doc" ),
                                         BIT7Z_STRING( "folder/subfolder2/The quick brown fox.pdf" ),
                                         BIT7Z_STRING( "folder/subfolder2/frequency.xlsx" ),
                                         BIT7Z_STRING( "dot.folder/hello.json" ) );

    DYNAMIC_SECTION( "Indexing file " << testInput.u8string() << " as a buffer" ) {
        REQUIRE_LOAD_FILE( input_buffer, testInput );
        REQUIRE_NOTHROW( itemsVector.indexBuffer( input_buffer, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == "custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}