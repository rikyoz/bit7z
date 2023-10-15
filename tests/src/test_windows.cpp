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

#ifndef _WIN32

#include <catch2/catch.hpp>

#include <array>
#include <cstring>
#include <iostream>

#include <internal/windows.hpp>

TEST_CASE( "winapi: Checking error code macros correct values", "[winapi]" ) {
#ifdef MY__E_ERROR_NEGATIVE_SEEK
    REQUIRE( HRESULT_WIN32_ERROR_NEGATIVE_SEEK == MY__E_ERROR_NEGATIVE_SEEK );
#elif defined( MY_E_ERROR_NEGATIVE_SEEK )
    REQUIRE( HRESULT_WIN32_ERROR_NEGATIVE_SEEK == MY_E_ERROR_NEGATIVE_SEEK );
#else
    REQUIRE( HRESULT_WIN32_ERROR_NEGATIVE_SEEK == HRESULT_FROM_WIN32(ERROR_NEGATIVE_SEEK) );
#endif
}

TEST_CASE( "winapi: Allocating BSTR string from nullptr C strings", "[winapi][string allocation]" ) {
    SECTION( "Using SysAllocString" ) {
        REQUIRE( SysAllocString( nullptr ) == nullptr );
    }

    SECTION( "Using a specific length" ) {
        BSTR resultString = nullptr;
        const size_t testLength = GENERATE( 0, 1, 42, 127, 128 );

        DYNAMIC_SECTION( "SysAllocStringLen with length " << testLength ) {
            resultString = SysAllocStringLen( nullptr, testLength );
            REQUIRE( SysStringLen( resultString ) == testLength );
            REQUIRE( SysStringByteLen( resultString ) == testLength * sizeof( std::remove_pointer< BSTR >::type ) );
        }

        DYNAMIC_SECTION( "SysAllocStringByteLen with byte length " << testLength ) {
            resultString = SysAllocStringByteLen( nullptr, testLength );
            REQUIRE( SysStringLen( resultString ) == testLength / sizeof( std::remove_pointer< BSTR >::type ) );
            REQUIRE( SysStringByteLen( resultString ) == testLength );
        }

        REQUIRE( resultString != nullptr );
        REQUIRE( resultString == std::wstring{} );
        REQUIRE_NOTHROW( SysFreeString( resultString ) );
    }

    SECTION( "Using the max value for the length type" ) {
        auto length = std::numeric_limits< UINT >::max();
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );
        REQUIRE( SysAllocStringByteLen( nullptr, length ) == nullptr );
    }

#if INTPTR_MAX == INT64_MAX
    SECTION( "Using a length value that wraps around" ) {
        auto length = 0xC0000000;
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );

        length = ( std::numeric_limits< UINT >::max() / sizeof( OLECHAR ) );
        REQUIRE( SysAllocStringLen( nullptr, length ) == nullptr );

        length = 0x30000000;
        BSTR string = SysAllocStringLen( nullptr, length );
        REQUIRE( string != nullptr );
        REQUIRE( string == std::wstring{} );
        REQUIRE( SysStringLen( string ) == length );
        REQUIRE_NOTHROW( SysFreeString( string ) );
    }
#endif
}

TEST_CASE( "winapi: Handling nullptr BSTR strings", "[winapi][nullptr BSTR]" ) {
    REQUIRE( SysStringByteLen( nullptr ) == 0 );
    REQUIRE( SysStringLen( nullptr ) == 0 );
    REQUIRE_NOTHROW( SysFreeString( nullptr ) );
}

TEST_CASE( "winapi: Allocating from wide strings", "[winapi][string allocation]" ) {
    const auto* testStr = GENERATE( as< const wchar_t* >(),
                                    L"",
                                    L"h",
                                    L"hello world!",
                                    L"supercalifragilistichespiralidoso",
                                    L"perché",
                                    L"\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" // メタルガルルモン
    );

    DYNAMIC_SECTION( "Testing L" << Catch::StringMaker< std::wstring >::convert( testStr ) << " wide string" ) {
        std::wstring expectedString;
        BSTR resultString = nullptr;

        SECTION( "SysAllocString" ) {
            expectedString = testStr;
            resultString = SysAllocString( testStr );
        }

        SECTION( "SysAllocStringLen" ) {
            expectedString = testStr;
            resultString = SysAllocStringLen( testStr, expectedString.size() );
        }

        SECTION( "SysAllocStringLen with half-length parameter" ) {
            // Note: flawfinder warns about potentially using non-null terminating strings,
            // but, in our case, the test string is guaranteed to be null-terminated.
            expectedString = std::wstring{ testStr, std::wcslen( testStr ) / 2 }; // flawfinder: ignore
            resultString = SysAllocStringLen( testStr, expectedString.size() );
        }

        SECTION( "SysAllocStringLen with zero length parameter" ) {
            // expectedString is already empty here
            resultString = SysAllocStringLen( testStr, 0 );
        }

        REQUIRE( resultString != nullptr );
        REQUIRE( resultString == expectedString );
        REQUIRE( SysStringLen( resultString ) == expectedString.size() );
        REQUIRE( SysStringByteLen( resultString ) == ( expectedString.size() * sizeof( OLECHAR ) ) );

        REQUIRE_NOTHROW( SysFreeString( resultString ) );
    }
}

TEST_CASE( "winapi: Allocating from narrow strings", "[winapi][string allocation]" ) {
    const auto* testStr = GENERATE( as< const char* >(),
                                     "",
                                     "h",
                                     "hello world!",
                                     "supercalifragilistichespiralidoso",
                                     "perché",
                                     "\u30e1\u30bf\u30eb\u30ac\u30eb\u30eb\u30e2\u30f3" // メタルガルルモン
    );

    DYNAMIC_SECTION( "Testing " << Catch::StringMaker< std::string >::convert( testStr ) << " string" ) {
        BSTR resultString = nullptr;
        std::string expectedString;

        SECTION( "SysAllocStringByteLen" ) {
            expectedString = testStr;
            resultString = SysAllocStringByteLen( testStr, expectedString.size() );
        }

        SECTION( "SysAllocStringByteLen with half-length parameter" ) {
            // Note: flawfinder warns about potentially using non-null terminating strings,
            // but, in our case, the test string is guaranteed to be null-terminated.
            expectedString = std::string{ testStr, std::strlen( testStr ) / 2 }; // flawfinder: ignore
            resultString = SysAllocStringByteLen( testStr, expectedString.size() );
        }

        SECTION( "SysAllocStringByteLen with zero length parameter" ) {
            // expectedString is already empty here
            resultString = SysAllocStringByteLen( testStr, 0 );
        }

        REQUIRE( resultString != nullptr );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        REQUIRE( reinterpret_cast< const char* >( resultString ) == expectedString );
        REQUIRE( SysStringLen( resultString ) == ( expectedString.size() / sizeof( OLECHAR ) ) );
        REQUIRE( SysStringByteLen( resultString ) == expectedString.size() );

        REQUIRE_NOTHROW( SysFreeString( resultString ) );
    }
}

#endif