// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitexception.hpp"

#include "../include/hresultcategory.hpp"

#include <utility> // for std::move

using std::string;

using namespace bit7z;

std::error_code bit7z::make_hresult_code( HRESULT res ) noexcept {
    return std::error_code{ static_cast< int >( res ), bit7z::hresult_category() };
}

BitException::BitException( const char* const message, std::error_code code, FailedFiles&& files )
    : system_error( code, message ), mFailedFiles( std::move( files ) ) { files.clear(); }

BitException::BitException( const char* const message, std::error_code code, const tstring& file )
    : system_error( code, message ), mFailedFiles( { std::make_pair<>( file, code ) } ) {}

BitException::BitException( const std::string& message, std::error_code code )
    : system_error( code, message.c_str() ) {}

const FailedFiles& BitException::failedFiles() const {
    return mFailedFiles;
}

BitException::native_code_type BitException::nativeCode() const {
    auto error = code();
#ifdef _WIN32
    if ( error.category() == bit7z::hresult_category() ) { // Already a HRESULT value
        return error.value();
    }
#ifdef _MSC_VER
    // Note: MinGW considers POSIX error codes in std::system_category, so this code is valid only for MSVC
    if ( error.category() == std::system_category() ) { // Win32 error code
        return HRESULT_FROM_WIN32( static_cast< DWORD >( error.value() ) );
    }
#endif
    // POSIX error code (generic_category)
    if ( error == std::errc::invalid_argument ) {
        return E_INVALIDARG;
    }
    if ( error == std::errc::not_a_directory ) {
        return HRESULT_FROM_WIN32( ERROR_DIRECTORY );
    }
    if ( error == std::errc::function_not_supported ) {
        return E_NOTIMPL;
    }
    if ( error == std::errc::no_such_file_or_directory ) {
        return HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
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
    if ( error == std::errc::permission_denied ) {
        return E_ACCESSDENIED;
    }
    return E_FAIL;
#else
    if ( error.category() == bit7z::hresult_category() ) {
        return error.default_error_condition().value();
    }
    return error.value(); // On POSIX systems, std::system_category == std::generic_category
#endif
}
