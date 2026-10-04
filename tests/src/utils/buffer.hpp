/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <bit7z/bittypes.hpp>

#include <cstddef>
#include <string>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

template< std::size_t N >
auto asBytes( const char (&text)[ N ] ) -> buffer_t { // NOLINT(*-avoid-c-arrays)
    /* N includes the trailing null terminator, which we exclude from the buffer
     * so the bytes match the string literal's content. */
    const auto* const begin = reinterpret_cast< const byte_t* >( text ); // NOLINT(*-pro-type-reinterpret-cast)
    return { begin, begin + ( N - 1 ) };
}

/* Note: the buffer can't be built from the string's iterators, as a char is not convertible to the byte_t type
 * when it is a scoped enumeration (BIT7Z_USE_STD_BYTE). */
inline auto asBytes( const std::string& text ) -> buffer_t {
    const auto* const begin = reinterpret_cast< const byte_t* >( text.data() ); // NOLINT(*-pro-type-reinterpret-cast)
    return { begin, begin + text.size() };
}

} // namespace test
} // namespace bit7z

#endif //BUFFER_HPP
