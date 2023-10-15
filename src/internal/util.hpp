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

#include "internal/com.hpp"

namespace bit7z {

constexpr inline auto check_overflow( int64_t position, int64_t offset ) noexcept -> bool {
    return ( ( offset > 0 ) && ( position > ( ( std::numeric_limits< int64_t >::max )() - offset ) ) ) ||
           ( ( offset < 0 ) && ( position < ( ( std::numeric_limits< int64_t >::min )() - offset ) ) );
}

/* Safe integer comparison like in C++20 */
#ifdef __cpp_if_constexpr

template< class T, class U >
constexpr auto cmp_less( T first, U second ) noexcept -> bool {
    using UT = std::make_unsigned_t< T >;
    using UU = std::make_unsigned_t< U >;
    if constexpr ( std::is_signed< T >::value == std::is_signed< U >::value ) {
        return first < second;
    } else if constexpr ( std::is_signed< T >::value ) {
        return ( first < 0 ) || ( UT( first ) < second );
    } else {
        return ( second >= 0 ) && ( first < UU( second ) );
    }
}

#else // SFINAE implementation for C++14

template< class T, class U, std::enable_if_t< std::is_signed< T >::value == std::is_signed< U >::value, int > = 0 >
constexpr auto cmp_less( T first, U second ) noexcept -> bool {
    return first < second;
}

template< class T, class U, std::enable_if_t< std::is_signed< T >::value && !std::is_signed< U >::value, int > = 0 >
constexpr auto cmp_less( T first, U second ) noexcept -> bool {
    return ( first < 0 ) || ( std::make_unsigned_t< T >( first ) < second );
}

template< class T, class U, std::enable_if_t< !std::is_signed< T >::value && std::is_signed< U >::value, int > = 0 >
constexpr auto cmp_less( T first, U second ) noexcept -> bool {
    return ( second >= 0 ) && ( first < std::make_unsigned_t< U >( second ) );
}

#endif

template< class T, class U >
constexpr auto cmp_greater( T first, U second ) noexcept -> bool {
    return cmp_less( second, first ); // NOLINT(*-suspicious-call-argument)
}

template< class T, class U >
constexpr auto cmp_greater_equal( T first, U second ) noexcept -> bool {
    return !cmp_less( first, second );
}

template< bool B >
using bool_constant = std::integral_constant< bool, B >; // like C++17's std::bool_constant

template< typename T, typename I = T >
using is_com_type = bool_constant< std::is_base_of< CMyUnknownImp, T >::value && std::is_base_of< I, T >::value >;

template< typename T, typename I = T, class... Args >
inline auto make_com( Args&& ... args ) -> CMyComPtr< typename std::enable_if< is_com_type< T, I >::value, I >::type > {
    return CMyComPtr< I >( new T( std::forward< Args >( args )... ) ); //-V2511
}

}  // namespace bit7z

#endif //UTIL_HPP
