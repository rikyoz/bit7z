#ifndef BITEXCEPTION_HPP
#define BITEXCEPTION_HPP

#include <iostream>
#include <exception>

namespace Bit7z {
    class BitException : public std::runtime_error {
        public:
            BitException( const std::string& );
    };
}

#endif // BITEXCEPTION_HPP
