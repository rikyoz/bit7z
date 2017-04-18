#include "include/bitarchivecreator.hpp"

using std::wstring;
using namespace bit7z;

BitArchiveCreator::BitArchiveCreator( const Bit7zLibrary& lib, const BitInOutFormat& format ) :
    BitArchiveHandler( lib ),
    mFormat( format ),
    mCompressionLevel( NORMAL ),
    mCryptHeaders( false ),
    mSolidMode( false ),
    mVolumeSize( 0 ) {}

BitArchiveCreator::~BitArchiveCreator() {}

const BitInOutFormat& BitArchiveCreator::compressionFormat() {
    return mFormat;
}

bool BitArchiveCreator::cryptHeaders() const {
    return mCryptHeaders;
}

BitCompressionLevel BitArchiveCreator::compressionLevel() const {
    return mCompressionLevel;
}

bool BitArchiveCreator::solidMode() const {
    return mSolidMode;
}

uint64_t BitArchiveCreator::volumeSize() const {
    return mVolumeSize;
}

void BitArchiveCreator::setPassword( const wstring &password ) {
    setPassword( password, mCryptHeaders );
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
