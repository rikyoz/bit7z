// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/hresultcategory.hpp"

#include "../include/bittypes.hpp"

using namespace bit7z;

const char* hresult_category_t::name() const noexcept {
    return "HRESULT";
}

std::string hresult_category_t::message( int ev ) const {
#ifdef _MSC_VER
    // MSVC compilers use FormatMessage, which seems to support both Win32 errors and HRESULT com errors
    return std::system_category().message( ev );
#elif defined( __MINGW32__ )
    // MinGW supports FormatMessageA!
    LPSTR messageBuffer = nullptr;
    auto msgSize = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                   FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS |
                                   FORMAT_MESSAGE_MAX_WIDTH_MASK,
                                   nullptr, ev, 0, reinterpret_cast<LPSTR>( &messageBuffer ), 0, nullptr );
    if ( msgSize == 0 ) {
        return "Unknown error";
    }
    /* Note: strings obtained using FormatMessageA have a trailing space and a \r\n pair of char.
     *       Using the FORMAT_MESSAGE_MAX_WIDTH_MASK flag removes the ending \r\n but leaves the trailing space.
     *       For this reason, we create the resulting std::string by considering msgSize - 1 as string size! */
    std::string errorMessage( messageBuffer, msgSize - 1 );
    LocalFree( messageBuffer );
    return errorMessage;
#else
    // Note: same messages returned by FormatMessageA on Windows platform.
    switch ( static_cast< HRESULT >( ev ) ) {
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
        /* Note: p7zip does not use POSIX-equivalent error codes for ERROR_DIRECTORY, ERROR_NO_MORE_FILES, and
         *       ERROR_NEGATIVE_SEEK, so we need to handle also these cases here. */
        case __HRESULT_FROM_WIN32( ERROR_DIRECTORY ):
            return "The directory name is invalid.";
        case __HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES ):
            return "There are no more files.";
        case __HRESULT_FROM_WIN32( ERROR_NEGATIVE_SEEK ):
            return "An attempt was made to move the file pointer before the beginning of the file.";
        case E_FAIL:
            return "Unspecified error";
        default:
            if ( HRESULT_FACILITY( ev ) == FACILITY_WIN32 ) {
                // POSIX error code wrapped in a HRESULT value (e.g., through HRESULT_FROM_WIN32 macro)
                return std::system_category().message( HRESULT_CODE( ev ) );
            }
            return "Unknown error";
    }
#endif
}

std::error_condition hresult_category_t::default_error_condition( int ev ) const noexcept {
    switch ( static_cast< HRESULT >( ev ) ) {
        // Note: in all cases, except the default one, error's category is std::generic_category(), i.e., POSIX errors
        case E_ABORT:
            return std::make_error_condition( std::errc::operation_canceled );
        case E_NOTIMPL:
            // e.g., function not implemented
            return std::make_error_condition( std::errc::function_not_supported );
#ifdef __MINGW32__
        case HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED ):
#endif
        case E_NOINTERFACE:
            // e.g., function implemented, parameters ok, but the requested functionality is not available
            return std::make_error_condition( std::errc::not_supported );
#ifdef _WIN32
        case E_PENDING:
            return std::make_error_condition( std::errc::resource_unavailable_try_again );
        case E_POINTER:
        case E_HANDLE:
#endif
        case E_INVALIDARG:
        case STG_E_INVALIDFUNCTION: // 7-zip uses this for wrong seekOrigin parameters in stream classes functions.
        case __HRESULT_FROM_WIN32( ERROR_NEGATIVE_SEEK ):
            return std::make_error_condition( std::errc::invalid_argument );
        case __HRESULT_FROM_WIN32( ERROR_DIRECTORY ):
            return std::make_error_condition( std::errc::not_a_directory );
        case __HRESULT_FROM_WIN32( ERROR_NO_MORE_FILES ):
            return std::make_error_condition( std::errc::no_such_file_or_directory );
        case E_OUTOFMEMORY:
            return std::make_error_condition( std::errc::not_enough_memory );
        default:
            if ( HRESULT_FACILITY( ev ) == FACILITY_WIN32 ) {
#ifndef __MINGW32__
                /* Note: MinGW compilers use POSIX error codes for std::system_category instead of Win32 error codes.
                 * However, on Windows ev is a Win32 error wrapped into a HRESULT (e.g., through HRESULT_FROM_WIN32).
                 * Hence, to avoid returning a wrong error_condition, this check is not performed on MinGW,
                 * and instead we rely on specific cases for most common Win32 error codes (see 'else' branch).
                 *
                 * Note 2: on Windows (MSVC compilers) the resulting error_condition's category is
                 *         - std::generic_category() for Win32 errors that can be mapped to a POSIX error;
                 *         - std::system_category() otherwise.
                 *
                 * Note 3: on Linux, most ev values returned by p7zip are POSIX error codes wrapped
                 * into a HRESULT value, hence the following line will return the correct error_condition.
                 * Some error codes returned by p7zip are, however, equal to the Windows code: such cases are
                 * taken into account in specific cases above!
                 */
                return std::system_category().default_error_condition( HRESULT_CODE( ev ) );
#else
                switch ( HRESULT_CODE( ev ) ) {
                    case ERROR_ACCESS_DENIED:
                        return std::make_error_condition( std::errc::permission_denied );
                    case ERROR_OPEN_FAILED:
                    case ERROR_SEEK:
                    case ERROR_READ_FAULT:
                    case ERROR_WRITE_FAULT:
                        return std::make_error_condition( std::errc::io_error );
                    case ERROR_FILE_NOT_FOUND:
                    case ERROR_PATH_NOT_FOUND:
                    case ERROR_NO_MORE_FILES:
                        return std::make_error_condition( std::errc::no_such_file_or_directory );
                    case ERROR_ALREADY_EXISTS:
                    case ERROR_FILE_EXISTS:
                        return std::make_error_condition( std::errc::file_exists );
                    case ERROR_DISK_FULL:
                        return std::make_error_condition( std::errc::no_space_on_device );
                    default: // do nothing;
                        break;
                }
#endif
            }
            /* E.g., E_FAIL
               Note: the resulting error_condition's category is std::hresult_category() */
            return error_category::default_error_condition( ev );
    }
}

std::error_category& bit7z::hresult_category() noexcept {
    static bit7z::hresult_category_t instance{};
    return instance;
}
