#include "include/bitarchivecreator.hpp"

using std::wstring;
using namespace bit7z;

BitArchiveCreator::BitArchiveCreator( const Bit7zLibrary& lib, const BitInOutFormat& format ) :
    mLibrary( lib ),
    mFormat( format ),
    mCompressionLevel( NORMAL ),
    mPassword( L"" ),
    mCryptHeaders( false ),
    mSolidMode( false ),
    mVolumeSize( 0 ) {}

BitArchiveCreator::~BitArchiveCreator() {}

const BitInOutFormat& BitArchiveCreator::compressionFormat() {
    return mFormat;
}

const wstring BitArchiveCreator::password() {
    return mPassword;
}

const bool BitArchiveCreator::isPasswordDefined() {
    return mPassword.size() != 0;
}

const bool BitArchiveCreator::cryptHeaders() {
    return mCryptHeaders;
}

const BitCompressionLevel BitArchiveCreator::compressionLevel() {
    return mCompressionLevel;
}

const bool BitArchiveCreator::solidMode() {
    return mSolidMode;
}

const uint64_t BitArchiveCreator::volumeSize() {
    return mVolumeSize;
}

void BitArchiveCreator::setPassword( const wstring& password, bool crypt_headers ) {
    mPassword = password;
    mCryptHeaders = ( password.length() > 0 ) && crypt_headers;
}

void BitArchiveCreator::setCompressionLevel( BitCompressionLevel compression_level ) {
    mCompressionLevel = compression_level;
}

void BitArchiveCreator::setSolidMode( bool solid_mode ) {
    mSolidMode = solid_mode;
}

void BitArchiveCreator::setVolumeSize( uint64_t size ) {
    mVolumeSize = size;
}
