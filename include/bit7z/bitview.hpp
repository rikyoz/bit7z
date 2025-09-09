/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITVIEW_HPP
#define BITVIEW_HPP

#include "bitdefines.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace bit7z {

// NOLINTBEGIN(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)
template <typename T>
class BitView {
public:
    using element_type = T; // T in std::span<T>.
    using value_type = typename std::remove_cv< element_type >::type;
    using size_type = std::uint32_t; // 7-Zip uses 32-bits for the size of the indices array.
    using difference_type = std::ptrdiff_t;
    using pointer = element_type*;
    using const_pointer = const element_type*;
    using reference = element_type&;
    using const_reference = const element_type&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator< iterator >;
    using const_reverse_iterator = std::reverse_iterator< const_iterator >;

    constexpr BitView() noexcept : mStart{ nullptr }, mSize{ 0 } {}

    /* implicit */ constexpr BitView( const_reference index ) noexcept
        : mStart{ &index }, mSize{ 1 } {}

    // Note: this constructor can't be constexpr until C++20 due to std::vector's methods.
    /* implicit */ BitView( const std::vector< T >& indices ) noexcept
        : BitView{ indices.data(), indices.size() } {}

    template< std::size_t N >
    /* implicit */ constexpr BitView( element_type (&indices)[ N ] ) noexcept
        : BitView{ static_cast< const_pointer >( indices ), N } {}

    template< typename U,
              std::size_t N,
              typename = typename std::enable_if< std::is_convertible<
                  U(*)[ ], element_type(*)[ ] >::value >::type >
    /* implicit */ constexpr BitView( const std::array< U, N >& indices ) noexcept
        : BitView{ indices.data(), indices.size() } {}

    BitView( std::initializer_list< value_type > indices ) noexcept
        : BitView{ indices.begin(), indices.size() } {}

    BIT7Z_NODISCARD
    constexpr auto data() const noexcept -> const_pointer {
        return mStart;
    }

    BIT7Z_NODISCARD
    constexpr auto size() const noexcept -> size_type {
        return mSize;
    }

    BIT7Z_NODISCARD
    constexpr auto begin() const noexcept -> iterator {
        return cbegin();
    }

    BIT7Z_NODISCARD
    constexpr auto end() const noexcept -> iterator {
        return cend();
    }

    BIT7Z_NODISCARD
    constexpr auto cbegin() const noexcept -> const_iterator {
        return mStart;
    }

    BIT7Z_NODISCARD
    constexpr auto cend() const noexcept -> const_iterator {
        return mStart + mSize;
    }

    BIT7Z_NODISCARD
    constexpr auto empty() const noexcept -> bool {
        return mStart == nullptr;
    }

private:
    const_pointer mStart;
    size_type mSize;

    constexpr BitView( const_pointer bufferStart, std::size_t size ) noexcept
        : mStart{ size == 0 ? nullptr : bufferStart },
          mSize{ static_cast< size_type >( size ) } {}
};
// NOLINTEND(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)

} // namespace bit7z

#endif //BITVIEW_HPP
