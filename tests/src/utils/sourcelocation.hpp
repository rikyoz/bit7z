/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SOURCELOCATION_HPP
#define SOURCELOCATION_HPP

#include <bit7z/bitdefines.hpp>

#include <cstdint>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

#if (!defined(__apple_build_version__) && defined(__clang__) && (__clang_major__ >= 9)) || \
        (!defined(__clang__) && defined(__GNUC__))
#define BIT7Z_COMPILER_SUPPORT_SOURCE_BUILTINS
#endif

/**
 * A naive version of C++20's std::source_location.
 */
struct SourceLocation final {
    public:
        using number_type = std::uint_least32_t;


#ifdef BIT7Z_COMPILER_SUPPORT_SOURCE_BUILTINS
        BIT7Z_NODISCARD
        static constexpr auto current( const char* fileName = __builtin_FILE(),
                                       const char* functionName = __builtin_FUNCTION(),
                                       number_type lineNumber = __builtin_LINE() ) noexcept -> SourceLocation {
            return { fileName, functionName, lineNumber };
        }
#else
        BIT7Z_NODISCARD
        static constexpr auto current( const char* fileName,
                                       const char* functionName,
                                       number_type lineNumber ) noexcept -> SourceLocation {
            return { fileName, functionName, lineNumber };
        }
#endif

        SourceLocation( const SourceLocation& ) = default;

        auto operator=( const SourceLocation& ) -> SourceLocation& = default;

        SourceLocation( SourceLocation&& ) = default;

        auto operator=( SourceLocation&& ) -> SourceLocation& = default;

        ~SourceLocation() = default;

        BIT7Z_NODISCARD
        constexpr auto file_name() const noexcept -> const char* {
            return mFileName;
        }

        BIT7Z_NODISCARD
        BIT7Z_MAYBE_UNUSED
        constexpr auto function_name() const noexcept -> const char* {
            return mFunctionName;
        }

        BIT7Z_NODISCARD
        constexpr auto line() const noexcept -> SourceLocation::number_type {
            return mLineNumber;
        }

    private:
        constexpr SourceLocation( const char* fileName, const char* functionName, number_type lineNumber ) noexcept
            : mFileName( fileName ), mFunctionName( functionName ), mLineNumber( lineNumber ) {}

        const char* mFileName;
        const char* mFunctionName;
        number_type mLineNumber;
};

#ifdef BIT7Z_COMPILER_SUPPORT_SOURCE_BUILTINS
#define BIT7Z_CURRENT_LOCATION SourceLocation::current()
#else
#define BIT7Z_CURRENT_LOCATION SourceLocation::current( __FILE__, __FUNCTION__, __LINE__ )
#endif

} // namespace test
} // namespace bit7z

#endif //SOURCELOCATION_HPP