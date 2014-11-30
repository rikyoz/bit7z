#include "../include/opencallback.hpp"

#include <iostream>

using namespace std;
using namespace Bit7z;

OpenCallback::OpenCallback() : passwordIsDefined( false ) {}

HRESULT OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

HRESULT OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

HRESULT OpenCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( !passwordIsDefined ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        cerr << "Password is not defined" << endl;
        return E_ABORT;
    }

    return StringToBstr( this->password, password );
}

void OpenCallback::setPassword( const UString& password ) {
    this->password = password;
    this->passwordIsDefined = password.Length() > 0;
}

