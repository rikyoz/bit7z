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

#ifdef __MINGW32__

#include "internal/win32category.hpp"

#include "internal/windows.hpp"

#include <string>
#include <system_error>

namespace bit7z {

auto Win32Category::name() const noexcept -> const char* {
    return "Win32";
}

auto Win32Category::message( int errorValue ) const -> std::string {
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
}

auto Win32Category::default_error_condition( int errorValue ) const noexcept -> std::error_condition {
    switch ( static_cast< DWORD >( errorValue ) ) {
        case ERROR_ACCESS_DENIED:
        case ERROR_INVALID_ACCESS:
        case ERROR_SHARING_VIOLATION:
            return std::make_error_condition( std::errc::permission_denied );
        case ERROR_DIRECTORY:
            return std::make_error_condition( std::errc::not_a_directory );
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:
            return std::make_error_condition( std::errc::not_enough_memory );
        case ERROR_OPERATION_ABORTED:
            return std::make_error_condition( std::errc::operation_canceled );
        case ERROR_OPEN_FAILED:
        case ERROR_SEEK:
        case ERROR_READ_FAULT:
        case ERROR_WRITE_FAULT:
            return std::make_error_condition( std::errc::io_error );
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:
        case ERROR_MOD_NOT_FOUND:
            return std::make_error_condition( std::errc::no_such_file_or_directory );
        case ERROR_ALREADY_EXISTS:
        case ERROR_FILE_EXISTS:
            return std::make_error_condition( std::errc::file_exists );
        case ERROR_DISK_FULL:
            return std::make_error_condition( std::errc::no_space_on_device );
        case ERROR_INVALID_DRIVE:
            return std::make_error_condition( std::errc::no_such_device );
        case ERROR_INVALID_FUNCTION:
            return std::make_error_condition( std::errc::function_not_supported );
        case ERROR_NOT_SUPPORTED:
            return std::make_error_condition( std::errc::not_supported );
        case ERROR_INVALID_HANDLE:
            return std::make_error_condition( std::errc::bad_file_descriptor );
        case ERROR_INVALID_PARAMETER:
            return std::make_error_condition( std::errc::invalid_argument );
        case ERROR_BAD_EXE_FORMAT:
            return std::make_error_condition( std::errc::executable_format_error );
        case ERROR_TOO_MANY_OPEN_FILES:
            return std::make_error_condition( std::errc::too_many_files_open );
        default:
            return error_category::default_error_condition( errorValue );
    }
}

auto win32Category() noexcept -> const std::error_category& {
    static const Win32Category instance{};
    return instance;
}

} // namespace bit7z

#endif // __MINGW32__
