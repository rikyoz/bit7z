/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITCOMPRESSIONMETHOD_HPP
#define BITCOMPRESSIONMETHOD_HPP

#include <cstdint>

namespace bit7z {

/**
 * @brief The BitCompressionMethod enum represents the compression methods used by 7z when creating archives.
 */
enum struct BitCompressionMethod : std::uint8_t {
    Copy,       ///< No compression (the data is stored as-is).
    Deflate,    ///< The Deflate compression method.
    Deflate64,  ///< The Deflate64 compression method.
    BZip2,      ///< The BZip2 compression method.
    Lzma,       ///< The LZMA compression method.
    Lzma2,      ///< The LZMA2 compression method.
    Ppmd        ///< The PPMd compression method.
};

} // namespace bit7z

#endif // BITCOMPRESSIONMETHOD_HPP
