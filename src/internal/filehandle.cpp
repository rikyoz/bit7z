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

#ifdef _WIN32
#include <io.h>
#include <share.h> // For _SH_DENYNO
#else
#include <unistd.h>

#if defined( __APPLE__ ) || defined( BSD ) || \
    defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
#define NO_LSEEK64
#endif
#endif

namespace bit7z {

namespace {

inline auto errno_as_hresult() -> HRESULT {
#ifdef _WIN32
    return HRESULT_FROM_WIN32( _doserrno );
#else
    return HRESULT_FROM_WIN32( static_cast< DWORD >( errno ) );
#endif
}

inline auto open_file( const fs::path& filePath, OpenFlags openFlags, PermissionFlag permissionFlag ) -> handle_t {
#ifdef _WIN32
    handle_t handle = -1;
    const errno_t result = _wsopen_s( &handle,
                                      filePath.c_str(),
                                      to_underlying( openFlags ),
                                      _SH_DENYNO,
                                      to_underlying( permissionFlag ) );
    if ( result != 0 ) {
        std::error_code error = make_hresult_code( HRESULT_FROM_WIN32( _doserrno ) );
#else
    // NOLINTNEXTLINE(*-vararg)
    handle_t handle = open( filePath.c_str(), to_underlying( openFlags ), to_underlying( permissionFlag ) );
    if ( handle < 0 ) {
        // Here we can directly use the POSIX error instead of converting it to HRESULT.
        std::error_code error = last_error_code();
#endif
        throw BitException( "Could not open the file", error );
    }
    return handle;
}

} // namespace

FileHandle::FileHandle( const fs::path& filePath, OpenFlags openFlags, PermissionFlag permissionFlag )
    : mHandle{ open_file( filePath, openFlags, permissionFlag ) } {}

FileHandle::~FileHandle() {
#ifdef _WIN32
    _close( mHandle );
#else
    close( mHandle );
#endif
}

auto FileHandle::seek( SeekOrigin origin, const Int64 distance, UInt64* newPosition ) const noexcept -> HRESULT {
#ifdef _WIN32
    const auto result = _lseeki64( mHandle, distance, static_cast< int >( origin ) );
#elif defined( NO_LSEEK64 )
    const auto result = lseek( mHandle, distance, static_cast< int >( origin ) );
#else
    const auto result = lseek64( mHandle, distance, static_cast< int >( origin ) );
#endif
    if ( result < 0 ) {
        return errno_as_hresult();
    }
    if ( newPosition != nullptr ) {
        *newPosition = static_cast< UInt64 >( result );
    }
    return S_OK;
}

OutputFile::OutputFile( const fs::path& filePath, const bool createAlways )
    : FileHandle{ filePath,
                  AccessFlag::WriteOnly | ( createAlways ? FileFlag::CreateAlways : FileFlag::CreateNew ),
                  PermissionFlag::ReadWrite } {}

auto OutputFile::write( const void* data, const UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT {
#ifdef _WIN32
    const auto result = _write( mHandle, data, size );
#else
    const auto result = ::write( mHandle, data, size );
#endif
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

// Guaranteeing that the input file open flags are calculated at compile time.
constexpr auto openInputFlags = AccessFlag::ReadOnly | FileFlag::Existing;

InputFile::InputFile( const fs::path& filePath )
    : FileHandle{ filePath, openInputFlags, PermissionFlag::Read } {}

auto InputFile::read( void* data, const UInt32 size, UInt32* processedSize ) const noexcept -> HRESULT {
#ifdef _WIN32
    const auto result = _read( mHandle, data, size );
#else
    const auto result = ::read( mHandle, data, size );
#endif
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
