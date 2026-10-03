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

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

template< std::size_t N >
auto asBytes( const char (&text)[ N ] ) -> buffer_t { // NOLINT(*-avoid-c-arrays)
    /* N includes the trailing null terminator, which we exclude from the buffer
     * so the bytes match the string literal's content. */
    const auto* const begin = reinterpret_cast< const byte_t* >( text ); // NOLINT(*-pro-type-reinterpret-cast)
    return { begin, begin + ( N - 1 ) };
}

} // namespace test
} // namespace bit7z

#endif //BUFFER_HPP
