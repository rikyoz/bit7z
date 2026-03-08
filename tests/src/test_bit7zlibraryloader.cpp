// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2026 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/shared_lib.hpp"

#include <bit7z/bit7zlibraryloader.hpp>
#include <bit7z/bitexception.hpp>
#include <bit7z/bittypes.hpp>

namespace bit7z {
namespace test {

const auto implicitConversion = []( const Bit7zLibrary& loader ) -> void {
    loader.useLargePages();
};

TEST_CASE( "Bit7zLibraryLoader: Default constructor", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{};

    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );

    REQUIRE_NOTHROW( loader.unload() );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );
}

TEST_CASE( "Bit7zLibraryLoader: Deferred loading", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{};

    REQUIRE_NOTHROW( loader.load( sevenzip_lib_path() ) );
    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    REQUIRE_NOTHROW( loader.unload() );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );
}

TEST_CASE( "Bit7zLibraryLoader: Immediate loading", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{ sevenzip_lib_path() };

    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    REQUIRE_NOTHROW( loader.unload() );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );
}

const auto isNonExistingLibraryErrorCode = []( const std::error_code code ) -> bool {
#ifdef _WIN32
    return code.value() == ERROR_MOD_NOT_FOUND;
#else
    return code.value() == EBADF;
#endif
};

const auto isNonExistingLibraryException = []( const BitException& ex ) -> bool {
    return isNonExistingLibraryErrorCode( ex.code() );
};

TEST_CASE( "Bit7zLibraryLoader: Constructing from a non-existing shared library", "[bit7zlibraryloader]" ) {
    REQUIRE_THROWS_WITH(
        Bit7zLibraryLoader{ BIT7Z_STRING( "NonExisting7z.dll" ) },
        Catch::Matchers::StartsWith( "Failed to load the library" )
    );

    REQUIRE_THROWS_MATCHES(
        Bit7zLibraryLoader{ BIT7Z_STRING( "NonExisting7z.dll" ) },
        BitException,
        Catch::Matchers::Predicate< BitException >(isNonExistingLibraryException, "Unexpected error code" )
    );
}

TEST_CASE( "Bit7zLibraryLoader: Deferred loading a non-existing shared library", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{};

    REQUIRE_THROWS_MATCHES(
        loader.load( BIT7Z_STRING( "NonExisting7z.dll" ) ),
        BitException,
        Catch::Matchers::Predicate< BitException >(isNonExistingLibraryException, "Unexpected error code" )
    );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );

    REQUIRE_NOTHROW( loader.load( sevenzip_lib_path() ) );
    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    REQUIRE_THROWS_MATCHES(
        loader.load( BIT7Z_STRING( "NonExisting7z.dll" ) ),
        BitException,
        Catch::Matchers::Predicate< BitException >(isNonExistingLibraryException, "Unexpected error code" )
    );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );
}

TEST_CASE( "Bit7zLibraryLoader: Deferred non-throwing loading a non-existing shared library", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{};

    std::error_code error;
    REQUIRE_NOTHROW( loader.load( BIT7Z_STRING( "NonExisting7z.dll" ), error ) );
    REQUIRE( isNonExistingLibraryErrorCode( error ) );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );

    REQUIRE_NOTHROW( loader.load( sevenzip_lib_path(), error ) );
    REQUIRE_FALSE( error );
    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    REQUIRE_NOTHROW( loader.load( BIT7Z_STRING( "NonExisting7z.dll" ), error ) );
    REQUIRE( isNonExistingLibraryErrorCode( error ) );
    REQUIRE_FALSE( loader.isLoaded() );
    REQUIRE_THROWS_WITH( implicitConversion( loader ), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader.library(), Catch::Matchers::StartsWith( "Library not loaded" ) );
    REQUIRE_THROWS_WITH( loader->useLargePages(), Catch::Matchers::StartsWith( "Library not loaded" ) );
}

TEST_CASE( "Bit7zLibraryLoader: Reloading library more than once", "[bit7zlibraryloader]" ) {
    Bit7zLibraryLoader loader{ sevenzip_lib_path() };

    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    REQUIRE_NOTHROW( loader.load( sevenzip_lib_path() ) );
    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );

    std::error_code error;
    REQUIRE_NOTHROW( loader.load( sevenzip_lib_path(), error ) );
    REQUIRE_FALSE( error );
    REQUIRE( loader.isLoaded() );
    REQUIRE_NOTHROW( implicitConversion( loader ) );
    REQUIRE_NOTHROW( loader.library() );
    REQUIRE_NOTHROW( loader->useLargePages() );
}

} // namespace test
} // namespace bit7z
