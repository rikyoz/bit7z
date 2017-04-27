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
