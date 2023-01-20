/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef INTERNALCATEGORY_HPP
#define INTERNALCATEGORY_HPP

#include <system_error>
#include <string>

#include "bitdefines.hpp"

namespace bit7z {

struct internal_category_t final : public std::error_category {
    BIT7Z_NODISCARD const char* name() const noexcept override;

    BIT7Z_NODISCARD std::string message( int error_value ) const noexcept override;

    BIT7Z_NODISCARD std::error_condition default_error_condition( int error_value ) const noexcept override;
};

const std::error_category& internal_category() noexcept;

}  // namespace bit7z

#endif //INTERNALCATEGORY_HPP
