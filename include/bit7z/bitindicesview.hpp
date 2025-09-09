/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2025 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITINDICESVIEW_HPP
#define BITINDICESVIEW_HPP

#include "bitdefines.hpp"
#include "bitview.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

namespace bit7z {

using BitIndicesView = BitView< const std::uint32_t >;

} // namespace bit7z

#endif //BITINDICESVIEW_HPP
