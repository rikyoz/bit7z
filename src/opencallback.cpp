#include "../include/opencallback.hpp"

#include <iostream>

using namespace std;
using namespace bit7z;

/* Most of this code is taken from the COpenCallback class in Client7z.cpp of the 7z SDK
 * Main changes made:
 *  + Use of wstring instead of UString (see Callback base interface)
 *  + Error messages are not showed (see comments in ExtractCallback) */

OpenCallback::OpenCallback() {}
OpenCallback::~OpenCallback() {}

STDMETHODIMP OpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP OpenCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( !isPasswordDefined() ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        mErrorMessage = L"Password is not defined";
        return E_ABORT;
    }

    return StringToBstr( mPassword.c_str(), password );
}
