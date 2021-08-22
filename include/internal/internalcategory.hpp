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

#ifndef INTERNALCATEGORY_HPP
#define INTERNALCATEGORY_HPP

#include <system_error>
#include <string>

#include "bittypes.hpp"

namespace bit7z {

    struct internal_category_t : public std::error_category {
        BIT7Z_NODISCARD const char* name() const noexcept override;

        BIT7Z_NODISCARD std::string message( int ev ) const noexcept override;

        BIT7Z_NODISCARD std::error_condition default_error_condition( int ev ) const noexcept override;
    };

    std::error_category& internal_category() noexcept;

}

#endif //INTERNALCATEGORY_HPP
