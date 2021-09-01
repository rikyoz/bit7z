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

#ifndef BITCOMPRESSIONLEVEL_HPP
#define BITCOMPRESSIONLEVEL_HPP

namespace bit7z {
    /**
     * @brief Enum that represents the compression level used by 7z when creating archives.
     * @note It uses the same values as in the 7z SDK (https://sevenzip.osdn.jp/chm/cmdline/switches/method.htm#ZipX).
     */
    enum struct BitCompressionLevel {
        None = 0,    ///< Copy mode (no compression)
        Fastest = 1, ///< Fastest compressing
        Fast = 3,    ///< Fast compressing
        Normal = 5,  ///< Normal compressing
        Max = 7,     ///< Maximum compressing
        Ultra = 9    ///< Ultra compressing
    };
}

#endif // BITCOMPRESSIONLEVEL_HPP
