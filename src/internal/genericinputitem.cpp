// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/genericinputitem.hpp"
#include "util.hpp"

namespace bit7z {

auto GenericInputItem::hasNewData() const noexcept -> bool {
    return true;
}

auto GenericInputItem::itemProperty( BitProperty propID ) const -> BitPropVariant {
    BitPropVariant prop;
    switch ( propID ) {
        case BitProperty::Path: {
#if defined(_MSC_VER) || !defined(BIT7Z_USE_STANDARD_FILESYSTEM)
            prop = inArchivePath().wstring();
#else
            // On some compilers and platforms (e.g., GCC before v12.3),
            // the direct conversion of the fs::path to wstring might throw an exception due to unicode characters.
            // So we simply convert to tstring, and then widen it if necessary.
            prop = WIDEN( inArchivePath().string< tchar >() );
#endif
            break;
        }
        case BitProperty::IsDir:
            prop = isDir();
            break;
        case BitProperty::Size:
            prop = size();
            break;
        case BitProperty::Attrib:
            prop = attributes();
            break;
        case BitProperty::CTime:
            prop = creationTime();
            break;
        case BitProperty::ATime:
            prop = lastAccessTime();
            break;
        case BitProperty::MTime:
            prop = lastWriteTime();
            break;
        default: //empty prop
            break;
    }
    return prop;
}

} // namespace bit7z