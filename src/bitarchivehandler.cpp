// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#include "../include/bitarchivehandler.hpp"

using namespace bit7z;
using std::wstring;

BitArchiveHandler::BitArchiveHandler( const Bit7zLibrary& lib, const wstring& password ) : mLibrary( lib ), mPassword( password ) {}

const Bit7zLibrary& BitArchiveHandler::library() const {
    return mLibrary;
}

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
