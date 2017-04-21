#include "include/bitarchivehandler.hpp"

using namespace bit7z;
using std::wstring;

BitArchiveHandler::BitArchiveHandler( const Bit7zLibrary &lib ) : mLibrary( lib ), mPassword(L"") {}

BitArchiveHandler::~BitArchiveHandler() {}

const wstring BitArchiveHandler::password() const {
    return mPassword;
}

bool BitArchiveHandler::isPasswordDefined() const {
    return !mPassword.empty();
}

TotalCallback BitArchiveHandler::totalCallback() const {
    return mTotalCallback;
}

ProgressCallback BitArchiveHandler::progressCallback() const {
    return mProgressCallback;
}

RatioCallback BitArchiveHandler::ratioCallback() const {
    return mRatioCallback;
}

FileCallback BitArchiveHandler::fileCallback() const {
    return mFileCallback;
}

PasswordCallback BitArchiveHandler::passwordCallback() const {
    return mPasswordCallback;
}

void BitArchiveHandler::setPassword( const wstring& password ) {
    mPassword = password;
}

void BitArchiveHandler::clearPassword() {
    setPassword( L"" );
}

void BitArchiveHandler::setTotalCallback( TotalCallback callback ) {
    mTotalCallback = callback;
}

void BitArchiveHandler::setProgressCallback( ProgressCallback callback ) {
    mProgressCallback = callback;
}

void BitArchiveHandler::setRatioCallback( RatioCallback callback ) {
    mRatioCallback = callback;
}

void BitArchiveHandler::setFileCallback( FileCallback callback ) {
    mFileCallback = callback;
}

void BitArchiveHandler::setPasswordCallback( PasswordCallback callback ) {
    mPasswordCallback = callback;
}
