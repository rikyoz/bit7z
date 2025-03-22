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

#include "internal/com.hpp"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace bit7z {

constexpr auto check_overflow( std::int64_t position, std::int64_t offset ) noexcept -> bool {
    return ( ( offset > 0 ) && ( position > ( ( std::numeric_limits< std::int64_t >::max )() - offset ) ) ) ||
           ( ( offset < 0 ) && ( position < ( ( std::numeric_limits< std::int64_t >::min )() - offset ) ) );
}

inline auto seek_to_offset( std::uint64_t& position, std::int64_t offset ) noexcept -> HRESULT {
    // Checking if adding the offset would result in the unsigned wrap around of the current position.
    if ( offset < 0 ) {
        if ( offset == std::numeric_limits< std::int64_t >::min() ) {
            return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
        }
        const auto positiveOffset = static_cast< std::uint64_t >( -offset );
        if ( position < positiveOffset ) {
            return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
        }
        position -= positiveOffset;
    } else if ( offset > 0 ) {
        const auto positiveOffset = static_cast< std::uint64_t >( offset );
        const std::uint64_t seekPosition = position + positiveOffset;
        if ( seekPosition < position ) {
            return E_INVALIDARG;
        }
        position = seekPosition;
    } else {
        // No action needed
    }
    return S_OK;
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

#if defined( __cpp_constexpr ) && __cpp_constexpr >= 201304L
#   define CONSTEXPR constexpr
#else
#   define CONSTEXPR
#endif

template< bool B >
using bool_constant = std::integral_constant< bool, B >; // like C++17's std::bool_constant

// TODO: Use a variable template like are_both_integral_v; supported from GCC 5+, MSVC 2015 Update 2
template< typename T, typename U >
using are_both_integral = bool_constant< std::is_integral< T >::value && std::is_integral< U >::value >;

template< typename To, typename From >
using needs_lower_clamp = bool_constant< std::is_signed< From >::value &&
                                         ( std::is_unsigned< To >::value || sizeof( To ) < sizeof( From ) ) >;

template< typename To, typename From >
CONSTEXPR auto lower_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       needs_lower_clamp< To, From >::value, From > {
    constexpr auto kMinValue = std::numeric_limits< To >::min();
    return cmp_less( value, kMinValue ) ? static_cast< From >( kMinValue ) : value;
}

template< typename To, typename From >
constexpr auto lower_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       !needs_lower_clamp< To, From >::value, From > {
    return value;
}

template< typename To, typename From >
using needs_upper_clamp = bool_constant< sizeof( To ) < sizeof( From ) ||
                                         ( sizeof( To ) == sizeof( From ) &&
                                           std::is_unsigned< From >::value && std::is_signed< To >::value ) >;

template< typename To, typename From >
CONSTEXPR auto upper_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       needs_upper_clamp< To, From >::value, From > {
    constexpr auto kMaxValue = std::numeric_limits< To >::max();
    return cmp_greater( value, kMaxValue ) ? static_cast< From >( kMaxValue ) : value;
}

template< typename To, typename From >
constexpr auto upper_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       !needs_upper_clamp< To, From >::value, From > {
    return value;
}

template< typename To, typename From >
CONSTEXPR auto clamp_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                      needs_lower_clamp< To, From >::value &&
                                                                      needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( upper_clamp< To, From >( lower_clamp< To, From >( value ) ) );
}

template< typename To, typename From >
CONSTEXPR auto clamp_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                      !needs_lower_clamp< To, From >::value &&
                                                                      needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( upper_clamp< To, From >( value ) );
}

template< typename To, typename From >
CONSTEXPR auto clamp_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                      needs_lower_clamp< To, From >::value &&
                                                                      !needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( lower_clamp< To, From >( value ) );
}

template< typename To, typename From >
constexpr auto clamp_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                      !needs_lower_clamp< To, From >::value &&
                                                                      !needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( value );
}

template< typename T, typename I = T >
using is_com_type = bool_constant< std::is_base_of< CMyUnknownImp, T >::value && std::is_base_of< I, T >::value >;

template< typename T, typename I = T, class... Args >
auto make_com( Args&& ... args ) -> CMyComPtr< std::enable_if_t< is_com_type< T, I >::value, I > > {
    return CMyComPtr< I >( new T( std::forward< Args >( args )... ) ); //-V2511
}

}  // namespace bit7z

#endif //UTIL_HPP
