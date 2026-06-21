/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITINDICESVIEW_HPP
#define BITINDICESVIEW_HPP

#include "bitdefines.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace bit7z {
/**
 * @brief A dynamically-sized vector of archive item indices.
 */
using IndicesVector = std::vector< std::uint32_t >;

/**
 * @brief A fixed-size array of N archive item indices.
 *
 * @tparam N  the number of indices in the array.
 */
template< std::size_t N >
using IndicesArray = std::array< std::uint32_t, N >;

// NOLINTBEGIN(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)
/**
 * @brief A non-owning, read-only view over a contiguous sequence of archive item indices.
 *
 * BitIndicesView is essentially a C++11-compatible equivalent of `std::span< const std::uint32_t >`.
 * It implicitly converts from the most common index containers (a single index, an IndicesVector,
 * a C array, a std::array, or an initializer list), so that the extraction and testing methods can
 * accept any of them through a single parameter type.
 */
class BitIndicesView final {
    public:
        // BitIndicesView is basically a C++20's std::span< const std::uint32_t >.
        // Note: still not using C++14's traits style as bit7z's public API must be written in C++11.
        using element_type = const std::uint32_t; // T in std::span<T>.
        using value_type = std::remove_cv< element_type >::type;
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

        /**
         * @brief Constructs an empty BitIndicesView.
         */
        constexpr BitIndicesView() noexcept : mIndices{ nullptr }, mSize{ 0 } {}

        /**
         * @brief Constructs a BitIndicesView referencing a single index.
         *
         * @param index  the single index to be viewed.
         */
        /* implicit */ constexpr BitIndicesView( const_reference index ) noexcept // NOSONAR
            : mIndices{ &index }, mSize{ 1 } {}

        /**
         * @brief Constructs a BitIndicesView referencing the indices stored in the given vector.
         *
         * @note This constructor can't be constexpr until C++20 due to std::vector's methods.
         *
         * @param indices  the vector of indices to be viewed.
         */
        /* implicit */ BitIndicesView( const IndicesVector& indices ) noexcept // NOSONAR
            : BitIndicesView{ indices.data(), indices.size() } {}

        /**
         * @brief Constructs a BitIndicesView referencing the indices stored in the given C array.
         *
         * @tparam N       the number of indices in the array.
         * @param indices  the array of indices to be viewed.
         */
        template< std::size_t N >
        /* implicit */ constexpr BitIndicesView( element_type (&indices)[ N ] ) noexcept // NOSONAR
            : BitIndicesView{ static_cast< const_pointer >( indices ), N } {}

        /**
         * @brief Constructs a BitIndicesView referencing the indices stored in the given std::array.
         *
         * @tparam U       the element type of the array (must be convertible to the view's element type).
         * @tparam N       the number of indices in the array.
         * @param indices  the array of indices to be viewed.
         */
        template<
            typename U,
            std::size_t N,
            typename = typename std::enable_if< std::is_convertible< U(*)[ ], element_type(*)[ ] >::value >::type
        >
        /* implicit */ constexpr BitIndicesView( const std::array< U, N >& indices ) noexcept // NOSONAR
            : BitIndicesView{ indices.data(), indices.size() } {}

        /**
         * @brief Constructs a BitIndicesView referencing the indices in the given initializer list.
         *
         * @param indices  the initializer list of indices to be viewed.
         */
        BitIndicesView( std::initializer_list< value_type > indices ) noexcept
            : BitIndicesView{ indices.begin(), indices.size() } {}

        /**
         * @return a pointer to the beginning of the viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto data() const noexcept -> const_pointer {
            return mIndices;
        }

        /**
         * @return the number of viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto size() const noexcept -> size_type {
            return mSize;
        }

        /**
         * @return an iterator to the beginning of the viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto begin() const noexcept -> iterator {
            return cbegin();
        }

        /**
         * @return an iterator to the end of the viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto end() const noexcept -> iterator {
            return cend();
        }

        /**
         * @return a const iterator to the beginning of the viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto cbegin() const noexcept -> const_iterator {
            return mIndices;
        }

        /**
         * @return a const iterator to the end of the viewed indices.
         */
        BIT7Z_NODISCARD
        constexpr auto cend() const noexcept -> const_iterator {
            return mIndices + mSize;
        }

        /**
         * @return true if and only if the view does not reference any index.
         */
        BIT7Z_NODISCARD
        constexpr auto empty() const noexcept -> bool {
            return mIndices == nullptr;
        }

    private:
        const_pointer mIndices;
        size_type mSize;

        constexpr BitIndicesView( const_pointer indices, std::size_t size ) noexcept
            : mIndices{ size == 0 ? nullptr : indices },
              mSize{ static_cast< size_type >( size ) } {}
};
// NOLINTEND(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)
} // namespace bit7z

#endif //BITINDICESVIEW_HPP
