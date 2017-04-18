#include "include/bitarchiveopener.hpp"

using namespace bit7z;

BitArchiveOpener::BitArchiveOpener( const Bit7zLibrary& lib, const BitInFormat& format )
    : BitArchiveHandler( lib ), mFormat( format ) {}

BitArchiveOpener::~BitArchiveOpener() {}

const BitInFormat& BitArchiveOpener::extractionFormat() {
    return mFormat;
}
