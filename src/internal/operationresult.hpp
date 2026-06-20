/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef OPERATIONRESULT_HPP
#define OPERATIONRESULT_HPP

#include "internal/com.hpp" // To be included before IArchive.h, since on Unix it redefines some of 7-zip's macros.

#include "bitdefines.hpp"

#include <7zip/Archive/IArchive.h>

#include <system_error>
#include <type_traits>

namespace bit7z {

enum struct OperationResult : std::uint8_t {
    Success            = NArchive::NExtract::NOperationResult::kOK,
    UnsupportedMethod  = NArchive::NExtract::NOperationResult::kUnsupportedMethod,
    DataError          = NArchive::NExtract::NOperationResult::kDataError,
    CRCError           = NArchive::NExtract::NOperationResult::kCRCError,
    Unavailable        = NArchive::NExtract::NOperationResult::kUnavailable,
    UnexpectedEnd      = NArchive::NExtract::NOperationResult::kUnexpectedEnd,
    DataAfterEnd       = NArchive::NExtract::NOperationResult::kDataAfterEnd,
    IsNotArc           = NArchive::NExtract::NOperationResult::kIsNotArc,
    HeadersError       = NArchive::NExtract::NOperationResult::kHeadersError,
    WrongPassword      = NArchive::NExtract::NOperationResult::kWrongPassword,
    DataErrorEncrypted = 2 * NArchive::NExtract::NOperationResult::kWrongPassword,
    CRCErrorEncrypted  = ( 2 * NArchive::NExtract::NOperationResult::kWrongPassword ) + 1,
    OpenErrorEncrypted = ( 2 * NArchive::NExtract::NOperationResult::kWrongPassword ) + 2,
    EmptyPassword      = ( 2 * NArchive::NExtract::NOperationResult::kWrongPassword ) + 3,
};

auto make_error_code( OperationResult error ) noexcept -> std::error_code;

} // namespace bit7z

namespace std {
template<>
struct BIT7Z_MAYBE_UNUSED is_error_code_enum< bit7z::OperationResult > : true_type {};
} // namespace std

#endif // OPERATIONRESULT_HPP
