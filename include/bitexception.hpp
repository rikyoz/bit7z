/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <vector>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#else
#include <myWindows/StdAfx.h>
#endif

#include "../include/bittypes.hpp"

namespace bit7z {
    using std::system_error;
    using FailedFiles = std::vector< std::pair< tstring, std::error_code > >;

    std::error_code make_hresult_code( HRESULT res ) noexcept;

    inline std::error_code last_error_code() noexcept {
        return std::error_code{ static_cast< int >( GetLastError() ), std::system_category() };
    }

    /**
     * @brief The BitException class represents a generic exception thrown from the bit7z classes.
     */
    class BitException : public system_error {
        public:
#ifdef _WIN32
            using native_code_type = HRESULT;
#else
            using native_code_type = int;
#endif

            /**
             * @brief Constructs a BitException object with the given message and the specific files that failed.
             *
             * @param message   the message associated with the exception object.
             * @param files     the vector of files that failed, with the corresponding error codes.
             * @param code      the HRESULT code associated with the exception object.
             */
            explicit BitException( const char* const message,
                                   std::error_code code = make_hresult_code( E_FAIL ),
                                   FailedFiles&& files = {} );

            /**
             * @brief Constructs a BitException object with the given message and the specific file that failed.
             *
             * @param message   the message associated with the exception object.
             * @param file      the file that failed during the operation.
             * @param code      the HRESULT code associated with the exception object.
             */
            BitException( const char* const message, std::error_code code, const tstring& file );

            /**
             * @brief Constructs a BitException object with the given message
             *
             * @param message   the message associated with the exception object.
             * @param code      the HRESULT code associated with the exception object.
             */
            explicit BitException( const std::string& message, std::error_code code = make_hresult_code( E_FAIL ) );

            native_code_type nativeCode() const;

            const FailedFiles& failedFiles() const;

        private:
            //HRESULT mErrorCode;
            FailedFiles mFailedFiles;
    };
}
#endif // BITEXCEPTION_HPP
