// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitexception.hpp"

using namespace bit7z;

BitException::BitException( const std::string &message ) : std::runtime_error( message ) {}

BitException::BitException( const std::wstring &message )
    : std::runtime_error( std::string( message.begin(), message.end() ) ) {}
