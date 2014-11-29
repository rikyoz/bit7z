#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <iostream>
#include <exception>

#include "Common/MyString.h"

namespace Bit7z {
    class BitException : public std::runtime_error {
        public:
            BitException( const std::string& );
            BitException(UString & );

            const char *what() const;

        private:
            std::string unicodeMessage;
            bool isUnicode;
    };
}

#endif // BITEXCEPTION_HPP
