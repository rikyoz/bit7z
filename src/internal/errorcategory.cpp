// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "internal/errorcategory.hpp"

#include "biterror.hpp"

using bit7z::error_category_t;

const char* error_category_t::name() const noexcept {
    return "bit7z";
}

std::string error_category_t::message( int ev ) const noexcept {
    switch ( static_cast< BitError >( ev ) ) {
        case BitError::Fail:
            return "Unspecified error.";
        case BitError::FilterNotSpecified:
            return "Item filter not specified.";
        case BitError::FormatFeatureNotSupported:
            return "Feature not supported by the archive format.";
        case BitError::IndicesNotSpecified:
            return "No indices specified.";
        case BitError::InvalidArchivePath:
            return "No matching file was found in the archive.";
        case BitError::InvalidOutputBufferSize:
            return "Invalid output buffer size.";
        case BitError::InvalidCompressionMethod:
            return "Invalid compression method for the chosen archive format.";
        case BitError::InvalidDictionarySize:
            return "Invalid dictionary size for the chosen compression method.";
        case BitError::InvalidIndex:
            return "Invalid index.";
        case BitError::InvalidWordSize:
            return "Invalid word size for the chosen compression method.";
        case BitError::NoMatchingExtension:
            return "No known extension found.";
        case BitError::NoMatchingItems:
            return "No matching item was found in the archive.";
        case BitError::NoMatchingSignature:
            return "No known signature found.";
        case BitError::NonEmptyOutputBuffer:
            return "Output buffer is not empty.";
        case BitError::RequestedWrongVariantType:
            return "Requested wrong variant type.";
        case BitError::UnsupportedOperation:
            return "Unsupported operation.";
        case BitError::WrongUpdateMode:
            return "Wrong update mode.";
        default:
            return "Unknown error.";
    }
}

std::error_condition bit7z::error_category_t::default_error_condition( int ev ) const noexcept {
    switch ( static_cast< BitError >( ev ) ) {
        case BitError::FilterNotSpecified:
        case BitError::FormatFeatureNotSupported:
        case BitError::IndicesNotSpecified:
        case BitError::InvalidArchivePath:
        case BitError::InvalidOutputBufferSize:
        case BitError::InvalidCompressionMethod:
        case BitError::InvalidDictionarySize:
        case BitError::InvalidIndex:
        case BitError::InvalidWordSize:
        case BitError::NonEmptyOutputBuffer:
            return std::make_error_condition( std::errc::invalid_argument );
        case BitError::NoMatchingItems:
            return std::make_error_condition( std::errc::no_such_file_or_directory );
        case BitError::RequestedWrongVariantType:
        case BitError::UnsupportedOperation:
            return std::make_error_condition( std::errc::operation_not_supported );
        default:
            return error_category::default_error_condition( ev );
    }
}

std::error_category& bit7z::error_category() noexcept {
    static error_category_t instance{};
    return instance;
}