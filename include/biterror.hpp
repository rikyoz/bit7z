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
