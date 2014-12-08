#include "../include/bitexception.hpp"

using namespace Bit7z;

BitException::BitException( const std::string&  message ) : std::runtime_error( message ),
    mUnicodeMessage( "" ) {}

BitException::BitException( const std::wstring& message ) : std::runtime_error( std::string(
                message.begin(), message.end() ) ) {}

const char* BitException::what() const {
    if ( mUnicodeMessage.size() > 0 )
        return mUnicodeMessage.c_str();

    return runtime_error::what();
}
