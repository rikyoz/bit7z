// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2026 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/opencategory.hpp"

#include "internal/openerror.hpp"

#include <system_error>
#include <string>

namespace bit7z {

auto OpenCategory::name() const noexcept -> const char* {
    return "open";
}

auto OpenCategory::message( int errorValue ) const -> std::string {
    switch ( static_cast< OpenError >( errorValue ) ) {
        case OpenError::IsNotArc:
            return "Invalid archive, or wrong format used.";
        case OpenError::HeadersError:
            return "Headers error.";
        case OpenError::EncryptedHeadersError:
            return "Encrypted headers error.";
        case OpenError::UnavailableStart:
            return "Unavailable start of archive.";
        case OpenError::UnconfirmedStart:
            return "Unconfirmed start of archive.";
        case OpenError::UnexpectedEnd:
            return "Reached an unexpected end of the archive.";
        case OpenError::DataAfterEnd:
            return "There are some data after the end of the archive.";
        case OpenError::UnsupportedMethod:
            return "Unsupported method.";
        case OpenError::UnsupportedFeature:
            return "Unsupported feature.";
        case OpenError::DataError:
            return "Data error.";
        case OpenError::CRCError:
            return "CRC failed.";
        default:
            return "Unknown operation error (code " + std::to_string( errorValue ) + ").";
    }
}

auto OpenCategory::default_error_condition( int errorValue ) const noexcept -> std::error_condition {
    switch ( static_cast< OpenError >( errorValue ) ) {
        case OpenError::UnsupportedMethod:
        case OpenError::UnsupportedFeature:
            return std::make_error_condition( std::errc::function_not_supported );
        case OpenError::IsNotArc:
        case OpenError::HeadersError:
        case OpenError::UnavailableStart:
        case OpenError::UnconfirmedStart:
        case OpenError::UnexpectedEnd:
        case OpenError::DataAfterEnd:
        case OpenError::DataError:
        case OpenError::CRCError:
            return std::make_error_condition( std::errc::io_error );
        case OpenError::EncryptedHeadersError:
            return std::make_error_condition( std::errc::operation_not_permitted );
        default:
            return error_category::default_error_condition( errorValue );
    }
}

auto open_category() noexcept -> const std::error_category& {
    static const OpenCategory instance{};
    return instance;
}

} // namespace bit7z