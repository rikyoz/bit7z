/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

#ifndef _WIN32
#include "internal/guiddef.hpp"
#include "internal/windows.hpp"
#endif

#include <Common/MyCom.h>

namespace bit7z {
#if defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
// On Windows, with native strings enabled, strings are already wide!
#   define WIDEN( tstr ) tstr
#else
#   define WIDEN( tstr ) bit7z::widen(tstr)
#endif

std::string narrow( const wchar_t* wideString, size_t size );

std::wstring widen( const std::string& narrowString );

constexpr inline bool check_overflow( int64_t position, int64_t offset ) noexcept {
    return ( offset > 0 && position > ( std::numeric_limits< int64_t >::max )() - offset ) ||
           ( offset < 0 && position < ( std::numeric_limits< int64_t >::min )() - offset );
}

template< typename T, typename I = T, class... Args >
inline CMyComPtr< std::enable_if_t< std::is_base_of< CMyUnknownImp, T >::value && std::is_base_of< I, T >::value, I>>
make_com( Args&& ... args ) {
    return CMyComPtr< I >( new T( std::forward< Args >( args )... ) );
}
}  // namespace bit7z

#endif //UTIL_HPP
