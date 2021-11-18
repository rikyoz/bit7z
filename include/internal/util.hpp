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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <limits>
#include <type_traits>

#ifndef _WIN32
#include <string>
#include "internal/guiddef.hpp"
#include "internal/windows.hpp"
#endif

#include <Common/MyCom.h>

//p7zip defines IUnknown with a virtual destructor, while Windows' IUnknown has a non-virtual destructor
#ifdef _WIN32
#define MY_UNKNOWN_DESTRUCTOR(x) x
#else
#define MY_UNKNOWN_DESTRUCTOR(x) x override
#endif

// Some stream classes are non-final (e.g., CStdOutStream), so on Windows they must have a virtual destructor
#ifdef _WIN32
#define MY_UNKNOWN_VIRTUAL_DESTRUCTOR(x) virtual x
#else
#define MY_UNKNOWN_VIRTUAL_DESTRUCTOR(x) MY_UNKNOWN_DESTRUCTOR(x)
#endif

#ifndef _WIN32
#define COM_DECLSPEC_NOTHROW
#endif

#define MY_STDMETHOD(method, ...) HRESULT STDMETHODCALLTYPE method ( __VA_ARGS__ )
#define BIT7Z_STDMETHOD(method, ...) COM_DECLSPEC_NOTHROW MY_STDMETHOD(method, __VA_ARGS__) override
#define BIT7Z_STDMETHOD_NOEXCEPT(method, ...) MY_STDMETHOD(method, __VA_ARGS__) noexcept override

namespace bit7z {
#ifdef _WIN32
#define WIDEN( tstr ) tstr
#else
    using std::string;
    using std::wstring;

    string narrow( const wchar_t* wideString );
    wstring widen( const string& narrowString );

#define WIDEN( tstr ) bit7z::widen(tstr)
#endif

    constexpr inline bool check_overflow( int64_t position, int64_t offset ) noexcept {
        return ( offset > 0 && position > (std::numeric_limits< int64_t >::max)() - offset ) ||
               ( offset < 0 && position < (std::numeric_limits< int64_t >::min)() - offset );
    }

    template<typename T, typename I = T, class... Args>
    inline CMyComPtr<std::enable_if_t<std::is_base_of<CMyUnknownImp, T>::value && std::is_base_of<I, T>::value, I>>
    make_com( Args&&... args ) {
        return CMyComPtr<I>(new T(std::forward<Args>(args)...));
    }
}

#endif //UTIL_HPP
