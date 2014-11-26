#include "../include/bitexception.hpp"

using namespace Bit7z;

BitException::BitException( const std::string&  message ) : std::runtime_error( message ) {}
