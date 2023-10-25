/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITSTREAMEXTRACTOR_HPP
#define BITSTREAMEXTRACTOR_HPP

#include "bitdefines.hpp"
#include "bitextractor.hpp"

#include <istream>

namespace bit7z {

/**
 * @brief The BitStreamExtractor alias allows extracting the content of in-memory archives.
 */
using BitStreamExtractor BIT7Z_MAYBE_UNUSED = BitExtractor< std::istream& >;

} // namespace bit7z

#endif // BITSTREAMEXTRACTOR_HPP
