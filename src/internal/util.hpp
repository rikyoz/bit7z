/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include "internal/com.hpp"
#include "internal/cpp17.hpp"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace bit7z {

constexpr auto checkOverflow( std::int64_t position, std::int64_t offset ) noexcept -> bool {
    return ( ( offset > 0 ) && ( position > ( ( std::numeric_limits< std::int64_t >::max )() - offset ) ) ) ||
           ( ( offset < 0 ) && ( position < ( ( std::numeric_limits< std::int64_t >::min )() - offset ) ) );
}

inline auto seekToOffset( std::uint64_t& position, std::int64_t offset ) noexcept -> HRESULT {
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

template< typename T, typename I = T >
using is_com_type = cpp17::bool_constant< std::is_base_of< CMyUnknownImp, T >::value && std::is_base_of< I, T >::value >;

template< typename T, typename I = T, class... Args >
auto make_com( Args&&... args ) -> CMyComPtr< std::enable_if_t< is_com_type< T, I >::value, I > > {
    return CMyComPtr< I >( new T( std::forward< Args >( args )... ) ); //-V2511
}

} // namespace bit7z

#endif //UTIL_HPP
