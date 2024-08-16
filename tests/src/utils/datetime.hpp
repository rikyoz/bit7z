/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DATETIME_HPP
#define DATETIME_HPP

#ifndef _WIN32

#include <bit7z/bitfs.hpp>

#include <chrono>
#include <cstddef>

namespace bit7z {
namespace test {

#if defined( BIT7Z_USE_STANDARD_FILESYSTEM ) && defined( __GLIBCXX__ )
constexpr std::chrono::seconds libstdcpp_file_clock_epoch{ 6437664000 };
#endif

template<
    class Clock,
    class Duration = typename Clock::duration
>
auto as_unix_timestamp( const std::chrono::time_point< Clock, Duration > timePoint ) -> std::uint64_t {
    const auto asSeconds = std::chrono::duration_cast< std::chrono::seconds >( timePoint.time_since_epoch() );
    return static_cast< std::uint64_t >( asSeconds.count() );
}

#if defined( BIT7Z_USE_STANDARD_FILESYSTEM ) && defined( __GLIBCXX__ )
template<>
inline auto as_unix_timestamp( const fs::file_time_type timePoint ) -> std::uint64_t {
    const auto sinceEpoch = timePoint.time_since_epoch();
    const auto asSeconds = std::chrono::duration_cast< std::chrono::seconds >( sinceEpoch );
    const auto sinceUnixEpoch = libstdcpp_file_clock_epoch + asSeconds;

    auto nano = std::chrono::duration_cast< std::chrono::nanoseconds >( sinceEpoch - asSeconds );
    if ( nano < fs::file_time_type::duration::zero() ) {
        return static_cast< std::uint64_t >( ( sinceUnixEpoch - std::chrono::seconds{ 1 } ).count() );
    }

    return static_cast< std::uint64_t >( sinceUnixEpoch.count() );
}
#endif

} // namespace test
} // namespace bit7z

#endif

#endif //DATETIME_HPP
