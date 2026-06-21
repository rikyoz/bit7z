/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SEEKORIGIN_HPP
#define SEEKORIGIN_HPP

#include "internal/windows.hpp" // For STREAM_SEEK_*, HRESULT, and STG_E_INVALIDFUNCTION

#include <cstdint>

namespace bit7z {

/**
 * @brief Enumeration defining the origin of a seek operation.
 *
 * The enumerators match the COM STREAM_SEEK_* constants, so a SeekOrigin can be
 * passed wherever 7-Zip expects a seek-origin value, and vice versa.
 */
enum struct SeekOrigin : std::uint8_t {
    Begin           = STREAM_SEEK_SET, ///< Seek relative to the beginning of the stream.
    CurrentPosition = STREAM_SEEK_CUR, ///< Seek relative to the current position in the stream.
    End             = STREAM_SEEK_END  ///< Seek relative to the end of the stream.
};

/**
 * @brief Validates a raw COM seek-origin value and converts it to a SeekOrigin.
 *
 * Validation happens on the raw value before any narrowing, so out-of-range
 * origins are rejected rather than silently aliased onto a valid enumerator.
 *
 * @param rawOrigin the seek-origin value received from the 7-Zip COM interface.
 * @param origin    set to the corresponding SeekOrigin on success.
 *
 * @return S_OK on success, or STG_E_INVALIDFUNCTION if the value is not a valid origin.
 */
inline auto toSeekOrigin( std::uint32_t rawOrigin, SeekOrigin& origin ) noexcept -> HRESULT {
    // SeekOrigin's enumerators are defined as the COM STREAM_SEEK_* constants, which the COM ABI fixes
    // to the contiguous range [0, 2]. So once the raw value is range-checked, it maps onto a SeekOrigin
    // by a plain cast. Unlike a cast to std::ios_base::seekdir (whose values are implementation-defined),
    // this relies only on our own enum and the fixed COM constants, never on a library's chosen values.
    static_assert( STREAM_SEEK_SET == 0 && STREAM_SEEK_CUR == 1 && STREAM_SEEK_END == 2,
                   "toSeekOrigin assumes the COM seek origins are contiguous starting from zero" );

    // The range check is on the raw value, before the narrowing cast, so an out-of-range origin is
    // rejected rather than silently aliased onto a valid enumerator (e.g., 258 would narrow to End).
    if ( rawOrigin > static_cast< std::uint32_t >( STREAM_SEEK_END ) ) {
        return STG_E_INVALIDFUNCTION;
    }
    origin = static_cast< SeekOrigin >( rawOrigin );
    return S_OK;
}

} // namespace bit7z

#endif //SEEKORIGIN_HPP
