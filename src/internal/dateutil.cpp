// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/dateutil.hpp"

namespace bit7z {
// 100ns intervals
using FileTimeTickRate = std::ratio< 1, 10'000'000 >;
// FileTimeDuration has the same layout as FILETIME;
using FileTimeDuration = std::chrono::duration< int64_t, FileTimeTickRate >;
// Seconds between 01/01/1601 (NT epoch) and 01/01/1970 (Unix epoch):
constexpr std::chrono::seconds nt_to_unix_epoch{ -11644473600 };

#ifndef _WIN32

auto FILETIME_to_file_time_type( const FILETIME& fileTime ) -> fs::file_time_type {
    const FileTimeDuration file_time_duration{
        ( static_cast< int64_t >( fileTime.dwHighDateTime ) << 32 ) + fileTime.dwLowDateTime
    };

    const auto unix_epoch = file_time_duration + nt_to_unix_epoch;
    return fs::file_time_type{ std::chrono::duration_cast< std::chrono::system_clock::duration >( unix_epoch ) };
}

auto time_to_FILETIME( const std::time_t& time ) -> FILETIME {
    uint64_t time_in_seconds = ( time * 10000000ull ) + 116444736000000000;
    FILETIME fileTime{};
    fileTime.dwLowDateTime = static_cast< DWORD >( time_in_seconds );
    fileTime.dwHighDateTime = static_cast< DWORD >( time_in_seconds >> 32 );
    return fileTime;
}

#endif

auto FILETIME_to_time_type( const FILETIME& fileTime ) -> time_type {
    const FileTimeDuration file_time_duration{
        ( static_cast< int64_t >( fileTime.dwHighDateTime ) << 32 ) + fileTime.dwLowDateTime
    };

    const auto unix_epoch = file_time_duration + nt_to_unix_epoch;
    return time_type{ std::chrono::duration_cast< std::chrono::system_clock::duration >( unix_epoch ) };
}

auto currentFileTime() -> FILETIME {
#ifdef _WIN32
    FILETIME file_time{};
    SYSTEMTIME system_time{};

    GetSystemTime( &system_time ); // gets current time
    SystemTimeToFileTime( &system_time, &file_time ); // converts to file time format
    return file_time;
#else
    auto current_time = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t( current_time );
    return time_to_FILETIME( time );
#endif
}
}  // namespace bit7z

//#endif