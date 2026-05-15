/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CPP26_HPP
#define CPP26_HPP

#include "internal/cpp17.hpp"
#include "internal/cpp20.hpp"

#include <limits>
#include <type_traits>

namespace bit7z {
namespace cpp26 {

// TODO: Merge with BIT7Z_CPP14_CONSTEXPR macro in bitdefines.hpp
#if defined( __cpp_constexpr ) && __cpp_constexpr >= 201304L
#   define CPP14_CONSTEXPR constexpr
#else
#   define CONSTEXPR
#endif

// TODO: Use a variable template like are_both_integral_v; supported from GCC 5+, MSVC 2015 Update 2
template< typename T, typename U >
using are_both_integral = cpp17::bool_constant< std::is_integral< T >::value && std::is_integral< U >::value >;

template< typename To, typename From >
using needs_lower_clamp = cpp17::bool_constant< std::is_signed< From >::value &&
                                                ( std::is_unsigned< To >::value || sizeof( To ) < sizeof( From ) ) >;

template< typename To, typename From >
CPP14_CONSTEXPR auto lower_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                             needs_lower_clamp< To, From >::value, From > {
    constexpr auto kMinValue = std::numeric_limits< To >::min();
    return cpp20::cmp_less( value, kMinValue ) ? static_cast< From >( kMinValue ) : value;
}

template< typename To, typename From >
constexpr auto lower_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       !needs_lower_clamp< To, From >::value, From > {
    return value;
}

template< typename To, typename From >
using needs_upper_clamp = cpp17::bool_constant< sizeof( To ) < sizeof( From ) ||
                                                ( sizeof( To ) == sizeof( From ) &&
                                                std::is_unsigned< From >::value && std::is_signed< To >::value ) >;

template< typename To, typename From >
CPP14_CONSTEXPR auto upper_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                             needs_upper_clamp< To, From >::value, From > {
    constexpr auto kMaxValue = std::numeric_limits< To >::max();
    return cpp20::cmp_greater( value, kMaxValue ) ? static_cast< From >( kMaxValue ) : value;
}

template< typename To, typename From >
constexpr auto upper_clamp( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                       !needs_upper_clamp< To, From >::value, From > {
    return value;
}

template< typename To, typename From >
CPP14_CONSTEXPR auto saturating_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                                 needs_lower_clamp< To, From >::value &&
                                                                                 needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( upper_clamp< To, From >( lower_clamp< To, From >( value ) ) );
}

template< typename To, typename From >
CPP14_CONSTEXPR auto saturating_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                                 !needs_lower_clamp< To, From >::value &&
                                                                                 needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( upper_clamp< To, From >( value ) );
}

template< typename To, typename From >
CPP14_CONSTEXPR auto saturating_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                                 needs_lower_clamp< To, From >::value &&
                                                                                 !needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( lower_clamp< To, From >( value ) );
}

template< typename To, typename From >
constexpr auto saturating_cast( From value ) noexcept -> std::enable_if_t< are_both_integral< To, From >::value &&
                                                                           !needs_lower_clamp< To, From >::value &&
                                                                           !needs_upper_clamp< To, From >::value, To > {
    return static_cast< To >( value );
}

#undef CPP14_CONSTEXPR

} // namespace cpp26
} // namespace bit7z

#endif //CPP26_HPP
