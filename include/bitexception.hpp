#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <iostream>
#include <exception>

#include "Common/MyString.h"

namespace Bit7z {
    class BitException : public std::runtime_error {
        public:
            BitException( const std::string& message );
            BitException( const std::wstring& message );
            //BitException( UString& message );

            const char *what() const;

        private:
            std::string mUnicodeMessage;
    };
}

#endif // BITEXCEPTION_HPP
