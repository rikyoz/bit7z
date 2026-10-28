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

#include "internal/cbufferoutstream.hpp"

#include <bit7z/bittypes.hpp>

using namespace bit7z;

TEST_CASE( "CBufferOutStream: SetSize preserves a valid write position", "[cbufferoutstream]" ) {
    SECTION( "Growing the buffer" ) {
        buffer_t buffer{ static_cast< byte_t >( 'A' ), static_cast< byte_t >( 'B' ), static_cast< byte_t >( 'C' ) };
        buffer.shrink_to_fit();
        CBufferOutStream outStream{ buffer };

        REQUIRE( outStream.Seek( 2, STREAM_SEEK_SET, nullptr ) == S_OK );
        REQUIRE( outStream.SetSize( 1024 ) == S_OK );

        const auto replacement = static_cast< byte_t >( 'X' );
        UInt32 processedSize{ 0 };
        REQUIRE( outStream.Write( &replacement, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( buffer[ 2 ] == replacement );
    }

    SECTION( "Shrinking below the current position clamps the next write to the new end" ) {
        buffer_t buffer{ static_cast< byte_t >( 'A' ), static_cast< byte_t >( 'B' ), static_cast< byte_t >( 'C' ),
                         static_cast< byte_t >( 'D' ) };
        CBufferOutStream outStream{ buffer };

        REQUIRE( outStream.Seek( 4, STREAM_SEEK_SET, nullptr ) == S_OK );
        REQUIRE( outStream.SetSize( 2 ) == S_OK );

        const auto replacement = static_cast< byte_t >( 'X' );
        UInt32 processedSize{ 0 };
        REQUIRE( outStream.Write( &replacement, 1, &processedSize ) == S_OK );
        REQUIRE( processedSize == 1 );
        REQUIRE( buffer.size() == 3 );
        REQUIRE( buffer[ 0 ] == static_cast< byte_t >( 'A' ) );
        REQUIRE( buffer[ 1 ] == static_cast< byte_t >( 'B' ) );
        REQUIRE( buffer[ 2 ] == replacement );
    }
}
