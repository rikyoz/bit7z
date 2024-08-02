// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filehandle.hpp"

#include "bitexception.hpp"
#include "internal/stringutil.hpp"

#include <fcntl.h>
#ifdef _WIN32
#include <io.h>

#if !defined( __MINGW32__ ) && !defined( __MINGW64__ )
constexpr auto close = &_close;
constexpr auto lseek64 = &_lseeki64;
constexpr auto write = &_write;
constexpr auto read = &_read;
#else
#include <share.h> // For _SH_DENYNO
#endif
#else
#include <unistd.h>
#endif

namespace bit7z {

inline auto errno_as_hresult() -> HRESULT {
#ifdef _WIN32
    return HRESULT_FROM_WIN32( _doserrno );
#else
    return HRESULT_FROM_WIN32( static_cast< DWORD >( errno ) );
#endif
}

FileHandle::FileHandle( const handle_t handle ) : mHandle{ handle } {}

FileHandle::~FileHandle() {
    close( mHandle );
}

auto FileHandle::seek( SeekOrigin origin, const Int64 distance, UInt64* newPosition ) const noexcept -> HRESULT {
    const auto result = lseek64( mHandle, distance, static_cast< int >( origin ) );
    if ( result < 0 ) {
        return errno_as_hresult();
    }
    if ( newPosition != nullptr ) {
        *newPosition = static_cast< UInt64 >( result );
    }
    return S_OK;
}

inline auto openOutputFile( const fs::path& filePath, const bool createAlways ) -> handle_t {
    std::error_code error;
    if ( !createAlways && fs::exists( filePath, error ) ) {
        if ( !error ) {
            // The call to fs::exists succeeded, but the filePath exists, and this is an error.
            error = std::make_error_code( std::errc::file_exists );
        }
        throw BitException( "Failed to create the output file", error, path_to_tstring( filePath ) );
    }

#ifdef _WIN32
    handle_t handle = -1;
    const errno_t result = _wsopen_s( &handle,
                                      filePath.c_str(),
                                      _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY,
                                      _SH_DENYNO,
                                      _S_IREAD | _S_IWRITE );
    if ( result != 0 ) {
        const auto errorValue = HRESULT_FROM_WIN32( _doserrno );
        throw BitException( "Could not open the input file", make_hresult_code( errorValue ) );
    }
#else
    handle_t handle = open( filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR ); // NOLINT(*-vararg)
    if ( handle < 0 ) {
        throw BitException( "Could not open the output file", last_error_code() );
    }
#endif
    return handle;
}

OutputFile::OutputFile( const fs::path& filePath, const bool createAlways )
    : FileHandle{ openOutputFile( filePath, createAlways ) } {}

auto OutputFile::write( const void* data, const UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT {
    const auto result = ::write( mHandle, data, size );
    if ( result < 0 ) {
        if ( processedSize != nullptr ) {
            *processedSize = 0;
        }
        return errno_as_hresult();
    }
    if ( processedSize != nullptr ) {
        *processedSize = static_cast< UInt32 >( result );
    }
    return S_OK;
}

inline auto openInputFile( const fs::path& filePath ) -> handle_t {
#ifdef _WIN32
    handle_t handle = -1;
    const errno_t result = _wsopen_s( &handle,
                                      filePath.c_str(),
                                      _O_RDONLY | _O_BINARY,
                                      _SH_DENYNO,
                                      _S_IREAD );
    if ( result != 0 ) {
        const auto error = HRESULT_FROM_WIN32( _doserrno );
        throw BitException( "Could not open the input file", make_hresult_code( error ) );
    }
#else
    handle_t handle = open( filePath.c_str(), O_RDONLY ); // NOLINT(*-vararg)
    if ( handle < 0 ) {
        throw BitException( "Could not open the input file", last_error_code() );
    }
#endif
    return handle;
}

InputFile::InputFile( const fs::path& filePath ) : FileHandle{ openInputFile( filePath ) } {}

auto InputFile::read( void* data, const UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT {
    const auto result = ::read( mHandle, data, size );
    if ( result < 0 ) {
        if ( processedSize != nullptr ) {
            *processedSize = 0;
        }
        return errno_as_hresult();
    }
    if ( processedSize != nullptr ) {
        *processedSize = static_cast< UInt32 >( result );
    }
    return S_OK;
}

} // namespace bit7z
