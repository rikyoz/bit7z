/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DATEUTIL_HPP
#define DATEUTIL_HPP

#include "bitpropvariant.hpp"
#include "bitwindows.hpp"
#include "internal/fs.hpp"

#ifndef _WIN32
#include <ctime>
#endif

namespace bit7z {

#ifndef _WIN32

auto toFileTimeType( FILETIME fileTime ) -> fs::file_time_type;

auto toFILETIME( std::time_t value ) -> FILETIME;

#endif

auto toTimeType( FILETIME fileTime ) -> time_type;

auto toFILETIME( time_type timePoint ) -> FILETIME;

auto currentFileTime() -> FILETIME;

} // namespace bit7z

#endif //DATEUTIL_HPP
