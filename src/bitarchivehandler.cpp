#include "include/bitarchivehandler.hpp"

using namespace bit7z;

BitArchiveHandler::BitArchiveHandler() {}

std::function<void ( uint64_t )> BitArchiveHandler::totalCallback() const {
    return mTotalCallback;
}

std::function<void ( uint64_t )> BitArchiveHandler::progressCallback() const {
    return mProgressCallback;
}

std::function<void (uint64_t, uint64_t)> BitArchiveHandler::ratioCallback() const {
    return mRatioCallback;
}

std::function<void (std::wstring)> BitArchiveHandler::fileCallback() const {
    return mFileCallback;
}

BitArchiveHandler::~BitArchiveHandler() {}

void BitArchiveHandler::setTotalCallback( std::function<void ( uint64_t )> callback ) {
    mTotalCallback = callback;
}

void BitArchiveHandler::setProgressCallback( std::function<void ( uint64_t )> callback ) {
    mProgressCallback = callback;
}

void BitArchiveHandler::setRatioCallback( std::function<void (uint64_t, uint64_t)> callback ) {
    mRatioCallback = callback;
}

void BitArchiveHandler::setFileCallback( std::function<void (std::wstring)> callback ) {
    mFileCallback = callback;
}
