#include "../include/bitexception.hpp"

using namespace bit7z;

BitException::BitException( const std::string &message ) : std::runtime_error( message ) {}

BitException::BitException( const std::wstring &message )
    : std::runtime_error( std::string( message.begin(), message.end() ) ) {}
