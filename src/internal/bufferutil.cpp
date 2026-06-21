// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/bufferutil.hpp"

#include "bittypes.hpp"
#include "bitwindows.hpp"
#include "internal/util.hpp"

auto bit7z::seek(
    const buffer_t& buffer,
    const buffer_t::const_iterator& currentPosition,
    std::int64_t offset,
    SeekOrigin seekOrigin,
    std::uint64_t& newPosition
) -> HRESULT {
    std::uint64_t currentIndex{};
    switch ( seekOrigin ) {
        case SeekOrigin::Begin: {
            break;
        }
        case SeekOrigin::CurrentPosition: {
            currentIndex = static_cast< std::uint64_t >( currentPosition - buffer.cbegin() );
            break;
        }
        case SeekOrigin::End: {
            currentIndex = static_cast< std::uint64_t >( buffer.cend() - buffer.cbegin() );
            break;
        }
    }

    RINOK( seekToOffset( currentIndex, offset ) ) //-V3504

    if ( currentIndex > buffer.size() ) {
        return E_INVALIDARG;
    }

    newPosition = currentIndex;
    return S_OK;
}
