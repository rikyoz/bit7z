#include "../include/callback.hpp"

using namespace Bit7z;

Callback::Callback() : mErrorMessage( L"" ), mPassword( L"" ) {}

void Callback::setPassword( const wstring& password ) {
    mPassword = password;
}

wstring Callback::getErrorMessage() { return mErrorMessage; }
