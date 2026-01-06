/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef TEST_HPP
#define TEST_HPP

#include <catch2/catch.hpp>

#include <bit7z/bitdefines.hpp>

#include <locale>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

BIT7Z_ALWAYS_INLINE
auto quoted( const std::string& str ) -> std::string {
    return "\"" + str + "\"";
}

BIT7Z_ALWAYS_INLINE
auto quoted( const char* str ) -> std::string {
    return "\"" + std::string{ str } + "\"";
}

#ifdef _WIN32
BIT7Z_ALWAYS_INLINE
auto quoted( const std::wstring& str ) -> std::string {
    return Catch::StringMaker< std::wstring >::convert( str );
}

BIT7Z_ALWAYS_INLINE
auto quoted( const wchar_t* str ) -> std::string {
    return Catch::StringMaker< const wchar_t* >::convert( str );
}
#endif

template< typename CharT, typename StringType = std::basic_string< CharT > >
class CasePermutationGenerator : public Catch::Generators::IGenerator< StringType > {
    StringType base;
    std::vector< StringType > variations;
    size_t index = 0;

    static auto toPermutation( const StringType& str, size_t mask ) -> StringType {
        static const auto& classicLocale = std::locale::classic();

        StringType out = str;
        for ( size_t i = 0; i < str.size(); ++i ) {
            if ( !std::isalpha( str[ i ] ) ) {
                continue;
            }

            if ( mask & ( 1ULL << i ) ) {
                out[i] = std::toupper( str[i], classicLocale );
            } else {
                out[i] = std::tolower( str[i], classicLocale );
            }
        }
        return out;
    }

    public:
        explicit CasePermutationGenerator( const CharT* input ) : base{ input } {
            const size_t total = 1ULL << base.size();

            variations.reserve( total );
            for ( size_t mask = 0; mask < total; ++mask ) {
                variations.push_back( toPermutation( base, mask ) );
            }
        }

        auto get() const -> const StringType& override {
            return variations[ index ];
        }

        auto next() -> bool override {
            if ( index + 1 < variations.size() ) {
                ++index;
                return true;
            }
            return false;
        }
};

template< typename CharT, typename StringType = std::basic_string< CharT > >
auto casePermutations( const CharT* str ) -> Catch::Generators::GeneratorWrapper< StringType > {
    return Catch::Generators::GeneratorWrapper< StringType >{
        std::make_unique< CasePermutationGenerator< CharT > >( str )
    };
}

} // namespace test
} // namespace bit7z

#endif //TEST_HPP