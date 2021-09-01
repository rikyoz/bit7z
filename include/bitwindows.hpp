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
/* We don't have the "Windows.h" header on Unix systems, so in theory, we could use the "MyWindows.h" of p7zip.
 * However, some of bit7z's public API headers need some Win32 API structs like PROPVARIANT and GUID.
 * Hence, it would result in the leak of p7zip headers, making bit7z's clients dependent on them.
 * Also, (publicly) forward declaring them and then (internally) using the "MyWindows.h" is impossible:
 * the two different declarations would conflict, making the compilation fail.
 *
 * To avoid all these issues, we define the required Win32 API structs, constants, and type aliases,
 * with the same definitions in the MyWindows.h header.
 * We will use only this header and avoid including "MyWindows.h" or similar headers (e.g., StdAfx.h). */
#include <cerrno>
#include <cstdint>

// Avoiding accidentally including p7zip's MyWindows.h, so that its inclusion is not needed in client code!
#ifndef __MYWINDOWS_H
#define __MYWINDOWS_H
#endif

// Avoiding accidentally including 7-zip's MyWindows.h, so that its inclusion is not needed in client code!
#ifndef __MY_WINDOWS_H
#define __MY_WINDOWS_H
#endif

using std::size_t;

#define WINAPI

namespace bit7z {
    // Win32 type aliases
    using HMODULE = void*;
    using HRESULT = int;
    using OLECHAR = wchar_t;
    using BSTR = OLECHAR*;
    using VARIANT_BOOL = short;
    using VARTYPE = unsigned short;

    using WORD = unsigned short;
    using DWORD = unsigned int;

    using ULONG = unsigned int;
    using PROPID = ULONG;

    // Error codes constants can be useful for bit7z's clients on Unix (since they don't have Windows.h header)

    // Win32 HRESULT error codes
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

    // Win32 structs
    struct FILETIME {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    };

    struct LARGE_INTEGER {
        int64_t QuadPart;
    };

    struct ULARGE_INTEGER {
        uint64_t QuadPart;
    };

    struct PROPVARIANT {
        VARTYPE vt;
        WORD    wReserved1;
        WORD    wReserved2;
        WORD    wReserved3;
        union {
            char           cVal;
            unsigned char  bVal;
            short          iVal;
            unsigned short uiVal;
            int            lVal;
            unsigned int   ulVal;
            int            intVal;
            unsigned int   uintVal;
            LARGE_INTEGER  hVal;
            ULARGE_INTEGER uhVal;
            VARIANT_BOOL   boolVal;
            int            scode;
            FILETIME       filetime;
            BSTR           bstrVal;
        };
    };
}
#endif

#endif //BITWINDOWS_HPP
