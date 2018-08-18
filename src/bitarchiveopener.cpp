// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "include/bitarchiveopener.hpp"

using namespace bit7z;

BitArchiveOpener::BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveHandler( lib ), mFormat( format ) {}

BitArchiveOpener::~BitArchiveOpener() {}

const BitInFormat& BitArchiveOpener::extractionFormat() {
    return mFormat;
}
