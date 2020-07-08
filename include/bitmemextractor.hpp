/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#ifndef BITMEMEXTRACTOR_HPP
#define BITMEMEXTRACTOR_HPP

#include "../include/bitextractor.hpp"
#include "../include/bittypes.hpp"

namespace bit7z {
    /**
     * @brief The BitMemExtractor class allows to extract the content of in-memory archives.
     */
    template class BitExtractor< const vector< byte_t >& >;
    using BitMemExtractor = BitExtractor< const vector< byte_t >& >;
}

#endif // BITMEMEXTRACTOR_HPP
