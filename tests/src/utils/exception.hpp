/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <catch2/catch.hpp>

#include <bit7z/bitexception.hpp>

// Asserts that evaluating expression throws a BitException whose error code equals the given code.
// The code may be a BitError enumerator or a std::errc value (both compare against std::error_code).
#define REQUIRE_THROWS_CODE( expression, errorCode )                                    \
    REQUIRE_THROWS_MATCHES(                                                             \
        expression,                                                                     \
        bit7z::BitException,                                                            \
        Catch::Matchers::Predicate< bit7z::BitException >(                              \
            [&]( const bit7z::BitException& exception ) -> bool {                       \
                return exception.code() == ( errorCode );                               \
            },                                                                          \
            "Error code should be " #errorCode                                          \
        )                                                                               \
    )

#endif //EXCEPTION_HPP