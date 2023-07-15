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

#include "internal/operationcategory.hpp"

#include "extractcallback.hpp"

namespace bit7z {

auto operation_category_t::name() const noexcept -> const char* {
    return "operation";
}

auto operation_category_t::message( int error_value ) const -> std::string {
    switch ( static_cast< OperationResult >( error_value ) ) {
        case OperationResult::UnsupportedMethod:
            return "Unsupported method.";
        case OperationResult::CRCError:
            return "CRC failed";
        case OperationResult::DataError:
            return "Data error.";
        case OperationResult::UnexpectedEnd:
            return "Reached an unexpected end of data.";
        case OperationResult::WrongPassword:
            return "Wrong password.";
        case OperationResult::DataErrorEncrypted:
            return "Data error in encrypted file (wrong password?).";
        case OperationResult::CRCErrorEncrypted:
            return "CRC error in encrypted file (wrong password?).";
        default:
            return "Unknown error.";
    }
}

auto operation_category_t::default_error_condition( int error_value ) const noexcept -> std::error_condition {
    switch ( static_cast< OperationResult >( error_value ) ) {
        case OperationResult::UnsupportedMethod:
            return std::make_error_condition( std::errc::operation_not_supported );
        case OperationResult::CRCError:
        case OperationResult::DataError:
        case OperationResult::UnexpectedEnd:
        case OperationResult::DataErrorEncrypted:
        case OperationResult::CRCErrorEncrypted:
            return std::make_error_condition( std::errc::io_error );
        case OperationResult::WrongPassword:
            return std::make_error_condition( std::errc::operation_not_permitted );
        default:
            return error_category::default_error_condition( error_value );
    }
}

auto operation_category() noexcept -> const std::error_category& {
    static const operation_category_t instance{};
    return instance;
}

} // namespace bit7z