// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "../include/coutmemstream.hpp"

#include <iterator>
#include <algorithm>
#include <iostream>

using namespace bit7z;

COutMemStream::COutMemStream( vector< byte_t >& out_buffer ) : mBuffer( out_buffer ) {}

COutMemStream::~COutMemStream() {};

STDMETHODIMP COutMemStream::Write( const void* data, UInt32 size, UInt32* processedSize ) {
    if ( processedSize != nullptr ) {
        *processedSize = 0;
    }
    if ( data == nullptr || size == 0 ) {
        return E_FAIL;
    }
    const auto* byte_data = static_cast< const byte_t* >( data );
    mBuffer.insert( mBuffer.end(), byte_data, byte_data + size );
    if ( processedSize != nullptr ) {
        *processedSize = size;
    }
    return S_OK;
}
