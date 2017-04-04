#include "include/bitarchiveopener.hpp"

using std::wstring;
using namespace bit7z;

BitArchiveOpener::BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format )
    : mLibrary( lib ), mFormat( format ), mPassword( L"" ) {}

BitArchiveOpener::~BitArchiveOpener() {}

const BitInFormat& BitArchiveOpener::extractionFormat() {
    return mFormat;
}

const wstring BitArchiveOpener::password() {
    return mPassword;
}

const bool BitArchiveOpener::isPasswordDefined() {
    return mPassword.size() != 0;
}

void BitArchiveOpener::setPassword( const wstring& password ) {
    mPassword = password;
}
