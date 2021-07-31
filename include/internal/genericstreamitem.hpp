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

#ifndef GENERICSTREAMITEM_HPP
#define GENERICSTREAMITEM_HPP

#include <cstdint>
#include <windows.h>

#ifndef _WIN32
#include <myWindows/StdAfx.h>
#endif

#include <7zip/IStream.h>

#include "bitpropvariant.hpp"
#include "bitgenericitem.hpp"
#include "fs.hpp"

namespace bit7z {
    struct GenericStreamItem : public BitGenericItem {
        virtual fs::path inArchivePath() const = 0;

        virtual HRESULT getStream( ISequentialInStream** inStream ) const = 0;

        virtual bool hasNewData() const noexcept;

        BitPropVariant getProperty( BitProperty propID ) const override;

        ~GenericStreamItem() override = default;
    };
}


#endif //GENERICSTREAMITEM_HPP
