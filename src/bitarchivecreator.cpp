#include "include/bitarchivecreator.hpp"

using namespace bit7z;

BitArchiveCreator::BitArchiveCreator( const Bit7zLibrary &lib, const BitInOutFormat &format ) :
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
