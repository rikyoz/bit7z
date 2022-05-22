/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2022  Riccardo Ostani - All Rights Reserved.
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

#ifndef BUFFERUTIL_HPP
#define BUFFERUTIL_HPP

#include "bittypes.hpp"
#include "internal/util.hpp"

namespace bit7z {
HRESULT seek( const buffer_t& buffer,
              const buffer_t::const_iterator& current_position,
              int64_t offset,
              uint32_t seek_origin,
              int64_t& new_position );
}

#endif //BUFFERUTIL_HPP
