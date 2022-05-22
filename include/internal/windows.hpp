/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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

#ifndef WINDOWS_HPP
#define WINDOWS_HPP

#include "bitwindows.hpp"

#ifndef _WIN32

#include <Common/MyTypes.h>

/* Note: we must avoid including other headers of p7zip, like stdafx.h! */

/* Making sure constants and type aliases declared in bitwindows.hpp are usable by p7zip
 * as if they were not inside the bit7z namespace. */
using namespace bit7z;

/* Extra Win32 type aliases used by p7zip */
using LPCOLESTR = const OLECHAR*;
using LPCSTR = const char*;
using UINT = unsigned int;
using LONG = int;
using UInt16 = uint16_t;
using UInt32 = uint32_t;

// Win32 file attributes flags
#ifndef FILE_ATTRIBUTE_READONLY
constexpr auto FILE_ATTRIBUTE_READONLY       = 1;
constexpr auto FILE_ATTRIBUTE_DIRECTORY      = 16;
constexpr auto FILE_ATTRIBUTE_ARCHIVE        = 32;
constexpr auto FILE_ATTRIBUTE_NORMAL         = 128;
#endif

constexpr auto MAX_PATHNAME_LEN = 1024;

// Win32 VARIANT_BOOL constants
constexpr auto VARIANT_TRUE = static_cast< VARIANT_BOOL >( -1 );
constexpr auto VARIANT_FALSE = static_cast< VARIANT_BOOL >( 0 );

// Win32 macros needed by p7zip code
#define FAILED( Status ) ((HRESULT)(Status)<0)
#define HRESULT_FACILITY( hr )  (((hr) >> 16) & 0x1FFF)
#define HRESULT_CODE( hr )    ((hr) & 0xFFFF)

// Win32 APIs
inline DWORD WINAPI GetLastError() { return static_cast< DWORD >( errno ); }

constexpr auto FACILITY_ERRNO = 0x800;
constexpr auto FACILITY_WIN32 = 7;

#ifndef HRESULT_FROM_WIN32 // for p7zip (7-zip declares HRESULT_FROM_WIN32 in C/7zTypes.h so there's no need for this).
constexpr auto FACILITY_CODE = FACILITY_WIN32;

/* Note: p7zip uses FACILITY_WIN32, 7-zip version of HRESULT_FROM_WIN32 uses FACILITY_ERRNO. */
inline constexpr HRESULT HRESULT_FROM_WIN32( unsigned int x ) {
    auto result = static_cast< HRESULT >( x );
    return result > 0 ? static_cast< HRESULT >( ( x & 0x0000FFFF ) | ( FACILITY_WIN32 << 16 ) | 0x80000000 ) : result;
}

constexpr auto ERROR_NEGATIVE_SEEK = 0x100131;
#endif

// Win32 structs
struct WIN32_FILE_ATTRIBUTE_DATA {
    uint32_t dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
};

// Win32 enums
enum VARENUM {
    VT_EMPTY = 0,
    VT_NULL = 1,
    VT_I2 = 2,
    VT_I4 = 3,
    VT_R4 = 4,
    VT_R8 = 5,
    VT_CY = 6,
    VT_DATE = 7,
    VT_BSTR = 8,
    VT_DISPATCH = 9,
    VT_ERROR = 10,
    VT_BOOL = 11,
    VT_VARIANT = 12,
    VT_UNKNOWN = 13,
    VT_DECIMAL = 14,
    VT_I1 = 16,
    VT_UI1 = 17,
    VT_UI2 = 18,
    VT_UI4 = 19,
    VT_I8 = 20,
    VT_UI8 = 21,
    VT_INT = 22,
    VT_UINT = 23,
    VT_VOID = 24,
    VT_HRESULT = 25,
    VT_FILETIME = 64
};

enum STREAM_SEEK {
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR = 1,
    STREAM_SEEK_END = 2
};

#ifndef MY_EXTERN_C
#define MY_EXTERN_C extern "C"
#endif

// String-related Win32 API functions (implemented in windows.cpp)
BSTR SysAllocStringByteLen( LPCSTR psz, UINT len );

BSTR SysAllocStringLen( const OLECHAR*, UINT );

BSTR SysAllocString( const OLECHAR* str );

void SysFreeString( BSTR bstr );

UINT SysStringByteLen( BSTR bstr );

UINT SysStringLen( BSTR bstr );

#endif

#ifndef __HRESULT_FROM_WIN32
#define __HRESULT_FROM_WIN32 HRESULT_FROM_WIN32 // NOLINT(bugprone-reserved-identifier)
#endif

/* For when we cannot include IStream.h */
#ifndef HRESULT_WIN32_ERROR_NEGATIVE_SEEK
#ifdef MY__E_ERROR_NEGATIVE_SEEK // 7-zip on Unix
// On Unix 7-zip uses a different facility code since it uses HRESULT_FROM_WIN32 only for POSIX error codes.
constexpr auto FACILITY_CODE = FACILITY_ERRNO;
constexpr auto HRESULT_WIN32_ERROR_NEGATIVE_SEEK = MY__E_ERROR_NEGATIVE_SEEK;
#else //p7zip or 7-zip on Windows
constexpr auto FACILITY_CODE = FACILITY_WIN32;
constexpr auto HRESULT_WIN32_ERROR_NEGATIVE_SEEK = __HRESULT_FROM_WIN32( ERROR_NEGATIVE_SEEK );
#endif
#endif

#ifndef FILE_ATTRIBUTE_UNIX_EXTENSION
constexpr auto FILE_ATTRIBUTE_UNIX_EXTENSION = 0x8000; //as defined by p7zip
#endif

#endif //WINDOWS_HPP
