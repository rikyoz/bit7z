#include <iostream>

#include "CArchiveOpenCallback.h"

using namespace std;

STDMETHODIMP CArchiveOpenCallback::SetTotal( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted( const UInt64* /* files */, const UInt64* /* bytes */ ) {
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword( BSTR* password ) {
    if ( !PasswordIsDefined ) {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        cerr << "Password is not defined" << endl;
        return E_ABORT;
    }

    return StringToBstr( Password, password );
}
