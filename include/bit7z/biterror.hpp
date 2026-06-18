/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITERROR_HPP
#define BITERROR_HPP

#include "bitdefines.hpp"

#include <cstdint>
#include <system_error>
#include <type_traits>

namespace bit7z {

/**
 * @brief The BitError enum struct values represent bit7z specific errors.
 */
enum struct BitError : std::uint8_t {
    Fail = 1,                        ///< Unspecified error.
    FilterNotSpecified,              ///< No item filter was specified.
    FormatFeatureNotSupported,       ///< The requested feature is not supported by the archive format.
    IndicesNotSpecified,             ///< No item indices were specified.
    InvalidArchivePath,              ///< Invalid archive path.
    InvalidOutputBufferSize,         ///< Invalid output buffer size.
    InvalidCompressionMethod,        ///< Invalid compression method for the chosen archive format.
    InvalidDictionarySize,           ///< Invalid dictionary size for the chosen compression method.
    InvalidIndex,                    ///< Invalid item index.
    InvalidWordSize,                 ///< Invalid word size for the chosen compression method.
    ItemIsAFolder,                   ///< The item is a folder, but a file was expected.
    ItemMarkedAsDeleted,             ///< The item is marked as deleted.
    NoMatchingItems,                 ///< No matching item was found in the archive.
    NoMatchingFile,                  ///< No matching file was found in the archive.
    NoMatchingSignature,             ///< No known archive signature was found.
    NonEmptyOutputBuffer,            ///< The given output buffer is not empty.
    NullOutputBuffer,                ///< The given output buffer is null.
    RequestedWrongVariantType,       ///< The wrong variant type was requested.
    UnsupportedOperation,            ///< Unsupported operation.
    UnsupportedVariantType,          ///< Unsupported variant type.
    WrongUpdateMode,                 ///< Wrong update mode.
    InvalidZipPassword,              ///< The Zip format only supports printable ASCII characters in passwords.
    InvalidDirectoryPath,            ///< Invalid directory path.
    ItemPathOutsideOutputDirectory,  ///< The extracted item path would be outside the output directory.
    ItemHasAbsolutePath,             ///< The item has an absolute path.
    InvalidItemPath                  ///< The item has an invalid path.
};

/**
 * @brief Creates a std::error_code from the given BitError value.
 *
 * @param error  the BitError value to be converted.
 *
 * @return the std::error_code corresponding to the given BitError value.
 */
auto make_error_code( BitError error ) noexcept -> std::error_code;

/**
 * @brief The BitFailureSource enum struct values represent bit7z error conditions.
 * They can be used for performing queries on bit7z's `error_code`s, for the purpose
 * of grouping, classification, or error translation.
 */
enum struct BitFailureSource : std::uint8_t {
    CRCError,               ///< A CRC check failed.
    DataAfterEnd,           ///< Some data was found after the end of the payload data.
    DataError,              ///< A data error was encountered.
    InvalidArchive,         ///< The input is not a valid archive.
    InvalidArgument,        ///< An invalid argument was provided.
    FormatDetectionError,   ///< The archive format could not be detected.
    HeadersError,           ///< An error was encountered while reading the archive headers.
    NoSuchItem,             ///< The requested item does not exist.
    OperationNotSupported,  ///< The requested operation is not supported.
    OperationNotPermitted,  ///< The requested operation is not permitted.
    UnavailableData,        ///< The requested data is unavailable.
    UnexpectedEnd,          ///< An unexpected end of data was reached.
    WrongPassword           ///< A wrong password was provided.
};

/**
 * @brief Creates a std::error_condition from the given BitFailureSource value.
 *
 * @param failureSource  the BitFailureSource value to be converted.
 *
 * @return the std::error_condition corresponding to the given BitFailureSource value.
 */
auto make_error_condition( BitFailureSource failureSource ) noexcept -> std::error_condition;

} // namespace bit7z

namespace std {
template<>
struct BIT7Z_MAYBE_UNUSED is_error_code_enum< bit7z::BitError > : std::true_type {};

template<>
struct BIT7Z_MAYBE_UNUSED is_error_condition_enum< bit7z::BitFailureSource > : std::true_type {};
} // namespace std

#endif //BITERROR_HPP
