// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "include/bitarchivehandler.hpp"

using namespace bit7z;
using std::wstring;

BitArchiveHandler::BitArchiveHandler( const Bit7zLibrary& lib ) : mLibrary( lib ), mPassword( L"" ) {}

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

void BitArchiveHandler::setTotalCallback( const TotalCallback& callback ) {
    mTotalCallback = callback;
}

void BitArchiveHandler::setProgressCallback( const ProgressCallback& callback ) {
    mProgressCallback = callback;
}

void BitArchiveHandler::setRatioCallback( const RatioCallback& callback ) {
    mRatioCallback = callback;
}

void BitArchiveHandler::setFileCallback( const FileCallback& callback ) {
    mFileCallback = callback;
}

void BitArchiveHandler::setPasswordCallback( const PasswordCallback& callback ) {
    mPasswordCallback = callback;
}
