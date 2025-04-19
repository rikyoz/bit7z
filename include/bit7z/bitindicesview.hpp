/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
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
using IndicesVector = std::vector< std::uint32_t >;

template<std::size_t N>
using IndicesArray = std::array< std::uint32_t, N >;

// NOLINTBEGIN(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)
class BitIndicesView final {
        const std::uint32_t* mIndices;
        std::uint32_t mSize;

        constexpr BitIndicesView( const std::uint32_t* indices, std::size_t size ) noexcept
            : mIndices{ size == 0 ? nullptr : indices },
              mSize{ static_cast< std::uint32_t >( size ) } {}

    public:
        class ConstIterator final {
                const std::uint32_t* mPointer;

                /* implicit */ constexpr ConstIterator( const std::uint32_t* ptr ) : mPointer{ ptr } {}

                friend class BitIndicesView;

            public:
                using iterator_category = std::random_access_iterator_tag;
                using difference_type   = std::ptrdiff_t;
                using value_type        = std::uint32_t;
                using pointer           = const value_type*;
                using reference         = const value_type&;

                constexpr auto operator*() const noexcept -> reference {
                    return *mPointer;
                }

                constexpr auto operator->() const noexcept -> pointer {
                    return mPointer;
                }

                constexpr auto operator[]( difference_type rhs ) const -> reference {
                    return mPointer[ rhs ];
                }

                auto operator++() noexcept -> ConstIterator& {
                    ++mPointer;
                    return *this;
                }

                auto operator--() -> ConstIterator& {
                    --mPointer;
                    return *this;
                }

                auto operator++( int ) noexcept -> ConstIterator {
                    const ConstIterator tmp = *this;
                    ++mPointer;
                    return tmp;
                }

                auto operator--( int ) -> ConstIterator {
                    const ConstIterator tmp = *this;
                    --mPointer;
                    return tmp;
                }

                auto operator-( const ConstIterator& rhs ) const -> difference_type {
                    return mPointer - rhs.mPointer;
                }

                auto operator+( difference_type rhs ) const -> ConstIterator {
                    return mPointer + rhs;
                }

                auto operator-( difference_type rhs ) const -> ConstIterator {
                    return mPointer - rhs;
                }

                friend auto operator+( difference_type lhs, ConstIterator rhs ) -> ConstIterator {
                    return lhs + rhs.mPointer;
                }

                friend constexpr auto operator==( ConstIterator lhs, ConstIterator rhs ) noexcept -> bool {
                    return lhs.mPointer == rhs.mPointer;
                }

                friend constexpr auto operator!=( ConstIterator lhs, ConstIterator rhs ) noexcept -> bool {
                    return lhs.mPointer != rhs.mPointer;
                }

                friend constexpr auto operator>(ConstIterator lhs, ConstIterator rhs) -> bool {
                    return lhs.mPointer > rhs.mPointer;
                }

                friend constexpr auto operator<(ConstIterator lhs, ConstIterator rhs) -> bool {
                    return lhs.mPointer < rhs.mPointer;
                }

                friend constexpr auto operator>=(ConstIterator lhs, ConstIterator rhs) -> bool {
                    return lhs.mPointer >= rhs.mPointer;
                }

                friend constexpr auto operator<=(ConstIterator lhs, ConstIterator rhs) -> bool {
                    return lhs.mPointer <= rhs.mPointer;
                }
        };

        using value_type = std::uint32_t;
        using size_type = decltype( mSize );
        using iterator = ConstIterator;
        using const_iterator = ConstIterator;
        using pointer = decltype( mIndices );
        using const_pointer = decltype( mIndices );

        constexpr BitIndicesView() noexcept : mIndices{ nullptr }, mSize{ 0 } {}

        /* implicit */ constexpr BitIndicesView( const std::uint32_t& index ) noexcept : mIndices{ &index }, mSize{ 1 } {}

        // Note: this constructor can't be constexpr until C++20 due to std::vector's methods.
        /* implicit */ BitIndicesView( const IndicesVector& indices ) noexcept
            : BitIndicesView{ indices.data(), indices.size() } {}

        template< std::size_t N >
        /* implicit */ constexpr BitIndicesView( const std::uint32_t (&indices)[N] ) noexcept
            : BitIndicesView{ static_cast< const std::uint32_t* >( indices ), N } {}

        template< std::size_t N >
        /* implicit */ constexpr BitIndicesView( const std::array< std::uint32_t, N >& indices ) noexcept
            : BitIndicesView{ indices.data(), indices.size() } {}

        BitIndicesView( std::initializer_list< std::uint32_t > indices ) noexcept
            : BitIndicesView{ indices.begin(), indices.size() } {}

        BIT7Z_NODISCARD
        constexpr auto data() const noexcept -> const_pointer {
            return mIndices;
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
            return mIndices;
        }

        BIT7Z_NODISCARD
        constexpr auto cend() const noexcept -> const_iterator {
            return mIndices + mSize;
        }

        BIT7Z_NODISCARD
        constexpr auto empty() const noexcept -> bool {
            return mIndices == nullptr;
        }
};
// NOLINTEND(*-explicit-conversions, *-avoid-c-arrays, *-pro-bounds-pointer-arithmetic)
} // namespace bit7z

#endif //BITINDICESVIEW_HPP
