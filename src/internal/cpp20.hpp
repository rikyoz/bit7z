/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CPP20_HPP
#define CPP20_HPP

#include <type_traits>

namespace bit7z {
namespace cpp20 {

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

} // namespace cpp20
} // namespace bit7z

#endif //CPP20_HPP
