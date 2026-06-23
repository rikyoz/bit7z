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

#include "filehandle.hpp"

#include "bitexception.hpp"
#include "internal/cpp26.hpp"
#include "internal/fsutil.hpp"
#include "internal/util.hpp"

#ifdef _WIN32
#include "bitwindows.hpp" // For FILE_ATTRIBUTE_TAG_INFO and GetFileInformationByHandleEx.
#else
#include <sys/stat.h> // For S_IRUSR and S_IWUSR
#include <unistd.h>

#if defined( __APPLE__ ) || defined( BSD ) || \
    defined( __FreeBSD__ ) || defined( __NetBSD__ ) || defined( __OpenBSD__ ) || defined( __DragonFly__ )
#define NO_LSEEK64
#endif
#endif

namespace bit7z {

// FileHandle::seek casts a SeekOrigin straight to the platform's seek-origin constant, so the
// enumerators must match the values that lseek (Unix) and SetFilePointerEx (Windows) expect.
#ifdef _WIN32
static_assert( static_cast< DWORD >( SeekOrigin::Begin ) == FILE_BEGIN, "SeekOrigin::Begin must match FILE_BEGIN" );
static_assert( static_cast< DWORD >( SeekOrigin::CurrentPosition ) == FILE_CURRENT,
               "SeekOrigin::CurrentPosition must match FILE_CURRENT" );
static_assert( static_cast< DWORD >( SeekOrigin::End ) == FILE_END, "SeekOrigin::End must match FILE_END" );
#else
static_assert( static_cast< int >( SeekOrigin::Begin ) == SEEK_SET, "SeekOrigin::Begin must match SEEK_SET" );
static_assert( static_cast< int >( SeekOrigin::CurrentPosition ) == SEEK_CUR,
               "SeekOrigin::CurrentPosition must match SEEK_CUR" );
static_assert( static_cast< int >( SeekOrigin::End ) == SEEK_END, "SeekOrigin::End must match SEEK_END" );
#endif

namespace {

BIT7Z_ALWAYS_INLINE
auto openFile( const native_string& filePath, OpenFlags openFlags ) -> handle_t {
#ifdef _WIN32
    DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    if ( hasFlag( openFlags.extraFlag, ExtraFlag::NoFollow ) ) {
        // Open the reparse point itself so CreateFileW doesn't traverse it.
        flagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;
    }
    const handle_t handle = ::CreateFileW(
        filePath.c_str(),
        to_underlying( openFlags.accessFlag ),
        to_underlying( openFlags.shareFlag ),
        nullptr,
        to_underlying( openFlags.fileFlag ),
        flagsAndAttributes,
        nullptr
    );
    if ( handle == INVALID_HANDLE_VALUE ) { // NOLINT(*-pro-type-cstyle-cast, *-no-int-to-ptr)
#else
    // NOLINTNEXTLINE(*-vararg)
    const handle_t handle = open( filePath.c_str(), openFlags.value(), S_IRUSR | S_IWUSR );
    if ( handle < 0 ) {
#endif
        const std::error_code error = lastErrorCode();
        throw BitException( "Could not open the file", error, pathToTstring( filePath ) );
    }
#ifdef _WIN32
    // Emulate the strict POSIX O_NOFOLLOW semantics: fail only on symlinks, not on other
    // reparse-point kinds (junctions, mount points, etc.), matching S_IFLNK on Unix.
    if ( hasFlag( openFlags.extraFlag, ExtraFlag::NoFollow ) ) {
        FILE_ATTRIBUTE_TAG_INFO tagInfo{};
        const BOOL queryOk = ::GetFileInformationByHandleEx(
            handle,
            FileAttributeTagInfo,
            &tagInfo,
            sizeof( tagInfo )
        );
        if ( queryOk == FALSE ) {
            const std::error_code error = lastErrorCode();
            CloseHandle( handle );
            throw BitException( "Could not query file reparse tag", error, pathToTstring( filePath ) );
        }
        if ( tagInfo.ReparseTag == IO_REPARSE_TAG_SYMLINK ) {
            CloseHandle( handle );
            throw BitException(
                "Refusing to open a symbolic link",
                std::make_error_code( std::errc::too_many_symbolic_link_levels ),
                pathToTstring( filePath )
            );
        }
    }
#endif
    return handle;
}

} // namespace

FileHandle::FileHandle( const native_string& filePath, OpenFlags openFlags )
    : mHandle{ openFile( filePath, openFlags ) } {}

FileHandle::~FileHandle() {
#ifdef _WIN32
    CloseHandle( mHandle );
#else
    close( mHandle );
#endif
}

auto FileHandle::seek(
    SeekOrigin origin,
    const std::int64_t distance,
    std::uint64_t& newPosition
) const noexcept -> HRESULT {
#ifdef _WIN32
    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = distance;

    LARGE_INTEGER finalPosition;
    const auto result = ::SetFilePointerEx(
        mHandle,
        distanceToMove,
        &finalPosition,
        static_cast< DWORD >( origin )
    );
    if ( result == FALSE ) {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    newPosition = static_cast< std::uint64_t >( finalPosition.QuadPart );
#else
#ifdef NO_LSEEK64
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

namespace {
constexpr auto makeOpenFlags( AccessFlag access, FileFlag fileFlag, ExtraFlag extraFlag ) noexcept -> OpenFlags {
#ifdef _WIN32
    return { access, fileFlag, ShareFlag::Read, extraFlag };
#else
    return { access, fileFlag, extraFlag };
#endif
}
} // namespace

OutputFile::OutputFile( const native_string& filePath, FileFlag fileFlag, ExtraFlag extraFlag )
    : FileHandle{ filePath, makeOpenFlags( AccessFlag::WriteOnly, fileFlag, extraFlag ) } {}

namespace {
BIT7Z_ALWAYS_INLINE
auto writeData(
    handle_t handle,
    const void* data, // NOSONAR
    std::uint32_t size,
    DWORD& bytesWritten
) noexcept -> bool {
#if SIZE_MAX == 0xFFFFFFFF // 32-bit architecture
    if ( cpp20::cmp_greater( size, std::numeric_limits< std::ptrdiff_t >::max() ) ) {
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

auto OutputFile::write(
    const void* data, // NOSONAR
    std::uint32_t size,
    std::uint32_t& processedSize
) const noexcept -> HRESULT {
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
        data = std::next(
            static_cast< const bit7z::byte_t* >( data ),
            cpp26::saturating_cast< std::ptrdiff_t >( bytesWritten )
        );
    } while ( size > 0 );
    return S_OK;
}

auto OutputFile::resize( std::uint64_t newSize ) const noexcept -> bool {
#ifdef _WIN32
    std::uint64_t newPosition = 0;
    const auto result = seek( SeekOrigin::Begin, static_cast< std::int64_t >( newSize ), newPosition );
    if ( result != S_OK || newPosition != newSize ) {
        return false;
    }

    return ::SetEndOfFile( mHandle ) != FALSE;
#elif defined( NO_LSEEK64 )
    return ftruncate( mHandle, static_cast< off_t >( newSize ) ) == 0;
#else
    return ftruncate64( mHandle, static_cast< off64_t >( newSize ) ) == 0;
#endif
}

#ifdef _WIN32
auto OutputFile::setFileTime( FILETIME creation, FILETIME access, FILETIME modified ) const noexcept -> bool {
    return ::SetFileTime( mHandle, &creation, &access, &modified ) != FALSE;
}
#endif

namespace {
constexpr auto inputOpenFlags( BIT7Z_MAYBE_UNUSED bool storeOpenFiles ) noexcept -> OpenFlags {
#ifdef _WIN32
    return storeOpenFiles
        ? OpenFlags{ AccessFlag::ReadOnly, FileFlag::Existing, ShareFlag::ReadWrite, ExtraFlag::None }
        : OpenFlags{ AccessFlag::ReadOnly, FileFlag::Existing, ShareFlag::Read, ExtraFlag::None };
#else
    return { AccessFlag::ReadOnly, FileFlag::Existing, ExtraFlag::None };
#endif
}
} // namespace

InputFile::InputFile( const native_string& filePath, bool storeOpenFiles )
    : FileHandle{ filePath, inputOpenFlags( storeOpenFiles ) } {}

InputFile::InputFile( const native_string& filePath, ExtraFlag extraFlag )
    : FileHandle{ filePath, makeOpenFlags( AccessFlag::ReadOnly, FileFlag::Existing, extraFlag ) } {}

namespace {
BIT7Z_ALWAYS_INLINE
auto readData( handle_t handle, void* data, std::uint32_t size, DWORD& bytesRead ) noexcept -> bool { // NOSONAR
#if SIZE_MAX == 0xFFFFFFFF // 32-bit architecture
    if ( cpp20::cmp_greater( size, std::numeric_limits< std::ptrdiff_t >::max() ) ) {
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

auto InputFile::read(
    void* data, // NOSONAR
    std::uint32_t size,
    std::uint32_t& processedSize
) const noexcept -> HRESULT {
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
        data = std::next(
            static_cast< bit7z::byte_t* >( data ),
            cpp26::saturating_cast< std::ptrdiff_t >( bytesRead )
        );
    } while ( size > 0 );
    return S_OK;
}

} // namespace bit7z
