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

#ifndef BITWINDOWS_HPP
#define BITWINDOWS_HPP

#ifdef _WIN32
#include <Windows.h>
#else
#include <cerrno>
#include <cstdint>

// Avoiding accidentally including MyWindows.h or MyTypes.h, so that their inclusion is not needed in client code!
#define __MYWINDOWS_H
#define __COMMON_MY_TYPES_H

using std::size_t;

// Type aliases as defined by p7zip
using Int32 = int32_t;
using Int64 = long long int; //as defined by p7zip
using UInt32 = uint32_t;
using UInt64 = unsigned long long int; //as defined by p7zip
using Byte = unsigned char;

// Windows-specific type aliases
using HMODULE = void*;
using HRESULT = int32_t;
using PROPID = uint32_t;
using WORD = unsigned short;
using DWORD = uint32_t;
using OLECHAR = wchar_t;
using BSTR = OLECHAR*;
using ULONG = uint32_t;
using LONG = int32_t;
using UINT = unsigned int;
using VARTYPE = unsigned short;
using LPCOLESTR = const OLECHAR*;
using LPCSTR = const char*;
using VARIANT_BOOL = short;

// HRESULT error codes
constexpr auto S_OK                  = static_cast< HRESULT >( 0x00000000L );
constexpr auto S_FALSE               = static_cast< HRESULT >( 0x00000001L );
constexpr auto E_NOTIMPL             = static_cast< HRESULT >( 0x80004001L );
constexpr auto E_NOINTERFACE         = static_cast< HRESULT >( 0x80004002L );
constexpr auto E_ABORT               = static_cast< HRESULT >( 0x80004004L );
constexpr auto E_FAIL                = static_cast< HRESULT >( 0x80004005L );
constexpr auto STG_E_INVALIDFUNCTION = static_cast< HRESULT >( 0x80030001L );
constexpr auto E_OUTOFMEMORY         = static_cast< HRESULT >( 0x8007000EL );
constexpr auto E_INVALIDARG          = static_cast< HRESULT >( 0x80070057L );

// Win32 error codes (defined as POSIX error codes as in p7zip)
constexpr auto ERROR_ALREADY_EXISTS = EEXIST;
constexpr auto ERROR_DISK_FULL      = ENOSPC;
constexpr auto ERROR_FILE_EXISTS    = EEXIST;
constexpr auto ERROR_INVALID_HANDLE = EBADF;
constexpr auto ERROR_OPEN_FAILED    = EIO;
constexpr auto ERROR_PATH_NOT_FOUND = ENOENT;
constexpr auto ERROR_SEEK           = EIO;
constexpr auto ERROR_READ_FAULT     = EIO;
constexpr auto ERROR_WRITE_FAULT    = EIO;

// Win32 error codes (as defined by p7zip)
constexpr auto ERROR_NEGATIVE_SEEK = 0x100131;
constexpr auto ERROR_NO_MORE_FILES = 0x100018;
constexpr auto ERROR_DIRECTORY     = 267;

// Win32 file attributes flags
constexpr auto FILE_ATTRIBUTE_READONLY       = 1;
constexpr auto FILE_ATTRIBUTE_DIRECTORY      = 16;
constexpr auto FILE_ATTRIBUTE_ARCHIVE        = 32;
constexpr auto FILE_ATTRIBUTE_NORMAL         = 128;
constexpr auto FILE_ATTRIBUTE_UNIX_EXTENSION = 0x8000; //as defined by p7zip

constexpr auto MAX_PATHNAME_LEN = 1024;
constexpr auto FACILITY_WIN32 = 7;

// Win32 VARIANT_BOOL constants
constexpr auto VARIANT_TRUE  = static_cast< VARIANT_BOOL >( -1 );
constexpr auto VARIANT_FALSE = static_cast< VARIANT_BOOL >( 0 );

constexpr auto CHAR_PATH_SEPARATOR  = '/';
constexpr auto WCHAR_PATH_SEPARATOR = L'/';

// Win32 macros needed by p7zip code
#define COM_DECLSPEC_NOTHROW
#define FAILED( Status ) ((HRESULT)(Status)<0)
#define HRESULT_FACILITY( hr )  (((hr) >> 16) & 0x1fff)
#define HRESULT_CODE( hr )    ((hr) & 0xFFFF)
#define WINAPI

// Win32 APIs
inline DWORD WINAPI GetLastError() { return errno; }

inline constexpr HRESULT HRESULT_FROM_WIN32( unsigned int x ) {
    return static_cast< HRESULT >( x ) > 0 ?
           static_cast< HRESULT >( ( x & 0x0000FFFF ) | ( FACILITY_WIN32 << 16 ) | 0x80000000 ) : static_cast< HRESULT >( x );
}

// Win32 structs
struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct WIN32_FILE_ATTRIBUTE_DATA {
    uint32_t dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
};

struct LARGE_INTEGER {
    int64_t QuadPart;
};

struct ULARGE_INTEGER {
    uint64_t QuadPart;
};

struct PROPVARIANT {
    unsigned short vt;
    unsigned short wReserved1;
    unsigned short wReserved2;
    unsigned short wReserved3;
    union {
        char           cVal;
        unsigned char  bVal;
        short          iVal;
        unsigned short uiVal;
        int32_t        lVal;
        uint32_t       ulVal;
        int            intVal;
        unsigned int   uintVal;
        LARGE_INTEGER  hVal;
        ULARGE_INTEGER uhVal;
        short          boolVal;
        int32_t        scode;
        FILETIME       filetime;
        BSTR           bstrVal;
    };
};

struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

// Win32 enums
enum VARENUM {
    VT_EMPTY    = 0,
    VT_NULL     = 1,
    VT_I2       = 2,
    VT_I4       = 3,
    VT_R4       = 4,
    VT_R8       = 5,
    VT_CY       = 6,
    VT_DATE     = 7,
    VT_BSTR     = 8,
    VT_DISPATCH = 9,
    VT_ERROR    = 10,
    VT_BOOL     = 11,
    VT_VARIANT  = 12,
    VT_UNKNOWN  = 13,
    VT_DECIMAL  = 14,
    VT_I1       = 16,
    VT_UI1      = 17,
    VT_UI2      = 18,
    VT_UI4      = 19,
    VT_I8       = 20,
    VT_UI8      = 21,
    VT_INT      = 22,
    VT_UINT     = 23,
    VT_VOID     = 24,
    VT_HRESULT  = 25,
    VT_FILETIME = 64
};

enum STREAM_SEEK {
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR = 1,
    STREAM_SEEK_END = 2
};

// Externally defined Win32 APIs
#ifndef MY_EXTERN_C
#define MY_EXTERN_C extern "C"
#endif

// String-related functions defined in MyWindows.cpp (compiled with bit7z)
MY_EXTERN_C BSTR SysAllocStringByteLen( LPCSTR psz, UINT len );
MY_EXTERN_C BSTR SysAllocStringLen( const OLECHAR*, UINT );
MY_EXTERN_C BSTR SysAllocString( const OLECHAR* sz );
MY_EXTERN_C void SysFreeString( BSTR bstr );
MY_EXTERN_C UINT SysStringByteLen( BSTR bstr );
MY_EXTERN_C UINT SysStringLen( BSTR bstr );

// Date-related functions defined in MyWindows.cpp (compiled with bit7z)
MY_EXTERN_C LONG CompareFileTime( const FILETIME* ft1, const FILETIME* ft2 );
#endif

#endif //BITWINDOWS_HPP
