/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include <iostream>
#include <exception>

namespace bit7z {
    using std::runtime_error;
    using std::string;
    using std::wstring;

    /**
     * @brief The BitException class represents a generic exception thrown from the bit7z classes.
     */
    class BitException : public runtime_error {
        public:
            /**
             * @brief Constructs a BitException object with the given message.
             *
             * @param message   the message associated with the exception object.
             */
            explicit BitException( const string& message );

            /**
             * @brief Constructs a BitException object with the given message.
             *
             * @note The wstring argument is converted into a string and then passed to the base
             * class constructor.
             *
             * @param message   the message associated with the exception object.
             */
            explicit BitException( const wstring& message );
    };
}
#endif // BITEXCEPTION_HPP
