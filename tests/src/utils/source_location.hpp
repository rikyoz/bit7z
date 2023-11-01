/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <bit7z/bitdefines.hpp>

#include <cstdint>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

/**
 * A naive version of C++20's std::source_location.
 */
struct source_location final {
    public:
        using number_type = std::uint_least32_t;


#ifdef _MSC_VER
        BIT7Z_NODISCARD
        static constexpr auto current( const char* fileName,
                                       const char* functionName,
                                       number_type lineNumber ) noexcept -> source_location {
            return { fileName, functionName, lineNumber };
        }
#else
        BIT7Z_NODISCARD
        static constexpr auto current( const char* fileName = __builtin_FILE(),
                                       const char* functionName = __builtin_FUNCTION(),
                                       number_type lineNumber = __builtin_LINE() ) noexcept -> source_location {
            return { fileName, functionName, lineNumber };
        }
#endif

        source_location( const source_location& ) = default;

        auto operator=( const source_location& ) -> source_location& = default;

        source_location( source_location&& ) = default;

        auto operator=( source_location&& ) -> source_location& = default;

        ~source_location() = default;

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
        constexpr auto line() const noexcept -> source_location::number_type {
            return mLineNumber;
        }

    private:
        constexpr source_location( const char* fileName, const char* functionName, number_type lineNumber ) noexcept
            : mFileName( fileName ), mFunctionName( functionName ), mLineNumber( lineNumber ) {}

        const char* mFileName;
        const char* mFunctionName;
        number_type mLineNumber;
};

#ifdef _MSC_VER
#define BIT7Z_CURRENT_LOCATION source_location::current( __FILE__, __FUNCTION__, __LINE__ )
#else
#define BIT7Z_CURRENT_LOCATION source_location::current()
#endif

} // namespace test
} // namespace bit7z

#endif //SOURCE_LOCATION_HPP