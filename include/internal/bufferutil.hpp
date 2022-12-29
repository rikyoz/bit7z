/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

} // namespace bit7z

#endif //BUFFERUTIL_HPP
