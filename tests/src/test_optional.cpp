// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include <internal/optional.hpp>

using bit7z::Optional;

// Trait for parameterized testing of built-in and std::string types
template< typename T >
struct TestTraits;

template<>
struct TestTraits< int > {
    // Note: using static constexpr variables doesn't work on some compilers.

    BIT7Z_NODISCARD
    static constexpr auto init_value() noexcept -> int {
        return 42;
    }

    BIT7Z_NODISCARD
    static constexpr auto other_value() noexcept -> int {
        return 24;
    }

    BIT7Z_NODISCARD
    static constexpr auto emplaced_value() noexcept -> int {
        return 7;
    }

    static void emplace( Optional< int >& opt ) {
        opt.emplace( emplaced_value() );
    }
};

template<>
struct TestTraits< std::string > {
    BIT7Z_NODISCARD
    static constexpr auto init_value() noexcept -> const char* {
        return "hello";
    }

    BIT7Z_NODISCARD
    static constexpr auto other_value() noexcept -> const char* {
        return "world!";
    }

    BIT7Z_NODISCARD
    static constexpr auto emplaced_value() noexcept -> const char* {
        return "xxxxx";
    }

    static void emplace( Optional< std::string >& opt ) {
        opt.emplace( std::char_traits< char >::length( init_value() ), 'x' );
    }
};

using TestTypes = std::tuple< int, std::string >;

TEMPLATE_LIST_TEST_CASE( "Optional: default constructor", "[optional]", TestTypes ) {
    Optional< TestType > defaultOpt;
    REQUIRE_FALSE( defaultOpt );
    REQUIRE_FALSE( defaultOpt.has_value() );
    REQUIRE_THROWS_AS( defaultOpt.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: nullopt constructor", "[optional]", TestTypes ) {
    Optional< TestType > emptyOpt{ bit7z::nullopt };
    REQUIRE_FALSE( emptyOpt );
    REQUIRE_FALSE( emptyOpt.has_value() );
    REQUIRE_THROWS_AS( emptyOpt.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: value constructor (copy)", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    const TestType value{ Traits::init_value() };
    Optional< TestType > opt{ value };
    REQUIRE( opt );
    REQUIRE( opt.has_value() );
    REQUIRE( *opt == Traits::init_value() );
    REQUIRE( opt.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: value constructor (move)", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    TestType value{ Traits::init_value() };
    Optional< TestType > opt{ std::move( value ) };
    REQUIRE( opt );
    REQUIRE( opt.has_value() );
    REQUIRE( *opt == Traits::init_value() );
    REQUIRE( opt.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: copy construction", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    const Optional< TestType > original{ Traits::init_value() };
    Optional< TestType > copyConstructed{ original };
    REQUIRE( copyConstructed );
    REQUIRE( copyConstructed.has_value() );
    REQUIRE( *copyConstructed == *original );
    REQUIRE( copyConstructed.value() == original.value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: copy construction from an empty Optional", "[optional]", TestTypes ) {
    const Optional< TestType > emptyOpt = bit7z::nullopt;
    Optional< TestType > emptyCopyConstructed{ emptyOpt };
    REQUIRE_FALSE( emptyCopyConstructed );
    REQUIRE_FALSE( emptyCopyConstructed.has_value() );
    REQUIRE_THROWS_AS( emptyCopyConstructed.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: move construction", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    Optional< TestType > original{ Traits::init_value() };
    Optional< TestType > moveConstructed{ std::move( original ) };
    REQUIRE( moveConstructed );
    REQUIRE( moveConstructed.has_value() );
    REQUIRE( *moveConstructed == Traits::init_value() );
    REQUIRE( moveConstructed.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: move construction from an empty Optional", "[optional]", TestTypes ) {
    Optional< TestType > original = bit7z::nullopt;
    Optional< TestType > emptyMoveConstructed{ std::move( original ) };
    REQUIRE_FALSE( emptyMoveConstructed );
    REQUIRE_FALSE( emptyMoveConstructed.has_value() );
    REQUIRE_THROWS_AS( emptyMoveConstructed.value(), bit7z::BitException );
}

TEST_CASE( "Optional: in-place constructor", "[optional]" ) {
    const Optional< std::string > inPlace{ bit7z::in_place, 5u, 'a' };
    REQUIRE( inPlace );
    REQUIRE( inPlace.has_value() );
    REQUIRE( *inPlace == "aaaaa" );
    REQUIRE( inPlace.value() == "aaaaa" );

    // We deliberately only take the first 5 chars of the C string "world!" using one of std::string's constructors.
    const Optional< std::string > inPlace2{ bit7z::in_place, TestTraits< std::string >::other_value(), 5u };
    REQUIRE( inPlace2 );
    REQUIRE( inPlace2.has_value() );
    REQUIRE( *inPlace2 == "world" );
    REQUIRE( inPlace2.value() == "world" );

    const Optional< std::string > inPlace3{ bit7z::in_place };
    REQUIRE( inPlace3 );
    REQUIRE( inPlace3.has_value() );
    REQUIRE( inPlace3->empty() );
    REQUIRE( *inPlace3 == "" );
    REQUIRE( inPlace3.value() == "" );
}

TEMPLATE_LIST_TEST_CASE( "Optional: nullopt assignment", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    auto assignOpt = GENERATE( as< Optional< TestType > >(),
                               Optional< TestType >{},
                               bit7z::nullopt,
                               Traits::init_value() );
    assignOpt = bit7z::nullopt;
    REQUIRE_FALSE( assignOpt );
    REQUIRE_FALSE( assignOpt.has_value() );
    REQUIRE_THROWS_AS( assignOpt.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: copy assignment using an empty Optional", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    const Optional< TestType > emptyOpt;
    auto copyAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::init_value() );
    copyAssigned = emptyOpt;
    REQUIRE_FALSE( copyAssigned );
    REQUIRE_FALSE( copyAssigned.has_value() );
    REQUIRE_THROWS_AS( copyAssigned.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: copy assignment", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    const Optional< TestType > original{ Traits::init_value() };
    auto copyAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::other_value() );
    copyAssigned = original;
    REQUIRE( copyAssigned );
    REQUIRE( copyAssigned.has_value() );
    REQUIRE( *copyAssigned == *original );
    REQUIRE( copyAssigned.value() == original.value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: move assignment using an empty Optional", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    auto moveAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::init_value() );
    moveAssigned = Optional< TestType >{};
    REQUIRE_FALSE( moveAssigned );
    REQUIRE_FALSE( moveAssigned.has_value() );
    REQUIRE_THROWS_AS( moveAssigned.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: move assignment", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    Optional< TestType > original{ Traits::init_value() };
    auto moveAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::other_value() );
    moveAssigned = std::move( original );
    REQUIRE( moveAssigned );
    REQUIRE( moveAssigned.has_value() );
    REQUIRE( *moveAssigned == Traits::init_value() );
    REQUIRE( moveAssigned.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: value assignment (copy)", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    const TestType original{ Traits::init_value() };
    auto moveAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::other_value() );
    moveAssigned = original;
    REQUIRE( moveAssigned );
    REQUIRE( moveAssigned.has_value() );
    REQUIRE( *moveAssigned == Traits::init_value() );
    REQUIRE( moveAssigned.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: value assignment (move)", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    auto moveAssigned = GENERATE( as< Optional< TestType > >(),
                                  Optional< TestType >{},
                                  bit7z::nullopt,
                                  Traits::other_value() );
    moveAssigned = Traits::init_value();
    REQUIRE( moveAssigned );
    REQUIRE( moveAssigned.has_value() );
    REQUIRE( *moveAssigned == Traits::init_value() );
    REQUIRE( moveAssigned.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: self copy/move assignment", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;
    Optional< TestType > opt{ Traits::init_value() };

    // ReSharper disable once CppIdenticalOperandsInBinaryExpression
    opt = opt; // Self copy-assign
    REQUIRE( opt );
    REQUIRE( opt.has_value() );
    REQUIRE( *opt == Traits::init_value() );
    REQUIRE( opt.value() == Traits::init_value() );

    opt = std::move( opt ); // Self move-assign
    REQUIRE( opt );
    REQUIRE( opt.has_value() );
    REQUIRE( *opt == Traits::init_value() );
    REQUIRE( opt.value() == Traits::init_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: reset", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    auto opt = GENERATE( as< Optional< TestType > >(),
                         Optional< TestType >{},
                         bit7z::nullopt,
                         Traits::init_value() );
    opt.reset();
    REQUIRE_FALSE( opt );
    REQUIRE_FALSE( opt.has_value() );
    REQUIRE_THROWS_AS( opt.value(), bit7z::BitException );
}

TEMPLATE_LIST_TEST_CASE( "Optional: emplace", "[optional]", TestTypes ) {
    using Traits = TestTraits< TestType >;

    auto opt = GENERATE( as< Optional< TestType > >(),
                         Optional< TestType >{},
                         bit7z::nullopt,
                         Traits::init_value() );
    Traits::emplace( opt );
    REQUIRE( opt );
    REQUIRE( opt.has_value() );
    REQUIRE( *opt == Traits::emplaced_value() );
    REQUIRE( opt.value() == Traits::emplaced_value() );
}

TEMPLATE_LIST_TEST_CASE( "Optional: constant Optional should be readable", "[optional]", TestTypes ) {
    const Optional< TestType > defaultOpt;
    REQUIRE_FALSE( defaultOpt );
    REQUIRE_FALSE( defaultOpt.has_value() );
    REQUIRE_THROWS_AS( defaultOpt.value(), bit7z::BitException );

    const Optional< TestType > nulloptOpt = bit7z::nullopt;
    REQUIRE_FALSE( nulloptOpt );
    REQUIRE_FALSE( nulloptOpt.has_value() );
    REQUIRE_THROWS_AS( nulloptOpt.value(), bit7z::BitException );

    using Traits = TestTraits< TestType >;

    const Optional< TestType > valueOpt{ Traits::init_value() };
    REQUIRE( valueOpt );
    REQUIRE( valueOpt.has_value() );
    REQUIRE( *valueOpt == Traits::init_value() );
    REQUIRE( valueOpt.value() == Traits::init_value() );
}

constexpr struct throwing_tag_t {} throwing_tag{};

class CustomType final {
    public:
        static int constructionCount;
        static int destructionCount;

        explicit CustomType( int value ) : mValue{ value } {
            ++constructionCount;
        }

        explicit CustomType( throwing_tag_t ) : mValue{} {
            throw std::runtime_error{ "ctor throws" };
        }

        CustomType( const CustomType& other ) : mValue{ other.mValue } {
            ++constructionCount;
        }

        CustomType( CustomType&& other ) noexcept : mValue{ other.mValue } {
            other.mValue = 0;
            ++constructionCount;
        }

        ~CustomType() {
            ++destructionCount;
        }

        auto operator=( const CustomType& other ) -> CustomType& {
            if ( this != &other ) {
                mValue = other.mValue;
            }
            return *this;
        }

        auto operator=( CustomType&& other ) noexcept -> CustomType& {
            mValue = other.mValue;
            other.mValue = 0;
            return *this;
        }

        auto operator==( const CustomType& other ) const -> bool {
            return mValue == other.mValue;
        }

        auto operator==( int other ) const -> bool {
            return mValue == other;
        }

        BIT7Z_NODISCARD
        auto customValue() const noexcept -> int {
            return mValue;
        }

    private:
        int mValue;
};

int CustomType::constructionCount = 0;
int CustomType::destructionCount = 0;

TEST_CASE( "Optional: lifecycle of an empty Optional", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > opt;
        REQUIRE( CustomType::constructionCount == 0 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    REQUIRE( CustomType::destructionCount == 0 );
}

TEST_CASE( "Optional: lifecycle of a in-place constructed Optional", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        const Optional< CustomType > opt{ bit7z::in_place, 10 };
        REQUIRE( *opt == 10 );
        REQUIRE( opt.value() == 10 );
        REQUIRE( opt->customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: lifecycle after copy construction", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    // Emplace new object
    {
        const Optional< CustomType > opt{ bit7z::in_place, 10 };
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );

        Optional< CustomType > copiedOpt{ opt };
        REQUIRE( *copiedOpt == 10 );
        REQUIRE( copiedOpt.value() == 10 );
        REQUIRE( copiedOpt->customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 2 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // All destructors should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: lifecycle after move construction", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    // Emplace new object
    {
        Optional< CustomType > opt{ bit7z::in_place, 10 };
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );

        Optional< CustomType > movedOpt{ std::move( opt ) };
        REQUIRE( *movedOpt == 10 );
        REQUIRE( movedOpt.value() == 10 );
        REQUIRE( movedOpt->customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 2 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // All destructors should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: lifecycle after copy construct from value", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        const CustomType value{ 10 };
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );

        Optional< CustomType > opt{ value };
        REQUIRE( *opt == 10 );
        REQUIRE( opt.value() == 10 );
        REQUIRE( opt->customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 2 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // All destructors should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: lifecycle after move construct from value", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        CustomType value{ 10 };
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );

        Optional< CustomType > opt{ std::move( value ) };
        REQUIRE( *opt == 10 );
        REQUIRE( opt.value() == 10 );
        REQUIRE( opt->customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 2 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // All destructors should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: a throwing constructor should not leave a partially constructed optional", "[optional]" ) {
    REQUIRE_THROWS_AS( Optional< CustomType >( bit7z::in_place, throwing_tag ), std::runtime_error );
    REQUIRE_THROWS_AS( Optional< CustomType >( CustomType{ throwing_tag } ), std::runtime_error );
}

TEST_CASE( "Optional: behavior of emplace when an exception is thrown", "[optional]" ) {
    Optional< CustomType > opt{ bit7z::in_place, 42 };
    try {
        opt.emplace( throwing_tag );
    } catch ( ... ) {
        REQUIRE_FALSE( opt );
        REQUIRE_FALSE( opt.has_value() );
        REQUIRE_THROWS_AS( opt.value(), bit7z::BitException );
    }
}

TEST_CASE( "Optional: lifecycle after emplace", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > emplacedOpt;
        const auto& ref = emplacedOpt.emplace( 10 );

        REQUIRE( ref.customValue() == 10 );
        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // The destructor should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: emplace must overwrite and destroy the previous value", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > emplacedOpt;
        emplacedOpt.emplace( 10 );
        emplacedOpt.emplace( 20 ); // Should destroy 10 and construct 20.

        REQUIRE( emplacedOpt->customValue() == 20 );
        REQUIRE( CustomType::constructionCount == 2 );
        REQUIRE( CustomType::destructionCount == 1 );
    }

    // All destructors should have run.
    REQUIRE( CustomType::destructionCount == CustomType::constructionCount );
}

TEST_CASE( "Optional: reset on an empty optional should do nothing", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > resetOpt;
        resetOpt.reset();

        REQUIRE( CustomType::constructionCount == 0 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // No unnecessary destructor call.
    REQUIRE( CustomType::destructionCount == 0 );
}

TEST_CASE( "Optional: assigning nullopt to an empty optional should do nothing", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        // ReSharper disable once CppJoinDeclarationAndAssignment
        Optional< CustomType > resetOpt;
        resetOpt = bit7z::nullopt;

        REQUIRE( CustomType::constructionCount == 0 );
        REQUIRE( CustomType::destructionCount == 0 );
    }

    // No unnecessary destructor call.
    REQUIRE( CustomType::destructionCount == 0 );
}

TEST_CASE( "Optional: reset must destroy the previous value", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > resetOpt{ bit7z::in_place, 10 };
        resetOpt.reset();

        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 1 );
    }

    // No unnecessary destructor call.
    REQUIRE( CustomType::destructionCount == 1 );
}

TEST_CASE( "Optional: assigning nullopt must destroy the previous value", "[optional]" ) {
    CustomType::constructionCount = 0;
    CustomType::destructionCount = 0;

    {
        Optional< CustomType > resetOpt{ bit7z::in_place, 10 };
        resetOpt = bit7z::nullopt;
        REQUIRE_FALSE( resetOpt );

        REQUIRE( CustomType::constructionCount == 1 );
        REQUIRE( CustomType::destructionCount == 1 );
    }

    // No unnecessary destructor call.
    REQUIRE( CustomType::destructionCount == 1 );
}