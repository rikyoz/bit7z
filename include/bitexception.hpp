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
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#else
#include <myWindows/StdAfx.h>
#endif

#include "../include/bittypes.hpp"

namespace bit7z {
    using std::runtime_error;
    using FailedFiles = std::vector< std::pair< tstring, HRESULT > >;

    /**
     * @brief The BitException class represents a generic exception thrown from the bit7z classes.
     */
    class BitException : public runtime_error {
        public:
            /**
             * @brief Constructs a BitException object with the given message.
             *
             * @param message   the message associated with the exception object.
             * @param code      the HRESULT code associated with the exception object.
             */
            explicit BitException( const char* const message, HRESULT code = E_FAIL );

            /**
             * @brief Constructs a BitException object with the given message and the specific files that failed.
             *
             * @param message   the message associated with the exception object.
             * @param files     the vector of files that failed, with the corresponding error codes.
             * @param code      the HRESULT code associated with the exception object.
             */
            BitException( const char* const message, FailedFiles&& files, HRESULT code = E_FAIL );


            /**
             * @brief Constructs a BitException object with the given message and the specific file that failed.
             *
             * @param message   the message associated with the exception object.
             * @param file      the file that failed during the operation.
             * @param code      the HRESULT code associated with the exception object.
             */
            BitException( const char* const message, const tstring& file, HRESULT code = E_FAIL );

            /**
             * @brief Constructs a BitException object with the given message.
             *
             * @param message   the message associated with the exception object.
             * @param code      the HRESULT code associated with the exception object.
             */
            explicit BitException( const std::string& message, HRESULT code = E_FAIL );

            /**
             * @return the HRESULT code associated with the exception object.
             */
            HRESULT getErrorCode() const;

            const FailedFiles& getFailedFiles() const;

        private:
            HRESULT mErrorCode;
            FailedFiles mFailedFiles;
    };
}
#endif // BITEXCEPTION_HPP
