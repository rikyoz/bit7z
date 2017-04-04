#include "include/bitarchivehandler.hpp"

using namespace bit7z;

BitArchiveHandler::BitArchiveHandler() {}

std::function<void ( uint64_t )> BitArchiveHandler::totalCallback() const {
    return mTotalCallback;
}

std::function<void ( uint64_t )> BitArchiveHandler::progressCallback() const {
    return mProgressCallback;
}

BitArchiveHandler::~BitArchiveHandler() {}

void BitArchiveHandler::setTotalCallback( std::function<void ( uint64_t )> callback ) {
    mTotalCallback = callback;
}

void BitArchiveHandler::setProgressCallback( std::function<void ( uint64_t )> callback ) {
    mProgressCallback = callback;
}
