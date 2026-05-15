/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FORMATDETECT_HPP
#define FORMATDETECT_HPP

#include "bitdefines.hpp" // For BIT7Z_AUTO_FORMAT.

#ifdef BIT7Z_AUTO_FORMAT

#include "bitfs.hpp"

struct IInStream;

namespace bit7z {

class BitInFormat;

#ifdef BIT7Z_DETECT_FROM_EXTENSION

auto detectFormatFromExtension( const fs::path& inFile ) -> const BitInFormat&;

#endif

auto detectFormatFromSignature( IInStream* stream ) -> const BitInFormat&;

} // namespace bit7z

#endif

#endif
