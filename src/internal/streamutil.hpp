/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STREAMUTIL_HPP
#define STREAMUTIL_HPP

#include "internal/seekorigin.hpp"

#include <ios>

namespace bit7z {

inline auto toSeekdir( SeekOrigin seekOrigin ) -> std::ios_base::seekdir {
    switch ( seekOrigin ) {
        case SeekOrigin::CurrentPosition:
            return std::ios_base::cur;
        case SeekOrigin::End:
            return std::ios_base::end;
        case SeekOrigin::Begin:
        default:
            return std::ios_base::beg;
    }
}

} // namespace bit7z

#endif //STREAMUTIL_HPP
