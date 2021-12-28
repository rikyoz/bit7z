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

#ifndef GENERICINPUTITEM_HPP
#define GENERICINPUTITEM_HPP

#include <cstdint>

#include "bitgenericitem.hpp"
#include "internal/fs.hpp"

struct ISequentialInStream;

namespace bit7z {
    struct GenericInputItem : public BitGenericItem {
        BIT7Z_NODISCARD virtual fs::path inArchivePath() const = 0;

        BIT7Z_NODISCARD virtual HRESULT getStream( ISequentialInStream** inStream ) const = 0;

        BIT7Z_NODISCARD virtual FILETIME creationTime() const = 0;

        BIT7Z_NODISCARD virtual FILETIME lastAccessTime() const = 0;

        BIT7Z_NODISCARD virtual FILETIME lastWriteTime() const = 0;

        BIT7Z_NODISCARD virtual bool hasNewData() const noexcept;

        BIT7Z_NODISCARD BitPropVariant itemProperty( BitProperty propID ) const override;

        ~GenericInputItem() override = default;
    };
}


#endif //GENERICINPUTITEM_HPP
