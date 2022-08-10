/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <vector>
#include <system_error>

#include "bitdefines.hpp"
#include "bittypes.hpp"
#include "bitwindows.hpp"

namespace bit7z {
using std::system_error;
using FailedFiles = std::vector< std::pair< tstring, std::error_code > >;

std::error_code make_hresult_code( HRESULT res ) noexcept;

std::error_code last_error_code() noexcept;

/**
 * @brief The BitException class represents a generic exception thrown from the bit7z classes.
 */
class BitException final : public system_error {
    public:
        using native_code_type = HRESULT;

        /**
         * @brief Constructs a BitException object with the given message, and the specific files that failed.
         *
         * @param message   the message associated with the exception object.
         * @param files     the vector of files that failed, with the corresponding error codes.
         * @param code      the HRESULT code associated with the exception object.
         */
        explicit BitException( const char* message, std::error_code code, FailedFiles&& files = {} );

        /**
         * @brief Constructs a BitException object with the given message, and the specific file that failed.
         *
         * @param message   the message associated with the exception object.
         * @param file      the file that failed during the operation.
         * @param code      the HRESULT code associated with the exception object.
         */
        BitException( const char* message, std::error_code code, const tstring& file );

#if !defined(BIT7Z_USE_NATIVE_STRING) && defined(_WIN32)
        BitException( const char* message, std::error_code code, const std::wstring& file );
#endif

        /**
         * @brief Constructs a BitException object with the given message
         *
         * @param message   the message associated with the exception object.
         * @param code      the HRESULT code associated with the exception object.
         */
        explicit BitException( const std::string& message, std::error_code code );

        /**
         * @return the native error code (e.g., HRESULT) corresponding to the exception's std::error_code
         */
        BIT7Z_NODISCARD native_code_type nativeCode() const noexcept;

        /**
         * @return the vector of files that caused the exception to be thrown, along with the corresponding
         *         error codes.
         */
        BIT7Z_NODISCARD const FailedFiles& failedFiles() const noexcept;

    private:
        FailedFiles mFailedFiles;
};
}  // namespace bit7z
#endif // BITEXCEPTION_HPP
