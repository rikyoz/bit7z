// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
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
    fs::file_time_type FILETIME_to_file_time_type( const FILETIME& fileTime ) {
        const FileTimeDuration file_time_duration{
            ( static_cast< int64_t >( fileTime.dwHighDateTime ) << 32 ) + fileTime.dwLowDateTime
        };

        const auto unix_epoch = file_time_duration + nt_to_unix_epoch;
        return fs::file_time_type{ std::chrono::duration_cast< std::chrono::system_clock::duration >( unix_epoch ) };
    }

    FILETIME time_to_FILETIME( const std::time_t& time ) {
        uint64_t time_in_seconds = ( time * 10000000ull ) + 116444736000000000;
        FILETIME fileTime{};
        fileTime.dwLowDateTime = static_cast< DWORD >( time_in_seconds );
        fileTime.dwHighDateTime = static_cast< DWORD >( time_in_seconds >> 32 );
        return fileTime;
    }
#endif

    time_type FILETIME_to_time_type( const FILETIME& fileTime ) {
        const FileTimeDuration file_time_duration{
            ( static_cast< int64_t >( fileTime.dwHighDateTime ) << 32 ) + fileTime.dwLowDateTime
        };

        const auto unix_epoch = file_time_duration + nt_to_unix_epoch;
        return time_type{ std::chrono::duration_cast< std::chrono::system_clock::duration >( unix_epoch ) };
    }

    FILETIME currentFileTime() {
#ifdef _WIN32
        FILETIME ft{};
        SYSTEMTIME st{};

        GetSystemTime( &st ); // gets current time
        SystemTimeToFileTime( &st, &ft ); // converts to file time format
        return ft;
#else
        auto current_time = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t( current_time );
        return time_to_FILETIME( time );
#endif
    }
}

//#endif