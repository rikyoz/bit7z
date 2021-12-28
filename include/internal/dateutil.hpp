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

#ifndef DATEUTIL_HPP
#define DATEUTIL_HPP

#include <chrono>
#include <ctime>

#include "bitgenericitem.hpp"
#include "bitwindows.hpp"
#include "internal/fs.hpp"

namespace bit7z {
#ifndef _WIN32
    fs::file_time_type FILETIME_to_file_time_type( const FILETIME& fileTime );

    FILETIME time_to_FILETIME( const std::time_t& time );
#endif

    time_type FILETIME_to_time_type( const FILETIME& fileTime );

    FILETIME currentFileTime();
}

#endif //DATEUTIL_HPP
