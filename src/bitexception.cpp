#include "../include/bitexception.hpp"

using namespace Bit7z;

BitException::BitException( const std::string&  message ) : std::runtime_error( message ),
    isUnicode( false ) {}

BitException::BitException( UString& message ) : std::runtime_error( "error" ),
    isUnicode( true ) {
    const std::wstring msg( message.GetBuffer( 0 ) );
    unicodeMessage = std::string( msg.begin(), msg.end() );
}

const char* BitException::what() const {
    if ( isUnicode )
        return unicodeMessage.c_str();

    return runtime_error::what();
}
