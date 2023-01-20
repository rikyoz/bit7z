// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/internalcategory.hpp"

#include "biterror.hpp"

using bit7z::internal_category_t;

const char* internal_category_t::name() const noexcept {
    return "bit7z";
}

std::string internal_category_t::message( int error_value ) const noexcept {
    switch ( static_cast< BitError >( error_value ) ) {
        case BitError::Fail:
            return "Unspecified error.";
        case BitError::FilterNotSpecified:
            return "No item filter specified.";
        case BitError::FormatFeatureNotSupported:
            return "Feature not supported by the archive format.";
        case BitError::IndicesNotSpecified:
            return "No indices specified.";
        case BitError::InvalidArchivePath:
            return "Invalid archive path.";
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
        case BitError::ItemIsAFolder:
            return "The item is a folder.";
        case BitError::ItemMarkedAsDeleted:
            return "The item is marked as deleted.";
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

std::error_condition bit7z::internal_category_t::default_error_condition( int error_value ) const noexcept {
    switch ( static_cast< BitError >( error_value ) ) {
        case BitError::FilterNotSpecified:
        case BitError::FormatFeatureNotSupported:
        case BitError::IndicesNotSpecified:
        case BitError::InvalidArchivePath:
        case BitError::InvalidOutputBufferSize:
        case BitError::InvalidCompressionMethod:
        case BitError::InvalidDictionarySize:
        case BitError::InvalidIndex:
        case BitError::InvalidWordSize:
        case BitError::ItemIsAFolder:
        case BitError::NonEmptyOutputBuffer:
            return std::make_error_condition( std::errc::invalid_argument );
        case BitError::NoMatchingItems:
            return std::make_error_condition( std::errc::no_such_file_or_directory );
        case BitError::RequestedWrongVariantType:
        case BitError::UnsupportedOperation:
            return std::make_error_condition( std::errc::operation_not_supported );
        case BitError::ItemMarkedAsDeleted:
        case BitError::WrongUpdateMode:
            return std::make_error_condition( std::errc::operation_not_permitted );
        default:
            return error_category::default_error_condition( error_value );
    }
}

const std::error_category& bit7z::internal_category() noexcept {
    static const internal_category_t instance{};
    return instance;
}