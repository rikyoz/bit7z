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

#include <bit7z/bitabstractarchivehandler.hpp>
#include <bit7z/bititemsvector.hpp>
#include <bit7z/bittypes.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

using std::vector;
using std::map;

using bit7z::tstring;
using bit7z::BitItemsVector;
using bit7z::IndexingOptions;
using namespace bit7z;
using namespace bit7z::test::filesystem;

TEST_CASE( "BitItemsVector: Default properties", "[bititemsvector]" ) {
    const BitItemsVector itemsVector{};

    REQUIRE( itemsVector.empty() );
    REQUIRE( itemsVector.cbegin() == itemsVector.cend() );
    REQUIRE( itemsVector.begin() == itemsVector.end() );
}

TEST_CASE( "BitItemsVector: Indexing an invalid directory (non-existing)", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( indexDirectory( itemsVector, "not_existing_path" ) );
}

TEST_CASE( "BitItemsVector: Indexing an invalid directory (empty string)", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( indexDirectory( itemsVector, "" ) );
}

auto in_archive_paths( const BitItemsVector& vector ) -> std::vector< sevenzip_string > {
    std::vector< sevenzip_string > paths;
    paths.reserve( vector.size() );
    std::transform( vector.cbegin(), vector.cend(), std::back_inserter( paths ),
                    []( const auto& item ) {
                        return item.inArchivePath();
                    } );
    return paths;
}

struct TestInputPath {
    fs::path path;
    vector< sevenzip_string > expectedItems;
};

TEST_CASE( "BitItemsVector: Indexing a valid directory", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"empty" } },
        TestInputPath{ "./empty", { L"empty" } },
        TestInputPath{
            "folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (only files)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.onlyFiles = true;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"dot.folder/hello.json",
                L"folder/clouds.jpg",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"empty" } },
        TestInputPath{ "./empty", { L"empty" } },
        TestInputPath{
            "folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "" ), options ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L".",
                L"./italy.svg",
                L"./Lorem Ipsum.pdf",
                L"./noext",
                L"./σαράντα δύο.txt",
                L"./dot.folder",
                L"./dot.folder/hello.json",
                L"./empty",
                L"./folder",
                L"./folder/clouds.jpg",
                L"./folder/subfolder",
                L"./folder/subfolder2",
                L"./folder/subfolder2/homework.doc",
                L"./folder/subfolder2/The quick brown fox.pdf",
                L"./folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"empty" } },
        TestInputPath{ "./empty", { L"./empty" } },
        TestInputPath{
            "folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"./folder",
                L"./folder/clouds.jpg",
                L"./folder/subfolder",
                L"./folder/subfolder2",
                L"./folder/subfolder2/homework.doc",
                L"./folder/subfolder2/The quick brown fox.pdf",
                L"./folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"./folder/subfolder2",
                L"./folder/subfolder2/homework.doc",
                L"./folder/subfolder2/The quick brown fox.pdf",
                L"./folder/subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "" ), options ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (filtered)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    SECTION( "Wildcard filter *" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"noext",
                    L"σαράντα δύο.txt",
                    L"dot.folder",
                    L"dot.folder/hello.json",
                    L"empty",
                    L"folder",
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder",
                    L"subfolder2",
                    L"subfolder2/homework.doc",
                    L"subfolder2/The quick brown fox.pdf",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"noext",
                    L"σαράντα δύο.txt",
                    L"dot.folder/hello.json",
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder2/homework.doc",
                    L"subfolder2/The quick brown fox.pdf",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"dot.folder",
                    L"dot.folder/hello.json",
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"σαράντα δύο.txt",
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder2/homework.doc",
                    L"subfolder2/The quick brown fox.pdf",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.*" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        IndexingOptions options{};
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"dot.folder/hello.json",
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"σαράντα δύο.txt",
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder2/homework.doc",
                    L"subfolder2/The quick brown fox.pdf",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.folder" ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, ".", BIT7Z_STRING( "*.folder" ) ) );

        const std::vector< sevenzip_string > expectedItems{ L"dot.folder" };

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( expectedItems ) );
    }

    SECTION( "Wildcard filter *.folder (only files)" ) {
        BitItemsVector itemsVector;
        // Even if we are indexing non-recursively, the "dot.folder" matches the filter; hence, it must be indexed.
        IndexingOptions options{};
        options.onlyFiles = true;

        REQUIRE_NOTHROW( indexDirectory( itemsVector, ".", BIT7Z_STRING( "*.folder" ), options ) );
        REQUIRE( itemsVector.empty() );
    }

    SECTION( "Only PDF files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"Lorem Ipsum.pdf", L"folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder", { L"subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder/subfolder2", { L"The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.pdf" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"italy.svg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.svg" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"folder/clouds.jpg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/clouds.jpg" } },
            TestInputPath{ "./folder", { L"clouds.jpg" } },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.jpg" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"folder/subfolder2/homework.doc" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder", { L"subfolder2/homework.doc" } },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder/subfolder2", { L"homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.doc" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder", { L"subfolder2/frequency.xlsx" } },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder/subfolder2", { L"frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.xlsx" ) ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput, BIT7Z_STRING( "*.png" ) ) );
            REQUIRE( itemsVector.empty() );
        }
    }

    SECTION( "Only non-PDF files" ) {
        IndexingOptions options{};
        options.filterPolicy = FilterPolicy::Exclude;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"italy.svg",
                    L"noext",
                    L"σαράντα δύο.txt",
                    L"dot.folder",
                    L"dot.folder/hello.json",
                    L"empty",
                    L"folder",
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder",
                    L"subfolder2",
                    L"subfolder2/homework.doc",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.pdf" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (excluding)" ) {
        IndexingOptions options{};
        options.filterPolicy = FilterPolicy::Exclude;

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
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Empty filter (excluding)" ) {
        IndexingOptions options{};
        options.filterPolicy = FilterPolicy::Exclude;

        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", { L"empty" } },
            TestInputPath{ "./empty", { L"empty" } },
            TestInputPath{ "folder", { L"folder" } },
            TestInputPath{ "./folder", { L"folder" } },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2" } },
            TestInputPath{ "./folder/subfolder2", { L"subfolder2" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            BitItemsVector itemsVector;
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"noext",
                    L"σαράντα δύο.txt",
                    L"dot.folder",
                    L"dot.folder/hello.json",
                    L"empty",
                    L"folder",
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg",
                    L"folder/subfolder",
                    L"folder/subfolder2",
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg",
                    L"subfolder",
                    L"subfolder2",
                    L"subfolder2/homework.doc",
                    L"subfolder2/The quick brown fox.pdf",
                    L"subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            },
            TestInputPath{
                "../test_filesystem/folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter * (only files)" ) {
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"noext",
                    L"σαράντα δύο.txt",
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.*" ) {
        const auto testInput = GENERATE(
            TestInputPath{
                ".",
                {
                    L"dot.folder",
                    L"dot.folder/hello.json",
                    L"italy.svg",
                    L"Lorem Ipsum.pdf",
                    L"σαράντα δύο.txt",
                }
            },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{
                "folder",
                {
                    L"folder/clouds.jpg"
                }
            },
            TestInputPath{
                "./folder",
                {
                    L"clouds.jpg"
                }
            },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{
                "./folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            },
            TestInputPath{
                "../test_filesystem/folder/subfolder2",
                {
                    L"homework.doc",
                    L"The quick brown fox.pdf",
                    L"frequency.xlsx"
                }
            }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.* (only files)" ) {
        options.onlyFiles = true;

        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"italy.svg", L"Lorem Ipsum.pdf", L"σαράντα δύο.txt", } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/clouds.jpg" } },
            TestInputPath{ "./folder", { L"clouds.jpg" } },
            TestInputPath{
                "folder/subfolder2",
                {
                    L"folder/subfolder2/homework.doc",
                    L"folder/subfolder2/The quick brown fox.pdf",
                    L"folder/subfolder2/frequency.xlsx"
                }
            },
            TestInputPath{ "./folder/subfolder2", { L"homework.doc", L"The quick brown fox.pdf", L"frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.*" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Wildcard filter *.folder" ) {
        // Even if we are indexing non-recursively, the "dot.folder" matches the filter; hence, it must be indexed.
        REQUIRE_NOTHROW( indexDirectory( itemsVector, ".", BIT7Z_STRING( "*.folder" ), options ) );

        const std::vector< sevenzip_string > expectedItems{ L"dot.folder" };

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( expectedItems ) );
    }

    SECTION( "Wildcard filter *.folder (only files)" ) {
        options.onlyFiles = true;

        REQUIRE_NOTHROW( indexDirectory( itemsVector, ".", BIT7Z_STRING( "*.folder" ), options ) );
        REQUIRE( itemsVector.empty() );
    }

    SECTION( "Only PDF files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"Lorem Ipsum.pdf" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/The quick brown fox.pdf" } },
            TestInputPath{ "./folder/subfolder2", { L"The quick brown fox.pdf" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.pdf" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only SVG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", { L"italy.svg" } },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.svg" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only JPG files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", { L"folder/clouds.jpg" } },
            TestInputPath{ "./folder", { L"clouds.jpg" } },
            TestInputPath{ "folder/subfolder2", {} },
            TestInputPath{ "./folder/subfolder2", {} }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.jpg" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only DOC files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/homework.doc" } },
            TestInputPath{ "./folder/subfolder2", { L"homework.doc" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.doc" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
        }
    }

    SECTION( "Only XLSX files" ) {
        const auto testInput = GENERATE(
            TestInputPath{ ".", {} },
            TestInputPath{ "empty", {} },
            TestInputPath{ "./empty", {} },
            TestInputPath{ "folder", {} },
            TestInputPath{ "./folder", {} },
            TestInputPath{ "folder/subfolder2", { L"folder/subfolder2/frequency.xlsx" } },
            TestInputPath{ "./folder/subfolder2", { L"frequency.xlsx" } }
        );

        DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "*.xlsx" ), options ) );

            const auto indexedPaths = in_archive_paths( itemsVector );
            REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
            REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput, BIT7Z_STRING( "*.png" ), options ) );
            REQUIRE( itemsVector.empty() );
        }
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (non-recursively)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L"dot.folder",
                L"dot.folder/hello.json",
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"empty" } },
        TestInputPath{ "./empty", { L"empty" } },
        TestInputPath{
            "folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "" ), options ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testInput = GENERATE(
        TestInputPath{
            "../test_filesystem",
            {
                L"test_filesystem",
                L"test_filesystem/italy.svg",
                L"test_filesystem/Lorem Ipsum.pdf",
                L"test_filesystem/noext",
                L"test_filesystem/σαράντα δύο.txt",
                L"test_filesystem/dot.folder",
                L"test_filesystem/dot.folder/hello.json",
                L"test_filesystem/empty",
                L"test_filesystem/folder",
                L"test_filesystem/folder/clouds.jpg",
                L"test_filesystem/folder/subfolder",
                L"test_filesystem/folder/subfolder2",
                L"test_filesystem/folder/subfolder2/homework.doc",
                L"test_filesystem/folder/subfolder2/The quick brown fox.pdf",
                L"test_filesystem/folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/..",
            {
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/../dot.folder",
            {
                L"dot.folder",
                L"dot.folder/hello.json"
            }
        },
        TestInputPath{ "../test_filesystem/empty", { L"empty" } },
        TestInputPath{
            "../test_filesystem/folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2/../",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (relative path, non-recursively)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    IndexingOptions options{};
    options.recursive = false;

    const auto testInput = GENERATE(
        TestInputPath{
            "../test_filesystem",
            {
                L"test_filesystem",
                L"test_filesystem/italy.svg",
                L"test_filesystem/Lorem Ipsum.pdf",
                L"test_filesystem/noext",
                L"test_filesystem/σαράντα δύο.txt",
                L"test_filesystem/dot.folder",
                L"test_filesystem/dot.folder/hello.json",
                L"test_filesystem/empty",
                L"test_filesystem/folder",
                L"test_filesystem/folder/clouds.jpg",
                L"test_filesystem/folder/subfolder",
                L"test_filesystem/folder/subfolder2",
                L"test_filesystem/folder/subfolder2/homework.doc",
                L"test_filesystem/folder/subfolder2/The quick brown fox.pdf",
                L"test_filesystem/folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/..",
            {
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/../dot.folder",
            {
                L"dot.folder",
                L"dot.folder/hello.json"
            }
        },
        TestInputPath{ "../test_filesystem/empty", { L"empty" } },
        TestInputPath{
            "../test_filesystem/folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2/../",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        }
    );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexDirectory( itemsVector, testInput.path, BIT7Z_STRING( "" ), options ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (custom path mapping)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L"custom_folder",
                L"custom_folder/dot.folder",
                L"custom_folder/dot.folder/hello.json",
                L"custom_folder/italy.svg",
                L"custom_folder/Lorem Ipsum.pdf",
                L"custom_folder/noext",
                L"custom_folder/σαράντα δύο.txt",
                L"custom_folder/empty",
                L"custom_folder/folder",
                L"custom_folder/folder/clouds.jpg",
                L"custom_folder/folder/subfolder",
                L"custom_folder/folder/subfolder2",
                L"custom_folder/folder/subfolder2/homework.doc",
                L"custom_folder/folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/..",
            {
                L"custom_folder",
                L"custom_folder/dot.folder",
                L"custom_folder/dot.folder/hello.json",
                L"custom_folder/italy.svg",
                L"custom_folder/Lorem Ipsum.pdf",
                L"custom_folder/noext",
                L"custom_folder/σαράντα δύο.txt",
                L"custom_folder/empty",
                L"custom_folder/folder",
                L"custom_folder/folder/clouds.jpg",
                L"custom_folder/folder/subfolder",
                L"custom_folder/folder/subfolder2",
                L"custom_folder/folder/subfolder2/homework.doc",
                L"custom_folder/folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/../",
            {
                L"custom_folder",
                L"custom_folder/dot.folder",
                L"custom_folder/dot.folder/hello.json",
                L"custom_folder/italy.svg",
                L"custom_folder/Lorem Ipsum.pdf",
                L"custom_folder/noext",
                L"custom_folder/σαράντα δύο.txt",
                L"custom_folder/empty",
                L"custom_folder/folder",
                L"custom_folder/folder/clouds.jpg",
                L"custom_folder/folder/subfolder",
                L"custom_folder/folder/subfolder2",
                L"custom_folder/folder/subfolder2/homework.doc",
                L"custom_folder/folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"custom_folder" } },
        TestInputPath{ "./empty", { L"custom_folder" } },
        TestInputPath{
            "folder",
            {
                L"custom_folder",
                L"custom_folder/clouds.jpg",
                L"custom_folder/subfolder",
                L"custom_folder/subfolder2",
                L"custom_folder/subfolder2/homework.doc",
                L"custom_folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"custom_folder",
                L"custom_folder/clouds.jpg",
                L"custom_folder/subfolder",
                L"custom_folder/subfolder2",
                L"custom_folder/subfolder2/homework.doc",
                L"custom_folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2/..",
            {
                L"custom_folder",
                L"custom_folder/clouds.jpg",
                L"custom_folder/subfolder",
                L"custom_folder/subfolder2",
                L"custom_folder/subfolder2/homework.doc",
                L"custom_folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2/../",
            {
                L"custom_folder",
                L"custom_folder/clouds.jpg",
                L"custom_folder/subfolder",
                L"custom_folder/subfolder2",
                L"custom_folder/subfolder2/homework.doc",
                L"custom_folder/subfolder2/The quick brown fox.pdf",
                L"custom_folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"custom_folder",
                L"custom_folder/homework.doc",
                L"custom_folder/The quick brown fox.pdf",
                L"custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"custom_folder",
                L"custom_folder/homework.doc",
                L"custom_folder/The quick brown fox.pdf",
                L"custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                L"custom_folder",
                L"custom_folder/homework.doc",
                L"custom_folder/The quick brown fox.pdf",
                L"custom_folder/frequency.xlsx"
            }
        },
        TestInputPath{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                L"custom_folder",
                L"custom_folder/homework.doc",
                L"custom_folder/The quick brown fox.pdf",
                L"custom_folder/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testInput.path.string< bit7z::tchar >(), BIT7Z_STRING( "custom_folder" ) }
        };
        REQUIRE_NOTHROW( indexPathsMap( itemsVector, pathMap ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a valid directory (empty custom path mapping)", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    const auto testInput = GENERATE(
        TestInputPath{
            ".",
            {
                L"dot.folder",
                L"dot.folder/hello.json",
                L"italy.svg",
                L"Lorem Ipsum.pdf",
                L"noext",
                L"σαράντα δύο.txt",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{ "empty", { L"empty" } },
        TestInputPath{ "./empty", { L"empty" } },
        TestInputPath{
            "folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder",
            {
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "folder/subfolder2",
            {
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "./folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            "../test_filesystem/folder/subfolder2",
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        },
        TestInputPath{
            fs::absolute( "../test_filesystem/folder/subfolder2" ),
            {
                L"subfolder2",
                L"subfolder2/homework.doc",
                L"subfolder2/The quick brown fox.pdf",
                L"subfolder2/frequency.xlsx"
            }
        } );

    DYNAMIC_SECTION ( "Indexing directory " << testInput.path ) {
        BitItemsVector itemsVector;
        const std::map< bit7z::tstring, bit7z::tstring > pathMap{
            { testInput.path.string< bit7z::tchar >(), BIT7Z_STRING( "" ) }
        };
        REQUIRE_NOTHROW( indexPathsMap( itemsVector, pathMap ) );

        const auto indexedPaths = in_archive_paths( itemsVector );
        REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
    }
}

struct TestInputPaths {
    vector< bit7z::tstring > inputPaths;
    vector< sevenzip_string > expectedItems;
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
                L"Lorem Ipsum.pdf",
                L"folder/clouds.jpg",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                L"italy.svg",
                L"dot.folder/hello.json",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( indexPaths( itemsVector, testInput.inputPaths ) );

    const auto indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
                L"Lorem Ipsum.pdf",
                L"folder/clouds.jpg",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                L"italy.svg",
                L"dot.folder/hello.json",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( indexPaths( itemsVector, testInput.inputPaths, options ) );

    const auto indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
                L"Lorem Ipsum.pdf",
                L"folder/clouds.jpg",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                L"italy.svg",
                L"dot.folder/hello.json",
                L"folder/subfolder2",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "dot.folder" ),
                BIT7Z_STRING( "empty" ),
                BIT7Z_STRING( "folder" )
            },
            {
                L"dot.folder",
                L"dot.folder/hello.json",
                L"empty",
                L"folder",
                L"folder/clouds.jpg",
                L"folder/subfolder2/homework.doc",
                L"folder/subfolder2/The quick brown fox.pdf",
                L"folder/subfolder2/frequency.xlsx"
            }
        }
    );

    REQUIRE_NOTHROW( indexPaths( itemsVector, testInput.inputPaths, options ) );

    const auto indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
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
                L"Lorem Ipsum.pdf",
                L"folder/clouds.jpg",
                L"folder/subfolder2/frequency.xlsx"
            }
        },
        TestInputPaths{
            {
                BIT7Z_STRING( "italy.svg" ),
                BIT7Z_STRING( "dot.folder/hello.json" ),
                BIT7Z_STRING( "folder/subfolder2" )
            },
            {
                L"italy.svg",
                L"dot.folder/hello.json"
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

    REQUIRE_NOTHROW( indexPaths( itemsVector, testInput.inputPaths, options ) );

    const auto indexedPaths = in_archive_paths( itemsVector );
    REQUIRE_THAT( indexedPaths, Catch::Matchers::UnorderedEquals( testInput.expectedItems ) );
}

TEST_CASE( "BitItemsVector: Indexing a directory as a file should fail", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "." ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "dot.folder" ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "dot.folder/../" ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "empty" ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "folder" ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "folder/subfolder" ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "folder/subfolder/../.." ) ) );
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "folder/subfolder2" ) ) );
}

TEST_CASE( "BitItemsVector: Indexing a non-existing file should fail", "[bititemsvector]" ) {
    BitItemsVector itemsVector;
    REQUIRE_THROWS( indexFile( itemsVector, BIT7Z_STRING( "non-existing.ext" ) ) );
}

struct TestFile {
    fs::path inputFile;
    sevenzip_string expectedItem;
};

TEST_CASE( "BitItemsVector: Indexing a single file", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

    // Catch2 doesn't support preprocessor ifdef inside GENERATE, at least on MSVC
#ifndef BIT7Z_USE_SYSTEM_CODEPAGE
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "empty/../Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "folder/subfolder2/../../Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", L"italy.svg" },
            TestFile{ "noext", L"noext" },
            TestFile{ BIT7Z_NATIVE_STRING( "σαράντα δύο.txt" ), L"σαράντα δύο.txt" },
            TestFile{ "folder/clouds.jpg", L"folder/clouds.jpg" },
            TestFile{ "dot.folder/../folder/clouds.jpg", L"clouds.jpg" },
            TestFile{ "folder/subfolder2/../clouds.jpg", L"clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", L"folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", L"folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", L"folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", L"dot.folder/hello.json" }
        );
#else
    const auto testInput =
        GENERATE(
            TestFile{ "Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "empty/../Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "folder/subfolder2/../../Lorem Ipsum.pdf", L"Lorem Ipsum.pdf" },
            TestFile{ "italy.svg", L"italy.svg" },
            TestFile{ "noext", L"noext" },
            TestFile{ "folder/clouds.jpg", L"folder/clouds.jpg" },
            TestFile{ "dot.folder/../folder/clouds.jpg", L"clouds.jpg" },
            TestFile{ "folder/subfolder2/../clouds.jpg", L"clouds.jpg" },
            TestFile{ "folder/subfolder2/homework.doc", L"folder/subfolder2/homework.doc" },
            TestFile{ "folder/subfolder2/The quick brown fox.pdf", L"folder/subfolder2/The quick brown fox.pdf" },
            TestFile{ "folder/subfolder2/frequency.xlsx", L"folder/subfolder2/frequency.xlsx" },
            TestFile{ "dot.folder/hello.json", L"dot.folder/hello.json" }
        );
#endif

    DYNAMIC_SECTION( "Indexing file " << to_utf8string( testInput.inputFile ) ) {
        BitItemsVector itemsVector;
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE_NOTHROW( indexFile( itemsVector, testInput.inputFile.string< bit7z::tchar >() ) );
#else
        REQUIRE_NOTHROW( indexFile( itemsVector, to_utf8string( testInput.inputFile ) ) );
#endif

        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == testInput.expectedItem );
        REQUIRE( itemsVector[ 0 ].path() == testInput.inputFile.native() );
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

    DYNAMIC_SECTION( "Indexing file " << to_utf8string( testInput ) ) {
        BitItemsVector itemsVector;
#if defined( BIT7Z_USE_NATIVE_STRING ) || defined( BIT7Z_USE_SYSTEM_CODEPAGE )
        REQUIRE_NOTHROW( indexFile( itemsVector, testInput.string< bit7z::tchar >(),
                                                BIT7Z_STRING( "custom_name.ext" ) ) );
#else
        REQUIRE_NOTHROW( indexFile( itemsVector, to_utf8string( testInput ), BIT7Z_STRING( "custom_name.ext" ) ) );
#endif

        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == L"custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == testInput.native() );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a single stream", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

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

    DYNAMIC_SECTION( "Indexing file " << to_utf8string( testInput ) << " as a stream" ) {
        REQUIRE_OPEN_IFSTREAM( input_stream, testInput );

        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexStream( itemsVector, input_stream, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == L"custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_NATIVE_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}

TEST_CASE( "BitItemsVector: Indexing a single buffer", "[bititemsvector]" ) {
    static const TestDirectory testDir{ test_filesystem_dir };

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

    DYNAMIC_SECTION( "Indexing file " << to_utf8string( testInput ) << " as a buffer" ) {
        REQUIRE_LOAD_FILE( input_buffer, testInput );

        BitItemsVector itemsVector;
        REQUIRE_NOTHROW( indexBuffer( itemsVector, input_buffer, BIT7Z_STRING( "custom_name.ext" ) ) );
        REQUIRE( itemsVector.size() == 1 );
        REQUIRE( itemsVector[ 0 ].inArchivePath() == L"custom_name.ext" );
        REQUIRE( itemsVector[ 0 ].path() == BIT7Z_NATIVE_STRING( "custom_name.ext" ) );
        REQUIRE( itemsVector[ 0 ].size() == fs::file_size( testInput ) );
    }
}