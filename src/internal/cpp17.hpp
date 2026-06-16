/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CPP17_HPP
#define CPP17_HPP

#include <type_traits>

namespace bit7z {
namespace cpp17 {

template< bool B >
using bool_constant = std::integral_constant< bool, B >; // like C++17's std::bool_constant

} // namespace cpp17
} // namespace bit7z

#endif //CPP17_HPP
