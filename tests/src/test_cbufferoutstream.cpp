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

#include <bit7z/bitwindows.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/cbufferoutstream.hpp>

#include <limits>

using bit7z::buffer_t;
using bit7z::CBufferOutStream;

TEST_CASE( "CBufferOutStream: Seeking with an invalid origin", "[cbufferoutstream][seeking]" ) {
    buffer_t buffer( 1024 );
    CBufferOutStream outStream{ buffer };
    UInt64 newPosition{ 0 };

    // Only STREAM_SEEK_SET (0), STREAM_SEEK_CUR (1), and STREAM_SEEK_END (2) are valid origins.
    // The 42 and max() cases also guard against an origin being narrowed onto a valid enumerator
    // (e.g., 258 would alias to STREAM_SEEK_END under a plain cast) instead of being rejected.
    const UInt32 invalidOrigin = GENERATE( 3u, 42u, 258u, std::numeric_limits< UInt32 >::max() );

    DYNAMIC_SECTION( "Invalid seek origin " << invalidOrigin ) {
        REQUIRE( outStream.Seek( 0, invalidOrigin, &newPosition ) == STG_E_INVALIDFUNCTION );
        REQUIRE( newPosition == 0 ); // The output value was not changed.
    }
}
