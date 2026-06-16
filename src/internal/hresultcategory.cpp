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

#include "internal/hresultcategory.hpp"

#ifdef __MINGW32__
#include "internal/win32category.hpp"
#endif
#include "internal/windows.hpp"

#include <string>
#include <system_error>

namespace bit7z {

auto HRESULTCategory::name() const noexcept -> const char* {
    return "HRESULT";
}

auto HRESULTCategory::message( int errorValue ) const -> std::string {
#ifdef _WIN32
    // Note: also MinGW supports FormatMessageA, so we use it.
    LPSTR messageBuffer = nullptr;
    const auto msgSize = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr,
        static_cast< DWORD >( errorValue ),
        0,
        reinterpret_cast< LPSTR >( &messageBuffer ), // NOLINT(*-pro-type-reinterpret-cast)
        0,
        nullptr
    );
    if ( msgSize == 0 ) {
        return "Unknown error";
    }
    /* Note: strings obtained using FormatMessageA have a trailing space, and a \r\n pair of char.
     *       Using the flag FORMAT_MESSAGE_MAX_WIDTH_MASK removes the ending \r\n but leaves the trailing space.
     *       For this reason, we create the resulting std::string by considering msgSize - 1 as string size. */
    std::string errorMessage( messageBuffer, msgSize - 1 );
    LocalFree( messageBuffer );
    return errorMessage;
#else
    // Note: same messages returned by FormatMessageA on Windows.
    switch ( static_cast< HRESULT >( errorValue ) ) {
        case E_ABORT:
            return "Operation aborted";
        case E_NOTIMPL:
            return "Not implemented";
        case E_NOINTERFACE:
            return "No such interface supported";
        case E_INVALIDARG:
            return "The parameter is incorrect.";
        case STG_E_INVALIDFUNCTION:
            return "Unable to perform requested operation.";
        case E_OUTOFMEMORY:
            return "Not enough memory resources are available to complete this operation.";
        case HRESULT_FROM_WIN32( ERROR_DIRECTORY ):
            /* Note: p7zip does not use POSIX-equivalent error codes for ERROR_DIRECTORY and ERROR_NO_MORE_FILES
             *       so we need to handle also these cases here. */
            return "The directory name is invalid.";
        case HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES ):
            return "There are no more files.";
        case HRESULT_WIN32_ERROR_NEGATIVE_SEEK:
            /* Note: p7zip and 7-zip do not use a POSIX-equivalent error code for ERROR_NEGATIVE_SEEK, but rather
             *       its Win32 value; both also use the FACILITY_WIN32 (7) as facility code for the corresponding
             *       HRESULT_WIN32_ERROR_NEGATIVE_SEEK, so we cannot use 7-zip's HRESULT_FROM_WIN32 macro
             *       since it uses another facility code, i.e., FACILITY_ERRNO (0x800).
             *       Hence, we simply check for the final HRESULT code. */
            return "An attempt was made to move the file pointer before the beginning of the file.";
        case E_FAIL:
            return "Unspecified error";
        default:
            if ( HRESULT_FACILITY( errorValue ) == FACILITY_CODE ) {
                // POSIX error code wrapped in a HRESULT value (e.g., through HRESULT_FROM_WIN32 macro)
                return std::system_category().message( HRESULT_CODE( errorValue ) );
            }
            return "Unknown HRESULT error (code " + std::to_string( errorValue ) + ").";
    }
#endif
}

auto HRESULTCategory::default_error_condition( int errorValue ) const noexcept -> std::error_condition {
    switch ( static_cast< HRESULT >( errorValue ) ) {
        // Note: in all cases, except the default one, error's category is std::generic_category(), i.e., POSIX errors.
        case E_ABORT:
            return std::make_error_condition( std::errc::operation_canceled );
        case E_NOTIMPL:
            // e.g., function not implemented
            return std::make_error_condition( std::errc::function_not_supported );
#ifdef _WIN32
        case __HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED ):
#endif
        case E_NOINTERFACE:
            // e.g., function implemented, parameters ok, but the requested functionality is not available.
            return std::make_error_condition( std::errc::not_supported );
#ifdef _WIN32
        case E_PENDING:
            return std::make_error_condition( std::errc::resource_unavailable_try_again );
        case E_POINTER:
        case E_HANDLE:
#endif
        case E_INVALIDARG:
        case STG_E_INVALIDFUNCTION: // 7-zip uses this for wrong seekOrigin parameters in the stream classes functions.
        case HRESULT_WIN32_ERROR_NEGATIVE_SEEK:
            return std::make_error_condition( std::errc::invalid_argument );
        case __HRESULT_FROM_WIN32( ERROR_DIRECTORY ):
            return std::make_error_condition( std::errc::not_a_directory );
        case E_OUTOFMEMORY:
            return std::make_error_condition( std::errc::not_enough_memory );
        default:
            if ( HRESULT_FACILITY( errorValue ) == FACILITY_CODE ) {
#ifndef __MINGW32__
                /* MinGW's std::system_category expects POSIX error codes, but on Windows errorValue is a Win32
                 * error wrapped in an HRESULT (e.g., via HRESULT_FROM_WIN32). Using system_category here would
                 * return a wrong error_condition, so MinGW delegates to win32Category instead (see else branch).
                 *
                 * On MSVC, the resulting error_condition's category is std::generic_category() for Win32 errors
                 * that map to a POSIX equivalent, or std::system_category() otherwise.
                 *
                 * On Linux, p7zip/7-Zip wraps most error codes as POSIX values inside HRESULTs, so this line returns
                 * the correct error_condition.
                 * The few p7zip/7-Zip codes that match Windows error codes are handled in the other switch cases.
                 */
                return std::system_category().default_error_condition( HRESULT_CODE( errorValue ) );
#else
                return bit7z::win32Category().default_error_condition( HRESULT_CODE( errorValue ) );
#endif
            }
            /* E.g., E_FAIL
             * Note: the resulting error_condition's category is std::hresult_category() */
            return error_category::default_error_condition( errorValue );
    }
}

auto hresultCategory() noexcept -> const std::error_category& {
    static const bit7z::HRESULTCategory instance{};
    return instance;
}

} // namespace bit7z
