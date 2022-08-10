/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITFILEEXTRACTOR_HPP
#define BITFILEEXTRACTOR_HPP

#include "bitextractor.hpp"

namespace bit7z {
/**
 * @brief The BitFileExtractor class allows to extract the content of archives on the filesystem.
 */
using BitFileExtractor BIT7Z_MAYBE_UNUSED = BitExtractor< const tstring& >;
}
#endif // BITFILEEXTRACTOR_HPP
