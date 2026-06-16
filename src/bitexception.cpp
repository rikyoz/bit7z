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

#include "bitexception.hpp"

#include "biterror.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"
#include "internal/internalcategory.hpp"
#include "internal/hresultcategory.hpp"
#include "internal/operationcategory.hpp"
#ifdef __MINGW32__
#include "internal/win32category.hpp"
#elif !defined( _WIN32 )
#include "internal/windows.hpp"
#endif

#include <string>
#include <utility>
#include <system_error>

namespace bit7z {

auto make_hresult_code( HRESULT res ) noexcept -> std::error_code {
    return std::error_code{ static_cast< int >( res ), bit7z::hresultCategory() };
}

auto lastErrorCode() noexcept -> std::error_code {
    const auto error = static_cast< int >( GetLastError() );
#ifdef __MINGW32__
    /* MinGW's std::system_category uses POSIX errno semantics rather than Win32 error codes,
     * so storing a raw Win32 value under it would break std::errc comparisons.
     * win32Category provides a dedicated category with the correct Win32 -> std::errc mapping for MinGW. */
    return std::error_code{ error, bit7z::win32Category() };
#else
    return std::error_code{ error, std::system_category() };
#endif
}

BitException::BitException( const char* const message, std::error_code code, FailedFiles&& files )
    : std::system_error( code, message ), mFailedFiles( std::move( files ) ) {
    files.clear();
}

BitException::BitException( const char* const message, std::error_code code, tstring&& file )
    : std::system_error( code, message ), mFailedFiles{ std::make_pair( std::move( file ), code ) } {}

BitException::BitException( const char* const message, std::error_code code, const tstring& file )
    : std::system_error( code, message ), mFailedFiles{ std::make_pair( file, code ) } {}

BitException::BitException( const std::string& message, std::error_code code )
    : std::system_error( code, message ) {}

auto BitException::failedFiles() const noexcept -> const FailedFiles& {
    return mFailedFiles;
}

auto BitException::nativeCode() const noexcept -> BitException::native_code_type {
#ifdef _WIN32 // On Windows, the native code must be a HRESULT value.
    return hresultCode();
#else // On Unix, the native code is a POSIX error code.
    return posixCode();
#endif
}

auto BitException::hresultCode() const noexcept -> HRESULT {
    const std::error_code& error = code();
    if ( error.category() == bit7z::hresultCategory() ) { // Already a HRESULT value
        return error.value();
    }
#ifdef _MSC_VER
    // Note: MinGW considers POSIX error codes in std::system_category, so this code is valid only for MSVC
    if ( error.category() == std::system_category() ) { // Win32 error code
        return HRESULT_FROM_WIN32( static_cast< DWORD >( error.value() ) );
    }
#elif defined( __MINGW32__ )
    if ( error.category() == bit7z::win32Category() ) { // Win32 error code (MinGW)
        return HRESULT_FROM_WIN32( static_cast< DWORD >( error.value() ) );
    }
#endif
    if ( error.category() == bit7z::internalCategory() ) {
        /* Some bit7z internal errors have no faithful std::errc equivalent, so they are mapped to a specific
         * HRESULT here, before the generic std::errc-based mapping below (which would otherwise route them
         * through their default_error_condition, e.g. NoMatchingItems -> ENOENT -> ERROR_PATH_NOT_FOUND). */
        const auto bitError = static_cast< BitError >( error.value() );
        if ( bitError == BitError::NoMatchingItems || bitError == BitError::NoMatchingFile ) {
            /* A "no matching item/file" result is a lookup miss, not a missing path: ERROR_NOT_FOUND
             * ("Element not found.") is the closest Win32 code, without the misleading "path not found" semantics. */
            return HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
        }
    }
    // POSIX error code (generic_category) or BitError code (internal_category)
    if ( error == std::errc::bad_file_descriptor ) {
        return HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE );
    }
    if ( error == std::errc::invalid_argument ) {
        return E_INVALIDARG;
    }
    if ( error == std::errc::not_a_directory ) {
        return HRESULT_FROM_WIN32( ERROR_DIRECTORY );
    }
    if ( error == std::errc::function_not_supported ) {
        return E_NOTIMPL;
    }
    if ( error == std::errc::no_space_on_device ) {
        return HRESULT_FROM_WIN32( ERROR_DISK_FULL );
    }
    if ( error == std::errc::no_such_file_or_directory ) {
        /* ENOENT is ambiguous on Windows (both ERROR_FILE_NOT_FOUND and ERROR_PATH_NOT_FOUND map back to it),
         * so we pick the generic "the named item wasn't found" code. ERROR_PATH_NOT_FOUND specifically means an
         * intermediate directory is missing, which genuine filesystem failures already report via their own
         * Win32 error code (i.e., without reaching this fallback). */
        return HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
    }
    if ( error == std::errc::not_enough_memory ) {
        return E_OUTOFMEMORY;
    }
    if ( error == std::errc::not_supported ) {
        return E_NOINTERFACE;
    }
    if ( error == std::errc::file_exists ) {
        return HRESULT_FROM_WIN32( ERROR_FILE_EXISTS );
    }
    if ( error == std::errc::operation_canceled ) {
        return E_ABORT;
    }
#ifdef _WIN32
    if ( error == std::errc::permission_denied ) {
        return E_ACCESSDENIED;
    }
#endif
    return E_FAIL;
}

namespace {
auto isNotPosixCategory( const std::error_category& category ) -> bool {
#ifdef _MSC_VER
    if ( category == std::system_category() ) {
        return true;
    }
#elif defined( __MINGW32__ )
    if ( category == bit7z::win32Category() ) {
        return true;
    }
#endif
    return category == bit7z::hresultCategory() ||
           category == bit7z::internalCategory() ||
           category == bit7z::operationCategory();
}
} // namespace

auto BitException::posixCode() const noexcept -> int {
    const auto& error = code();
    if ( isNotPosixCategory( error.category() ) ) {
        return error.default_error_condition().value();
    }
    return error.value(); // On POSIX systems, std::system_category == std::generic_category
}

} // namespace bit7z
