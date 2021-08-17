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

#ifndef BITGENERICITEM_HPP
#define BITGENERICITEM_HPP

#include <cstdint>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#endif

#include "bitpropvariant.hpp"

namespace bit7z {
    class BitGenericItem {
        public:
            BIT7Z_NODISCARD virtual bool isDir() const = 0;

            BIT7Z_NODISCARD virtual uint64_t size() const = 0;

            BIT7Z_NODISCARD virtual FILETIME creationTime() const = 0;

            BIT7Z_NODISCARD virtual FILETIME lastAccessTime() const = 0;

            BIT7Z_NODISCARD virtual FILETIME lastWriteTime() const = 0;

            BIT7Z_NODISCARD virtual tstring name() const = 0;

            BIT7Z_NODISCARD virtual tstring path() const = 0;

            BIT7Z_NODISCARD virtual uint32_t attributes() const = 0;

            /**
             * @brief Gets the specified item property.
             *
             * @param property  the property to be retrieved.
             *
             * @return the value of the item property, if available, or an empty BitPropVariant.
             */
            BIT7Z_NODISCARD virtual BitPropVariant getProperty( BitProperty property ) const = 0;

            virtual ~BitGenericItem() = default;
    };
}

#endif //BITGENERICITEM_HPP
