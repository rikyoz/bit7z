/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FORMATDETECT_HPP
#define FORMATDETECT_HPP

#ifdef BIT7Z_AUTO_FORMAT

#include "bitformat.hpp"

struct IInStream;

namespace bit7z {

const BitInFormat& detectFormatFromExt( const tstring& in_file );

const BitInFormat& detectFormatFromSig( IInStream* stream );

} // namespace bit7z

#endif

#endif