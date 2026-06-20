/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OPENERROR_HPP
#define OPENERROR_HPP

#include "bitdefines.hpp"

#include <cstdint>
#include <system_error>
#include <type_traits>

namespace bit7z {

// Values match 7-Zip's kpv_ErrorFlags_* constants from PropID.h.
enum struct OpenError : std::uint32_t { // NOLINT(*-enum-size)
    IsNotArc              = 1u << 0u,
    HeadersError          = 1u << 1u,
    EncryptedHeadersError = 1u << 2u,
    UnavailableStart      = 1u << 3u,
    UnconfirmedStart      = 1u << 4u,
    UnexpectedEnd         = 1u << 5u,
    DataAfterEnd          = 1u << 6u,
    UnsupportedMethod     = 1u << 7u,
    UnsupportedFeature    = 1u << 8u,
    DataError             = 1u << 9u,
    CRCError              = 1u << 10u
};

auto make_error_code( OpenError error ) noexcept -> std::error_code;

BIT7Z_ALWAYS_INLINE
auto make_open_error_code( std::uint32_t errorFlags ) noexcept -> std::error_code {
    // TODO: Handle multiple errors.
    // For now, we just take the first error flag set by 7-Zip and report only that.
    // To do this, we isolate the lowest set bit in the given error flags using bit manipulation.
    errorFlags &= ~errorFlags + 1;
    return make_error_code( static_cast< OpenError >( errorFlags ) );
}

} // namespace bit7z

namespace std {
template<>
struct BIT7Z_MAYBE_UNUSED is_error_code_enum< bit7z::OpenError > : true_type {};
} // namespace std

#endif // OPENERROR_HPP
