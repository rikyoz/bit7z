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
#include "utils/shared_lib.hpp"

#include <bit7z/bitabstractarchivehandler.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bittypes.hpp>

#include <cstdint>
#include <utility>

using namespace bit7z;

namespace {
// A minimal archive handler, used for testing BitAbstractArchiveHandler's own non-virtual settings.
class DummyHandler final : public BitAbstractArchiveHandler {
    public:
        explicit DummyHandler( tstring password = {}, OverwriteMode overwriteMode = OverwriteMode::None )
            : BitAbstractArchiveHandler{ test::sevenzipLib(), std::move( password ), overwriteMode } {}

        BIT7Z_NODISCARD auto format() const noexcept -> const BitInFormat& override {
            return BitFormat::SevenZip;
        }
};
} // namespace

TEST_CASE( "BitAbstractArchiveHandler: constructing a handler", "[bitabstractarchivehandler]" ) {
    SECTION( "Using the default arguments" ) {
        const DummyHandler handler;
        REQUIRE( &handler.library() == &test::sevenzipLib() );
        REQUIRE( handler.password().empty() );
        REQUIRE_FALSE( handler.isPasswordDefined() );
        REQUIRE( handler.overwriteMode() == OverwriteMode::None );
        REQUIRE( handler.retainDirectories() );
    }

    SECTION( "Using an explicit password and overwrite mode" ) {
        const DummyHandler handler{ BIT7Z_STRING( "ciao" ), OverwriteMode::Skip };
        REQUIRE( &handler.library() == &test::sevenzipLib() );
        REQUIRE( handler.password() == BIT7Z_STRING( "ciao" ) );
        REQUIRE( handler.isPasswordDefined() );
        REQUIRE( handler.overwriteMode() == OverwriteMode::Skip );
        REQUIRE( handler.retainDirectories() );
    }
}

TEST_CASE(
    "BitAbstractArchiveHandler: setPassword(...) / clearPassword() / password() / isPasswordDefined()",
    "[bitabstractarchivehandler]"
) {
    DummyHandler handler;
    REQUIRE( handler.password().empty() );
    REQUIRE_FALSE( handler.isPasswordDefined() );

    handler.setPassword( BIT7Z_STRING( "ciao" ) );
    REQUIRE( handler.password() == BIT7Z_STRING( "ciao" ) );
    REQUIRE( handler.isPasswordDefined() );

    // Setting an empty password clears the previously set one.
    handler.setPassword( BIT7Z_STRING( "" ) );
    REQUIRE( handler.password().empty() );
    REQUIRE_FALSE( handler.isPasswordDefined() );

    handler.setPassword( BIT7Z_STRING( "mondo" ) );
    REQUIRE( handler.isPasswordDefined() );

    handler.clearPassword();
    REQUIRE( handler.password().empty() );
    REQUIRE_FALSE( handler.isPasswordDefined() );
}

TEST_CASE(
    "BitAbstractArchiveHandler: setRetainDirectories(...) / retainDirectories()",
    "[bitabstractarchivehandler]"
) {
    DummyHandler handler;
    REQUIRE( handler.retainDirectories() );

    handler.setRetainDirectories( false );
    REQUIRE_FALSE( handler.retainDirectories() );

    handler.setRetainDirectories( true );
    REQUIRE( handler.retainDirectories() );
}

TEST_CASE( "BitAbstractArchiveHandler: setOverwriteMode(...) / overwriteMode()", "[bitabstractarchivehandler]" ) {
    DummyHandler handler;
    REQUIRE( handler.overwriteMode() == OverwriteMode::None );

    handler.setOverwriteMode( OverwriteMode::Overwrite );
    REQUIRE( handler.overwriteMode() == OverwriteMode::Overwrite );

    handler.setOverwriteMode( OverwriteMode::Skip );
    REQUIRE( handler.overwriteMode() == OverwriteMode::Skip );

    handler.setOverwriteMode( OverwriteMode::None );
    REQUIRE( handler.overwriteMode() == OverwriteMode::None );
}

TEST_CASE(
    "BitAbstractArchiveHandler: setting, calling, and resetting the callbacks",
    "[bitabstractarchivehandler]"
) {
    DummyHandler handler;

    SECTION( "setTotalCallback(...) / totalCallback()" ) {
        REQUIRE( handler.totalCallback() == nullptr );

        std::uint64_t totalSize = 0;
        handler.setTotalCallback( [ &totalSize ]( std::uint64_t total ) -> void { totalSize = total; } );
        REQUIRE( handler.totalCallback() != nullptr );

        constexpr auto totalValue = 42u;
        handler.totalCallback()( totalValue );
        REQUIRE( totalSize == totalValue );

        handler.setTotalCallback( nullptr );
        REQUIRE( handler.totalCallback() == nullptr );
    }

    SECTION( "setProgressCallback(...) / progressCallback()" ) {
        REQUIRE( handler.progressCallback() == nullptr );

        handler.setProgressCallback( []( std::uint64_t progress ) -> bool { return progress > 0; } );
        REQUIRE( handler.progressCallback() != nullptr );

        REQUIRE( handler.progressCallback()( 42u ) );
        REQUIRE_FALSE( handler.progressCallback()( 0u ) );

        handler.setProgressCallback( nullptr );
        REQUIRE( handler.progressCallback() == nullptr );
    }

    SECTION( "setRatioCallback(...) / ratioCallback()" ) {
        REQUIRE( handler.ratioCallback() == nullptr );

        std::uint64_t inputSize = 0;
        std::uint64_t outputSize = 0;
        handler.setRatioCallback( [ &inputSize, &outputSize ]( std::uint64_t input, std::uint64_t output ) -> void {
            inputSize = input;
            outputSize = output;
        } );
        REQUIRE( handler.ratioCallback() != nullptr );

        handler.ratioCallback()( 1024u, 512u );
        REQUIRE( inputSize == 1024u );
        REQUIRE( outputSize == 512u );

        handler.setRatioCallback( nullptr );
        REQUIRE( handler.ratioCallback() == nullptr );
    }

    SECTION( "setFileCallback(...) / fileCallback()" ) {
        REQUIRE( handler.fileCallback() == nullptr );

        tstring processedFile;
        handler.setFileCallback( [ &processedFile ]( const tstring& file ) -> void { // NOSONAR
            processedFile = file;
        } );
        REQUIRE( handler.fileCallback() != nullptr );

        using test::filesystem::italy;
        handler.fileCallback()( italy.name );
        REQUIRE( processedFile == italy.name );

        handler.setFileCallback( nullptr );
        REQUIRE( handler.fileCallback() == nullptr );
    }

    SECTION( "setPasswordCallback(...) / passwordCallback()" ) {
        REQUIRE( handler.passwordCallback() == nullptr );

        handler.setPasswordCallback( []() -> tstring { return BIT7Z_STRING( "ciao" ); } );
        REQUIRE( handler.passwordCallback() != nullptr );

        REQUIRE( handler.passwordCallback()() == BIT7Z_STRING( "ciao" ) );

        // Note: the password callback is only used when no password was set on the handler.
        REQUIRE( handler.password().empty() );
        REQUIRE_FALSE( handler.isPasswordDefined() );

        handler.setPasswordCallback( nullptr );
        REQUIRE( handler.passwordCallback() == nullptr );
    }
}

