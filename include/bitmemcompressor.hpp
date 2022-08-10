/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITMEMCOMPRESSOR_HPP
#define BITMEMCOMPRESSOR_HPP

#include "bitcompressor.hpp"

namespace bit7z {
/**
 * @brief The BitMemCompressor class allows to compress memory buffers to the filesystem, to standard streams,
 *        or to other memory buffers.
 *
 * It let decide various properties of the produced archive file, such as the password
 * protection and the compression level desired.
 */
using BitMemCompressor BIT7Z_MAYBE_UNUSED = BitCompressor< const buffer_t >;
}
#endif // BITMEMCOMPRESSOR_HPP
