#include "../include/coutstdstream.hpp"

#include <iterator>

using namespace bit7z;

COutStdStream::COutStdStream( std::ostream& outputStream ) : mOutputStream( outputStream ) {}

COutStdStream::~COutStdStream() {}

STDMETHODIMP COutStdStream::Write( const void* data, uint32_t size, uint32_t* processedSize ) {
    if ( processedSize ) {
        *processedSize = 0;
    }

    if ( size == 0 ) {
        return S_OK;
    }

    const auto old_pos = mOutputStream.tellp();

    mOutputStream.write( static_cast< const char* >( data ), size );

    if ( processedSize ) {
        *processedSize = static_cast< uint32_t >( mOutputStream.tellp() - old_pos );
    }

    return mOutputStream ? S_OK : E_FAIL;
}

STDMETHODIMP COutStdStream::Seek( int64_t offset, uint32_t seekOrigin, uint64_t* newPosition ) {
    std::ios_base::seekdir way;
    switch ( seekOrigin ) {
        case STREAM_SEEK_SET:
            way = std::ios_base::beg;
            break;
        case STREAM_SEEK_CUR:
            way = std::ios_base::cur;
            break;
        case STREAM_SEEK_END:
            way = std::ios_base::end;
            break;
        default:
            return STG_E_INVALIDFUNCTION;
    }

    if ( offset < 0 ) {
        return HRESULT_WIN32_ERROR_NEGATIVE_SEEK;
    }

    mOutputStream.seekp( static_cast< std::ostream::off_type >( offset ), way );

    if ( newPosition ) {
        *newPosition = mOutputStream.tellp();
    }

    return mOutputStream ? S_OK : E_FAIL;
}

STDMETHODIMP COutStdStream::SetSize( uint64_t newSize ) {
    const auto old_pos = mOutputStream.tellp();
    mOutputStream.seekp( 0, ostream::end );

    const auto diff_pos = newSize - mOutputStream.tellp();
    if ( diff_pos > 0 ) {
        std::fill_n( std::ostream_iterator< char >( mOutputStream ), diff_pos, '\0' );
    }

    mOutputStream.seekp( old_pos );

    return mOutputStream ? S_OK : E_FAIL;
}

