#include "../include/opencallback.hpp"

#include <iostream>

using namespace std;
using namespace Bit7z;

OpenCallback::OpenCallback() : mPassword(L"")/*mPasswordIsDefined( false )*/ {}

STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( mPassword.length() == 0 ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        cerr << "Password is not defined" << endl;
        return E_ABORT;
    }

    return StringToBstr( mPassword.c_str(), password );
}

void OpenCallback::setPassword( const wstring& password ) {
    this->mPassword = password;
    //this->mPasswordIsDefined = password.length() > 0;
}

