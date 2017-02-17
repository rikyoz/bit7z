#include "../include/coutmemstream.hpp"

#include <iterator>
#include <algorithm>
#include <iostream>

using namespace bit7z;

COutMemStream::COutMemStream( vector< byte_t >& out_buffer ) : mBuffer( out_buffer ) {}

COutMemStream::~COutMemStream(){}

STDMETHODIMP COutMemStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != NULL ) {
        *processedSize = 0;
    }
    if ( data == NULL || size == 0 ) {
        return E_FAIL;
    }
    const byte_t* byte_data = static_cast< const byte_t* >( data );
    mBuffer.insert( mBuffer.end(), byte_data, byte_data + size );
    *processedSize = size;
    return S_OK;
}
