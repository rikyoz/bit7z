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

#include "utils/buffer.hpp"

#include <bit7z/bitwindows.hpp>
#include <bit7z/bittypes.hpp>
#include <internal/cbufferoutstream.hpp>

#include <limits>

using bit7z::buffer_t;
using bit7z::CBufferOutStream;
using bit7z::test::asBytes;

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

// This is how 7-Zip takes back what it just wrote (e.g., the Tar handler when a file changed size while being read,
// or the Wim one when the data turns out to be a duplicate): it seeks back, then cuts the stream at that position.
TEST_CASE( "CBufferOutStream: Writing after shrinking to the current position", "[cbufferoutstream][resizing]" ) {
    buffer_t buffer;
    CBufferOutStream outStream{ buffer };

    UInt32 processedSize{ 0 };
    REQUIRE( outStream.Write( "Hello, world!", 13, &processedSize ) == S_OK );
    REQUIRE( outStream.Seek( -8, STREAM_SEEK_CUR, nullptr ) == S_OK );

    REQUIRE( outStream.SetSize( 5 ) == S_OK );
    REQUIRE( buffer == asBytes( "Hello" ) );

    REQUIRE( outStream.Write( " there", 6, &processedSize ) == S_OK );
    REQUIRE( processedSize == 6 );
    REQUIRE( buffer == asBytes( "Hello there" ) );
}

TEST_CASE( "CBufferOutStream: Growing the buffer keeps the current position", "[cbufferoutstream][resizing]" ) {
    buffer_t buffer;
    CBufferOutStream outStream{ buffer };

    UInt32 processedSize{ 0 };
    REQUIRE( outStream.Write( "abc", 3, &processedSize ) == S_OK );

    // Growing past the capacity makes the buffer reallocate, moving the bytes the position pointed to.
    const auto grownSize = buffer.capacity() + 1;
    REQUIRE( outStream.SetSize( grownSize ) == S_OK );

    UInt64 position{ 0 };
    REQUIRE( outStream.Seek( 0, STREAM_SEEK_CUR, &position ) == S_OK );
    REQUIRE( position == 3 );

    REQUIRE( outStream.Write( "d", 1, &processedSize ) == S_OK );
    buffer_t expectedBuffer = asBytes( "abcd" );
    expectedBuffer.resize( grownSize ); // The bytes the buffer grew by are zeros.
    REQUIRE( buffer == expectedBuffer );
}

TEST_CASE(
    "CBufferOutStream: Shrinking below the current position clamps it to the new end",
    "[cbufferoutstream][resizing]"
) {
    buffer_t buffer;
    CBufferOutStream outStream{ buffer };

    UInt32 processedSize{ 0 };
    REQUIRE( outStream.Write( "Hello, world!", 13, &processedSize ) == S_OK );

    REQUIRE( outStream.SetSize( 5 ) == S_OK );
    REQUIRE( buffer == asBytes( "Hello" ) );

    UInt64 position{ 0 };
    REQUIRE( outStream.Seek( 0, STREAM_SEEK_CUR, &position ) == S_OK );
    REQUIRE( position == 5 );

    REQUIRE( outStream.Write( " there", 6, &processedSize ) == S_OK );
    REQUIRE( buffer == asBytes( "Hello there" ) ); // The write operation appended at the new end.
}
