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

#ifndef GUIDDEF_HPP
#define GUIDDEF_HPP

#include "bitformat.hpp"

#ifdef _WIN32
#include <Windows.h>
#else

#ifndef GUID_DEFINED
#define GUID_DEFINED
#endif

#include <cstdint>

#include "internal/windows.hpp"

struct GUID {
    UInt32        Data1;
    UInt16        Data2;
    UInt16        Data3;
    unsigned char Data4[8];
};

#define DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
    extern "C" const GUID name

using REFGUID = const GUID&;
using REFIID = REFGUID;

inline bool operator==( REFGUID g1, REFGUID g2 ) {
    return std::memcmp( &g1, &g2, sizeof( GUID ) ) == 0;
}

inline bool operator!=( REFGUID g1, REFGUID g2 ) { return !( g1 == g2 ); }

#define STDMETHODCALLTYPE
#define STDMETHOD_( t, f ) virtual t STDMETHODCALLTYPE f
#define STDMETHOD( f ) STDMETHOD_(HRESULT, f)
#define STDMETHODIMP_( type ) type STDMETHODCALLTYPE
#define STDMETHODIMP STDMETHODIMP_(HRESULT)

#define PURE = 0

struct IUnknown {
    STDMETHOD ( QueryInterface )( REFIID iid, void** outObject ) PURE;

    STDMETHOD_( ULONG, AddRef )() PURE;

    STDMETHOD_( ULONG, Release )() PURE;

    virtual ~IUnknown() = default;
};

#endif

namespace bit7z {
    /**
     * @return the GUID that identifies the file format in the 7z SDK.
     */
    inline GUID formatGUID( const BitInFormat& format ) {
        return { 0x23170F69, 0x40C1, 0x278A, { 0x10, 0x00, 0x00, 0x01, 0x10, format.value(), 0x00, 0x00 } }; // NOLINT
    }
}

#endif //GUIDDEF_HPP
