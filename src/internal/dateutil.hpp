/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

}  // namespace bit7z

#endif //DATEUTIL_HPP
