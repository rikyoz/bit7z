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

#include <catch2/catch.hpp>

#include "utils/filesystem.hpp"

#include <bit7z/bitwindows.hpp>
#include <internal/cfileinstream.hpp>

#include <limits>

#ifdef BIT7Z_TESTS_FILESYSTEM

using namespace bit7z;
using namespace bit7z::test::filesystem;

TEST_CASE( "CFileInStream: Seeking with an invalid origin", "[cfileinstream][seeking]" ) {
    // The invalid origin is rejected before any read, so any existing, openable file works as input.
    const fs::path filePath = fs::path{ test_filesystem_dir } / italy.name;
    CFileInStream inStream{ filePath.native() };
    UInt64 newPosition{ 0 };

    // Only STREAM_SEEK_SET (0), STREAM_SEEK_CUR (1), and STREAM_SEEK_END (2) are valid origins.
    // The 258 case guards against an out-of-range origin being narrowed onto a valid enumerator
    // (258 would alias to STREAM_SEEK_END under a plain cast) instead of being rejected.
    const UInt32 invalidOrigin = GENERATE( 3u, 42u, 258u, std::numeric_limits< UInt32 >::max() );

    DYNAMIC_SECTION( "Invalid seek origin " << invalidOrigin ) {
        REQUIRE( inStream.Seek( 0, invalidOrigin, &newPosition ) == STG_E_INVALIDFUNCTION );
        REQUIRE( newPosition == 0 ); // The output value was not changed.
    }
}

#endif
