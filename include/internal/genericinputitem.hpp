/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
}  // namespace bit7z


#endif //GENERICINPUTITEM_HPP
