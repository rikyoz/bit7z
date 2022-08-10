// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/genericinputitem.hpp"

namespace bit7z {
bool GenericInputItem::hasNewData() const noexcept {
    return true;
}

BitPropVariant GenericInputItem::itemProperty( BitProperty propID ) const {
    BitPropVariant prop;
    switch ( propID ) {
        case BitProperty::Path:
            prop = inArchivePath().wstring();
            break;
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