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

#include <bit7z/bit7zlibrary.hpp>

#if !defined(__GNUC__) || __GNUC__ >= 5
#include <bit7z/bitexception.hpp>
#endif

#include "utils/shared_lib.hpp"

namespace bit7z {
namespace test {

TEST_CASE( "Bit7zLibrary: Constructing from a non-existing shared library", "[bit7zlibrary]" ) {
    REQUIRE_THROWS_WITH( Bit7zLibrary( BIT7Z_STRING( "NonExisting7z.dll" ) ),
                         Catch::Matchers::StartsWith( "Failed to load the 7-zip library" ) );

#ifdef _WIN32
    REQUIRE_THROWS_MATCHES( Bit7zLibrary( BIT7Z_STRING( "NonExisting7z.dll" ) ),
                            BitException,
                            Catch::Matchers::Predicate< BitException >( [ & ]( const BitException& ex ) -> bool {
                                return std::strncmp( ex.code().category().name(), "HRESULT", 8 );
                            }, "Error code should be E_FAIL" ) );
#elif !defined(__GNUC__) || __GNUC__ >= 5
    REQUIRE_THROWS_MATCHES( Bit7zLibrary( BIT7Z_STRING( "NonExisting7z.dll" ) ),
                            BitException,
                            Catch::Matchers::Predicate< BitException >( [ & ]( const BitException& ex ) -> bool {
                                return ex.code() == std::errc::bad_file_descriptor;
                            }, "Error code should be bad file descriptor" ) );
#endif
}

TEST_CASE( "Bit7zLibrary: Normal construction", "[bit7zlibrary]" ) {
    const auto libPath = sevenzip_lib_path();
    INFO( "Library path: " << fs::path{ libPath }.string() )

    REQUIRE_NOTHROW( Bit7zLibrary{ libPath } );
}

// Note: we are assuming that the 7-zip shared library was built with support to large pages.
// This is usually true due to the default compilation flags of the 7-zip libraries.
TEST_CASE( "Bit7zLibrary: Set 7-zip to use the large page mode", "[bit7zlibrary]" ) {
    const auto libPath = sevenzip_lib_path();

    Bit7zLibrary lib{ libPath };
    REQUIRE_NOTHROW( lib.setLargePageMode() );
}

} // namespace test
} // namespace bit7z