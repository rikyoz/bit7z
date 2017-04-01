#include "include/bitarchiveopener.hpp"

using namespace bit7z;

BitArchiveOpener::BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format )
    : mLibrary( lib ), mFormat( format ), mPassword( L"" ) {}

BitArchiveOpener::~BitArchiveOpener() {}

const BitInFormat& BitArchiveOpener::extractionFormat() {
    return mFormat;
}

void BitArchiveOpener::setPassword( const wstring &password ) {
    mPassword = password;
}
