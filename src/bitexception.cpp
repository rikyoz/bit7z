// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/bitexception.hpp"

using namespace bit7z;

BitException::BitException( const string& message ) : runtime_error( message ) {}

BitException::BitException( const wstring& message )
    : runtime_error( string( message.begin(), message.end() ) ) {}
/* NOTE: this doesn't convert the character set of the original wstring message!
 * But we expect every error message to be ASCII (apart from those containing filenames)!
 * This will need to be fixed in the future */
