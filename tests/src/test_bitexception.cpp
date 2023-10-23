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

#if !defined(__GNUC__) || __GNUC__ >= 5 || defined( __clang__ )

#include <catch2/catch.hpp>

#include <bit7z/bitexception.hpp>
#include <internal/windows.hpp>

#include <iostream>

using bit7z::BitException;

struct PortableErrorTest { //-V802
    const char* name;
    HRESULT error;
    const char* message;
    std::errc portableError;
};

#define ERROR_TEST( code ) #code, code
#define HRESULT_WIN32_TEST( code ) #code, __HRESULT_FROM_WIN32( code )

constexpr PortableErrorTest hresult_tests[] = { // NOLINT(*-avoid-c-arrays)
    { ERROR_TEST( E_ABORT ), "Operation aborted", std::errc::operation_canceled },
    { ERROR_TEST( E_NOTIMPL ), "Not implemented", std::errc::function_not_supported },
    { ERROR_TEST( E_NOINTERFACE ), "No such interface supported", std::errc::not_supported },
    { ERROR_TEST( E_INVALIDARG ), "The parameter is incorrect.", std::errc::invalid_argument },
    { ERROR_TEST( STG_E_INVALIDFUNCTION ), "Unable to perform requested operation.", std::errc::invalid_argument },
    { ERROR_TEST( E_OUTOFMEMORY ),
      "Not enough memory resources are available to complete this operation.",
      std::errc::not_enough_memory },
#ifdef _WIN32
    { ERROR_TEST( E_PENDING ),
      "The data necessary to complete this operation is not yet available.",
      std::errc::resource_unavailable_try_again },
    { ERROR_TEST( E_POINTER ), "Invalid pointer", std::errc::invalid_argument },
#endif
    { HRESULT_WIN32_TEST( ERROR_OPEN_FAILED ),
#ifdef _WIN32
      "The system cannot open the device or file specified.",
#else
        "Input/output error",
#endif
      std::errc::io_error },
#ifdef _WIN32
    { HRESULT_WIN32_TEST( ERROR_FILE_NOT_FOUND ),
      "The system cannot find the file specified.",
      std::errc::no_such_file_or_directory },
    { HRESULT_WIN32_TEST( ERROR_ACCESS_DENIED ),
      "Access is denied.",
      std::errc::permission_denied },
    { HRESULT_WIN32_TEST( ERROR_NOT_SUPPORTED ),
      "The request is not supported.",
      std::errc::not_supported },
#endif
    { HRESULT_WIN32_TEST( ERROR_SEEK ),
#ifdef _WIN32
      "The drive cannot locate a specific area or track on the disk.",
#else
        "Input/output error",
#endif
      std::errc::io_error },
    { HRESULT_WIN32_TEST( ERROR_READ_FAULT ),
#ifdef _WIN32
      "The system cannot read from the specified device.",
#else
        "Input/output error",
#endif
      std::errc::io_error },
    { HRESULT_WIN32_TEST( ERROR_WRITE_FAULT ),
#ifdef _WIN32
      "The system cannot write to the specified device.",
#else
        "Input/output error",
#endif
      std::errc::io_error },
    { HRESULT_WIN32_TEST( ERROR_ALREADY_EXISTS ),
#ifdef _WIN32
      "Cannot create a file when that file already exists.",
#else
        "File exists",
#endif
      std::errc::file_exists },
    { HRESULT_WIN32_TEST( ERROR_FILE_EXISTS ),
#ifdef _WIN32
      "The file exists.",
#else
        "File exists",
#endif
      std::errc::file_exists },
    { HRESULT_WIN32_TEST( ERROR_INVALID_HANDLE ),
#ifdef _WIN32
      "The handle is invalid.", std::errc::invalid_argument
#else
        "Bad file descriptor", std::errc::bad_file_descriptor
#endif
    },
    { HRESULT_WIN32_TEST( ERROR_PATH_NOT_FOUND ),
#ifdef _WIN32
      "The system cannot find the path specified.",
#else
        "No such file or directory",
#endif
      std::errc::no_such_file_or_directory },
    { HRESULT_WIN32_TEST( ERROR_DISK_FULL ),
#ifdef _WIN32
      "There is not enough space on the disk.",
#else
        "No space left on device",
#endif
      std::errc::no_space_on_device },
    { HRESULT_WIN32_TEST( ERROR_DIRECTORY ),
      "The directory name is invalid.",
      std::errc::not_a_directory },
    { ERROR_TEST( HRESULT_WIN32_ERROR_NEGATIVE_SEEK ),
      "An attempt was made to move the file pointer before the beginning of the file.",
      std::errc::invalid_argument }
};

TEST_CASE( "BitException: Constructing from an HRESULT error", "[BitException][HRESULT]" ) {
    for ( const auto& test : hresult_tests ) {
        DYNAMIC_SECTION( "Testing " << test.name << " (value 0x" << std::hex << test.error << std::dec << ")" ) {
            auto code = bit7z::make_hresult_code( test.error );

            REQUIRE( static_cast< HRESULT >( code.value() ) == test.error );
            REQUIRE( code.message() == test.message );
            REQUIRE( code == test.portableError );

            const auto exception = BitException( "Hello World", code );
#ifdef _WIN32
            REQUIRE( exception.nativeCode() == code.value() );
            REQUIRE( exception.hresultCode() == exception.nativeCode() );
            REQUIRE( exception.posixCode() == static_cast<int>( test.portableError ) );
#else
            REQUIRE( exception.nativeCode() == static_cast<int>( test.portableError ) );
            REQUIRE( exception.hresultCode() == test.error );
            REQUIRE( exception.posixCode() == exception.nativeCode() );
#endif

            REQUIRE( exception.what() == std::string{ "Hello World: " } + test.message );
        }
    }
}

#ifndef __MINGW32__

struct win32_error_test {
    const char* name;
    DWORD error;
};

constexpr win32_error_test win32_tests[] = { // NOLINT(*-avoid-c-arrays)
#ifdef _WIN32
    { ERROR_TEST( ERROR_FILE_NOT_FOUND ) },
    { ERROR_TEST( ERROR_NOT_SUPPORTED ) },
    { ERROR_TEST( ERROR_INVALID_PARAMETER ) },
    { ERROR_TEST( ERROR_OUTOFMEMORY ) },
    // ERROR_DIRECTORY should correspond to errc::not_a_directory; however, MSVC maps it to errc::invalid_argument
    { ERROR_TEST( ERROR_NEGATIVE_SEEK ) }, //ERROR_NEGATIVE_SEEK is not POSIX on p7zip
#endif
    { ERROR_TEST( ERROR_OPEN_FAILED ) },
    { ERROR_TEST( ERROR_SEEK ) },
    { ERROR_TEST( ERROR_READ_FAULT ) },
    { ERROR_TEST( ERROR_WRITE_FAULT ) },
    { ERROR_TEST( ERROR_PATH_NOT_FOUND ) },
    { ERROR_TEST( ERROR_ALREADY_EXISTS ) },
    { ERROR_TEST( ERROR_FILE_EXISTS ) },
    { ERROR_TEST( ERROR_DISK_FULL ) },
    { ERROR_TEST( ERROR_INVALID_HANDLE ) }
};

/* The following tests assume that std::system_category() refers to either Win32 error codes on Windows,
 * and POSIX error codes on Unix (i.e., the native error codes).
 * However, MinGW uses the std::system_category() for POSIX error codes on Windows, so we don't test it. */

TEST_CASE( "BitException: Constructing from Win32/POSIX error codes", "[BitException][win32-posix]" ) {
    for ( const auto& test : win32_tests ) {
        DYNAMIC_SECTION( "Testing " << test.name << " (value 0x" << std::hex << test.error << std::dec << ")" ) {
            auto sys_error = std::error_code{ static_cast<int>( test.error ), std::system_category() };

#if defined(_MSC_VER) && _MSC_VER > 1900
            auto hresult_error = bit7z::make_hresult_code( HRESULT_FROM_WIN32( test.error ) );
            REQUIRE( sys_error.default_error_condition() == hresult_error.default_error_condition() );
#endif

            const auto exception = BitException( "Hello World", sys_error );
#ifdef _WIN32
            REQUIRE( exception.nativeCode() == HRESULT_FROM_WIN32( test.error ) );
            REQUIRE( exception.hresultCode() == exception.nativeCode() );
#else
            REQUIRE( exception.nativeCode() == test.error );
            if ( sys_error != std::errc::io_error ) { // Multiple Win32 errors might be mapped to the POSIX IO error.
                REQUIRE( exception.hresultCode() == HRESULT_FROM_WIN32( test.error ) );
            }
#endif
            REQUIRE( exception.posixCode() == sys_error.default_error_condition().value() );
            REQUIRE( exception.what() == std::string{ "Hello World: " } + sys_error.message() );
        }
    }
}

#endif

struct HRESULTErrorTest {
    const char* name;
    HRESULT error;
    const char* message;
};

constexpr HRESULTErrorTest unmapped_hresult_tests[] = { // NOLINT(*-avoid-c-arrays)
    // Tests for HRESULT values without a POSIX error counterpart
    { ERROR_TEST( E_FAIL ), "Unspecified error" },
#ifdef _WIN32
    { ERROR_TEST( STG_E_INVALIDPOINTER ), "Invalid pointer error." }
#endif
};

TEST_CASE( "BitException: Constructing std::error_code from unmapped HRESULT values", "[BitException][unmapped]" ) {
    for ( const auto& test : unmapped_hresult_tests ) {
        DYNAMIC_SECTION( "Testing " << test.name << " (value 0x" << std::hex << test.error << std::dec << ")" ) {
            auto hresultCode = bit7z::make_hresult_code( test.error );

            REQUIRE( hresultCode.message() == test.message );

            auto hresultCondition = hresultCode.default_error_condition();
            if ( HRESULT_FACILITY( test.error ) == FACILITY_WIN32 ) {
                REQUIRE( hresultCondition.value() == HRESULT_CODE( hresultCode.value() ) );
            } else {
                REQUIRE( hresultCondition.value() == hresultCode.value() );
                REQUIRE( hresultCondition.category() == hresultCode.category() );
            }
        }
    }
}

TEST_CASE( "BitException: Checking if failed files are moved to the exception constructor", "[bitexception]" ) {
    bit7z::FailedFiles failedFiles = { { BIT7Z_STRING( "hello.txt" ),
                                         std::make_error_code( std::errc::bad_file_descriptor ) } };
    const BitException exception{ "Error Message",
                                  std::make_error_code( std::errc::io_error ), std::move( failedFiles ) };
    REQUIRE( exception.code() == std::errc::io_error );
    const auto& exceptionFailedFiles = exception.failedFiles();
    REQUIRE( exceptionFailedFiles.size() == 1 );
    REQUIRE( exceptionFailedFiles[ 0 ].first == BIT7Z_STRING( "hello.txt" ) );
    REQUIRE( exceptionFailedFiles[ 0 ].second == std::errc::bad_file_descriptor );
    // Note: BitException should have cleared failedFiles, so it is again usable.
    REQUIRE( failedFiles.empty() ); // NOLINT(bugprone-use-after-move) //-V1030
}

#endif