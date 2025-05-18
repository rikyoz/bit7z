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
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

#ifndef _WIN32
#include <sys/stat.h> // For S_IRUSR and S_IWUSR
#include <unistd.h>

#if defined( __APPLE__ ) || defined( BSD ) || \
    defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
#define NO_LSEEK64
#endif
#endif

namespace bit7z {

namespace {

BIT7Z_ALWAYS_INLINE
auto open_file( const native_string& filePath, OpenFlags openFlags ) -> handle_t {
#ifdef _WIN32
    const handle_t handle = ::CreateFileW( filePath.c_str(),
                                           to_underlying( openFlags.accessFlag ),
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           nullptr,
                                           to_underlying( openFlags.fileFlag ),
                                           FILE_ATTRIBUTE_NORMAL,
                                           nullptr );
    if ( handle == INVALID_HANDLE_VALUE ) { // NOLINT(*-pro-type-cstyle-cast, *-no-int-to-ptr)
#else
    // NOLINTNEXTLINE(*-vararg)
    const handle_t handle = open( filePath.c_str(), openFlags.value(), S_IRUSR | S_IWUSR );
    if ( handle < 0 ) {
#endif
        const std::error_code error = last_error_code();
        throw BitException( "Could not open the file", error, path_to_tstring( filePath ) );
    }
    return handle;
}

} // namespace

FileHandle::FileHandle( const native_string& filePath, OpenFlags openFlags )
    : mHandle{ open_file( filePath, openFlags ) } {}

FileHandle::~FileHandle() {
#ifdef _WIN32
    CloseHandle( mHandle );
#else
    close( mHandle );
#endif
}

auto FileHandle::seek( SeekOrigin origin,
                       const std::int64_t distance,
                       std::uint64_t& newPosition ) const noexcept -> HRESULT {
#ifdef _WIN32
    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = distance;

    LARGE_INTEGER finalPosition;
    const auto result = ::SetFilePointerEx( mHandle,
                                            distanceToMove,
                                            &finalPosition,
                                            static_cast< DWORD >( origin ) );
    if ( result == FALSE ) {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    newPosition = static_cast< std::uint64_t >( finalPosition.QuadPart );
#else
#if defined( NO_LSEEK64 )
    const auto result = lseek( mHandle, distance, static_cast< int >( origin ) );
#else
    const auto result = lseek64( mHandle, distance, static_cast< int >( origin ) );
#endif
    if ( result < 0 ) {
        return HRESULT_FROM_WIN32( static_cast< DWORD >( errno ) );
    }

    newPosition = static_cast< std::uint64_t >( result );
#endif
    return S_OK;
}

OutputFile::OutputFile( const native_string& filePath, FileFlag fileFlag )
    : FileHandle{ filePath, OpenFlags{ AccessFlag::WriteOnly, fileFlag } } {}

namespace {
BIT7Z_ALWAYS_INLINE
auto writeData( handle_t handle, const void* data, std::uint32_t size, DWORD& bytesWritten ) noexcept -> bool {
#if SIZE_MAX == 0xFFFFFFFF // 32-bit architecture
    if ( cmp_greater( size, std::numeric_limits< std::ptrdiff_t >::max() ) ) {
        size = static_cast< std::uint32_t >( std::numeric_limits< std::ptrdiff_t >::max() );
    }
#endif

#ifdef _WIN32
    return WriteFile( handle, data, size, &bytesWritten, nullptr ) != FALSE;
#else
    const auto result = ::write( handle, data, size );
    if ( result < 0 ) {
        return false;
    }
    bytesWritten = static_cast< DWORD >( result );
    return true;
#endif
}
} // namespace

auto OutputFile::write( const void* data, std::uint32_t size, std::uint32_t& processedSize ) const noexcept -> HRESULT {
    do {
        DWORD bytesWritten = 0;
        const auto writeSuccessful = writeData( mHandle, data, size, bytesWritten );
        if ( !writeSuccessful ) {
            return HRESULT_FROM_WIN32( GetLastError() );
        }
        if ( bytesWritten == 0 ) {
            break;
        }
        processedSize += bytesWritten;
        size -= bytesWritten;
        data = std::next( static_cast< const bit7z::byte_t* >( data ), clamp_cast< std::ptrdiff_t >( bytesWritten ) );
    } while ( size > 0 );
    return S_OK;
}

// Guaranteeing that the input file open flags are calculated at compile time.
static constexpr OpenFlags openInputFlags{ AccessFlag::ReadOnly, FileFlag::Existing };

InputFile::InputFile( const native_string& filePath )
    : FileHandle{ filePath, openInputFlags } {}

namespace {
BIT7Z_ALWAYS_INLINE
auto readData( handle_t handle, void* data, std::uint32_t size, DWORD& bytesRead ) noexcept -> bool {
#if SIZE_MAX == 0xFFFFFFFF // 32-bit architecture
    if ( cmp_greater( size, std::numeric_limits< std::ptrdiff_t >::max() ) ) {
        size = static_cast< std::uint32_t >( std::numeric_limits< std::ptrdiff_t >::max() );
    }
#endif

#ifdef _WIN32
    return ReadFile( handle, data, size, &bytesRead, nullptr ) != FALSE;
#else
    const auto result = ::read( handle, data, size );
    if ( result < 0 ) {
        return false;
    }
    bytesRead = static_cast< DWORD >( result );
    return true;
#endif
}
} // namespace

auto InputFile::read( void* data, std::uint32_t size, std::uint32_t& processedSize ) const noexcept -> HRESULT {
    do {
        DWORD bytesRead = 0;
        const auto readSuccessful = readData( mHandle, data, size, bytesRead );
        if ( !readSuccessful ) {
            return HRESULT_FROM_WIN32( GetLastError() );
        }
        if ( bytesRead == 0 ) {
            break;
        }
        processedSize += bytesRead;
        size -= bytesRead;
        data = std::next( static_cast< bit7z::byte_t* >( data ), clamp_cast< std::ptrdiff_t >( bytesRead ) );
    } while ( size > 0 );
    return S_OK;
}

} // namespace bit7z
