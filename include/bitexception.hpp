#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <iostream>
#include <exception>

namespace bit7z {

    /**
     * @brief The BitException class represents a generic exception thrown from the bit7z classes
     */
    class BitException : public std::runtime_error {
        public:
            /**
             * @brief Constructs a BitException object with the given message
             *
             * @param message   the message associated with the exception object
             */
            BitException( const std::string& message );

            /**
             * @brief Constructs a BitException object with the given message
             *
             * @note the wstring argument is converted into a string and then passed to the base
             * class constructor
             *
             * @param message   the message associated with the exception object
             */
            BitException( const std::wstring& message );
    };

}

#endif // BITEXCEPTION_HPP
