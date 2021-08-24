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
#ifndef _WIN32

#ifndef GUIDDEF_HPP
#define GUIDDEF_HPP

#define GUID_DEFINED

#include <cstdint>

#include "bitwindows.hpp"

struct GUID {
    uint32_t      Data1;
    uint32_t      Data2;
    uint32_t      Data3;
    unsigned char Data4[8];
};

#define DEFINE_GUID( name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8 ) \
    MY_EXTERN_C const GUID name

using REFGUID = const GUID&;
using REFIID = REFGUID;

inline int operator==( REFGUID g1, REFGUID g2 ) {
    for ( int i = 0; i < ( int ) sizeof( g1 ); i++ ) {
        if ( ( ( unsigned char* ) &g1 )[ i ] != ( ( unsigned char* ) &g2 )[ i ] ) {
            return 0;
        }
    }
    return 1;
}

inline int operator!=( REFGUID g1, REFGUID g2 ) { return !( g1 == g2 ); }

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

#endif //GUIDDEF_HPP

#endif
