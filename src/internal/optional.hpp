/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include <utility>
#include <type_traits>

#include "biterror.hpp"
#include "bitexception.hpp"
#include "bittypes.hpp"

namespace bit7z {
template< typename T >
using is_movable = std::integral_constant< bool, std::is_move_constructible<T>::value &&
                                                 std::is_nothrow_move_constructible< T >::value >;

struct nullopt_t {
    constexpr explicit nullopt_t( int ) noexcept {}
};

static constexpr nullopt_t nullopt{0};

constexpr struct in_place_t{} in_place{};

template< typename T, bool = std::is_trivially_destructible< T >::value >
union OptionalStorage { // NOLINT(*-special-member-functions)
    byte_t mEmptyByte;
    T mValue;

    constexpr OptionalStorage() noexcept : mEmptyByte{} {}

    template < typename... Args >
    explicit constexpr OptionalStorage( Args&&... args ) : mValue( std::forward< Args >( args )... ) {}

    // Using the default destructor to make OptionalStorage trivially-destructible (as the type T).
    ~OptionalStorage() noexcept = default;
};

template< typename T >
union OptionalStorage<T, false> { // NOLINT(*-special-member-functions)
    byte_t mEmptyByte;
    T mValue;

    constexpr OptionalStorage() noexcept : mEmptyByte{} {}

    template < typename... Args >
    explicit constexpr OptionalStorage( Args&&... args ) : mValue( std::forward< Args >( args )... ) {}

    // Non-trivially-destructible types need the destructor body; OptionalStorage will not be trivially-destructible.
    // Note: we do nothing in the destructor, as the destruction of an eventual value in the storage is performed
    // by OptionalBase's destructor (which performs it based on whether the Optional is engaged or not).
    ~OptionalStorage() noexcept {}
};

template< typename T >
class Optional;

// CRTP base for conditional destruction
template < typename T, bool = std::is_trivially_destructible< T >::value >
struct OptionalBase {
    // Trivially destructible: default destructor
};

template < typename T >
struct OptionalBase< T, false > { // NOLINT(*-special-member-functions)
    // Non-trivial destructor: destroy stored T if engaged
    ~OptionalBase() noexcept {
        auto& self = static_cast< Optional< T >& >( *this );
        if ( self.mEngaged ) {
            self.stored_value().~T();
        }
    }
};

// (Incomplete/stripped-down) implementation of std::optional for older compilers.
// It will be replaced with std::optional once the library will be fully written in C++17.
template< typename T >
class Optional final : OptionalBase< T > {
    public:
        using value_type = T;
        using base_type = OptionalBase< T >;

        friend struct OptionalBase<T>;

        constexpr Optional() noexcept : mEngaged{ false } {}

        //NOLINTNEXTLINE(*-explicit-conversions)
        /* implicit */ constexpr Optional( nullopt_t /*unused*/ ) noexcept : mEngaged{ false } {}

        /* implicit */ Optional( const Optional& other ) : mEngaged{ other.mEngaged } {
            if ( other.mEngaged ) {
                new( data() ) T( *other );
            }
        }

        /* implicit */ Optional( Optional&& other ) noexcept( std::is_nothrow_move_constructible<T>::value )
            : mEngaged{ other.mEngaged } {
            if ( other.mEngaged ) {
                new( data() ) T( std::move( *other ) );
            }
        }

        //NOLINTNEXTLINE(*-explicit-conversions)
        /* implicit */ constexpr Optional( const T& value ) : mEngaged{ true }, mStorage{ value } {}

        //NOLINTNEXTLINE(*-explicit-conversions)
        /* implicit */ constexpr Optional( T&& value ) : mEngaged{ true }, mStorage{ std::move( value ) } {}

        template< class... Args >
        constexpr explicit Optional( in_place_t /*unused*/, Args&&... args )
            : mEngaged{ true }, mStorage{ std::forward< Args >( args )... } {}

        ~Optional() = default;

        auto operator=( nullopt_t /*unused*/ ) noexcept -> Optional& {
            reset();
            return *this;
        }

        auto operator=( const Optional& other ) -> Optional& {
            if ( this != &other ) {
                if ( !other.mEngaged ) {
                    reset();
                } else if ( mEngaged ) {
                    stored_value() = *other;
                } else {
                    new( data() ) T( *other );
                    mEngaged = true;
                }
            }
            return *this;
        }

        auto operator=( Optional&& other ) noexcept( is_movable< T >::value ) -> Optional& {
            if ( !other.mEngaged ) {
                reset();
            } else if ( mEngaged ) {
                stored_value() = std::move( *other );
            } else {
                new( data() ) T( std::move( *other ) );
                mEngaged = true;
            }
            return *this;
        }

        auto operator=( const T& value ) -> Optional& {
            if ( !mEngaged ) {
                new( data() ) T( value );
                mEngaged = true;
            } else {
                stored_value() = value;
            }
            return *this;
        }

        auto operator=( T&& value ) -> Optional& {
            if ( !mEngaged ) {
                new( data() ) T( std::move( value ) );
                mEngaged = true;
            } else {
                stored_value() = std::move( value );
            }
            return *this;
        }

        BIT7Z_CPP14_CONSTEXPR auto operator*() & -> T& {
            return stored_value();
        }

        constexpr auto operator*() const& -> const T& {
            return stored_value();
        }

        BIT7Z_CPP14_CONSTEXPR auto operator->() & -> T* {
            return data();
        }

        constexpr auto operator->() const& -> const T* {
            return data();
        }

        explicit constexpr operator bool() const noexcept {
            return mEngaged;
        }

        BIT7Z_NODISCARD
        constexpr auto has_value() const noexcept -> bool {
            return mEngaged;
        }

        BIT7Z_NODISCARD
        BIT7Z_CPP14_CONSTEXPR auto value() & -> T& {
            if ( !mEngaged ) {
                throw BitException{"Bad Optional access", BitError::Fail};
            }
            return stored_value();
        }

        // Note: there are tricks to make this function constexpr also with C++11,
        // but they would make the code less clear, and we are targeting C++14 anyway.
        BIT7Z_NODISCARD
        BIT7Z_CPP14_CONSTEXPR auto value() const & -> const T& {
            if ( !mEngaged ) {
                throw BitException{"Bad Optional access", BitError::Fail};
            }
            return stored_value();
        }

        void reset() noexcept {
            if ( mEngaged ) {
                stored_value().~T();
                mEngaged = false;
            }
        }

        template <class... Args>
        auto emplace(Args&&... args) -> T& {
            reset();
            new ( raw_data() ) T( std::forward<Args>( args )... );
            mEngaged = true;
            return stored_value();
        }

    private:
        BIT7Z_NODISCARD
        BIT7Z_CPP14_CONSTEXPR auto raw_data() noexcept -> void* {
            // Note: this is not UB only when used after reset(), as the storage is then uninitialized.
            return const_cast< void * >( static_cast< const void * >( data() ) ); // NOLINT(*-pro-type-const-cast)
        }

        BIT7Z_NODISCARD
        BIT7Z_CPP14_CONSTEXPR auto data() & noexcept -> T* {
            return &mStorage.mValue;
        }

        BIT7Z_NODISCARD
        constexpr auto data() const& noexcept -> const T* {
            return &mStorage.mValue;
        }

        BIT7Z_NODISCARD
        BIT7Z_CPP14_CONSTEXPR auto stored_value() & noexcept -> T& {
            return mStorage.mValue;
        }

        BIT7Z_NODISCARD
        constexpr auto stored_value() const& noexcept -> const T& {
            return mStorage.mValue;
        }

        bool mEngaged;
        OptionalStorage<T> mStorage;
};

static_assert(std::is_trivially_destructible<Optional<int>>::value,
              "Optional<T> doesn't respect T's trivially destructible property");

} // namespace bit7z

#endif //OPTIONAL_HPP
