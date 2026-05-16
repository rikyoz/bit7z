/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef WIN32CATEGORY_HPP
#define WIN32CATEGORY_HPP

#ifdef __MINGW32__

#include "bitdefines.hpp"

#include <system_error>
#include <string>

namespace bit7z {

struct Win32Category final : std::error_category {
    BIT7Z_NODISCARD auto name() const noexcept -> const char* override;

    BIT7Z_NODISCARD auto message( int errorValue ) const -> std::string override;

    BIT7Z_NODISCARD auto default_error_condition( int errorValue ) const noexcept -> std::error_condition override;
};

auto win32Category() noexcept -> const std::error_category&;

} // namespace bit7z

#endif // __MINGW32__

#endif // WIN32CATEGORY_HPP
