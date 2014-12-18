#include "../include/callback.hpp"

using namespace bit7z;

Callback::Callback() : mPassword( L"" ), mErrorMessage( L"" ) {}

void Callback::setPassword( const wstring& password ) {
    mPassword = password;
}

bool Callback::isPasswordDefined() const {
    return mPassword.length() != 0;
}

wstring Callback::getErrorMessage() const { return mErrorMessage; }
