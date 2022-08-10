/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITERROR_HPP
#define BITERROR_HPP

#include <system_error>

#include "bitdefines.hpp"

namespace bit7z {
/**
 * @brief The BitError enum struct values represent bit7z specific errors.
 */
enum struct BitError {
    Fail = 1,
    FilterNotSpecified,
    FormatFeatureNotSupported,
    IndicesNotSpecified,
    InvalidArchivePath,
    InvalidOutputBufferSize,
    InvalidCompressionMethod,
    InvalidDictionarySize,
    InvalidIndex,
    InvalidWordSize,
    ItemIsAFolder,
    ItemMarkedAsDeleted,
    NoMatchingExtension,
    NoMatchingItems,
    NoMatchingSignature,
    NonEmptyOutputBuffer,
    RequestedWrongVariantType,
    UnsupportedOperation,
    WrongUpdateMode
};

std::error_code make_error_code( const BitError& e );
}  // namespace bit7z

namespace std {
template<>
struct BIT7Z_MAYBE_UNUSED is_error_code_enum< bit7z::BitError > : public true_type {};
}


#endif //BITERROR_HPP
