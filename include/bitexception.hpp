#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <iostream>
#include <exception>

namespace bit7z {

    class BitException : public std::runtime_error {
        public:
            BitException( const std::string& message );
            BitException( const std::wstring& message );

            const char* what() const;

        private:
            std::string mUnicodeMessage;
    };

}

#endif // BITEXCEPTION_HPP
