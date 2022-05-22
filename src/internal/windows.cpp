// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * Bit7z is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bit7z; if not, see https://www.gnu.org/licenses/.
 */

#ifndef _WIN32

#include "bittypes.hpp"
#include "internal/windows.hpp"

#include <iostream>
#include <cstring>
#include <limits>


size_t wcsnlen_s( const wchar_t* str, size_t max_size ) {
    if ( str == nullptr || max_size == 0 ) {
        return 0;
    }

    size_t result = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (; result < max_size && str[ result ] != L'\0'; ++result ) {
        // continue;
    }
    return result;
}

BSTR SysAllocString( const OLECHAR* str ) {
    if ( str == nullptr ) {
        return nullptr;
    }

    auto len = static_cast< UINT >( wcsnlen_s( str, FILENAME_MAX ) );
    return SysAllocStringLen( str, len );
}

using bstr_prefix_t = uint32_t;

/* Internal implementation of SysAllocStringByteLen for Unix systems.
 *
 * Notes:
 *   - We use C allocation functions instead of "new" since we must be able to also free BSTR objects
 *     allocated by 7-zip (which uses malloc). Never mix new/delete and malloc/free.
 *   - We use calloc instead of malloc, so that we do not have to manually add the termination character at the end.
 *   - The length parameter is an uint64_t, instead of the UINT parameter used in the WinAPI interface.
 *     This allows to avoid unsigned integer wrap around in SysAllocStringLen.
 * */
BSTR AllocStringBuffer( LPCSTR str, uint64_t byte_length ) {
    // Maximum value that can be stored in the BSTR byte_length prefix.
    constexpr auto max_prefix_value = std::numeric_limits< bstr_prefix_t >::max();

    // Max number of bytes that can be stored in the BSTR (excluding the termination char, and the byte_length prefix).
    constexpr auto max_length = max_prefix_value - sizeof( OLECHAR ) - sizeof( bstr_prefix_t );

    if ( byte_length >= max_length ) { // Invalid byte_length parameter
        return nullptr;
    }

    // Length prefix (32 bits, as in Microsoft specs) + string bytes + termination character (32 bits on Linux).
    auto buffer_size = sizeof( bstr_prefix_t ) + byte_length + sizeof( OLECHAR );

    // Allocating memory for storing the BSTR as a byte array.
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    auto* bstr_buffer = static_cast< byte_t* >( std::calloc( buffer_size, sizeof( byte_t ) ) );

    if ( bstr_buffer == nullptr ) { // Failed to allocate memory for the BSTR buffer.
        return nullptr;
    }

    // Storing the number of bytes of the BSTR as a prefix of it.
    *reinterpret_cast< bstr_prefix_t* >( bstr_buffer ) = byte_length;

    // The actual BSTR must point after the byte_length prefix.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BSTR result = reinterpret_cast< BSTR >( bstr_buffer + sizeof( bstr_prefix_t ) );
    if ( str != nullptr ) {
        // Copying byte-by-byte the input string to the BSTR.
        std::memcpy( result, str, byte_length );
    }
    return result;
}

BSTR SysAllocStringLen( const OLECHAR* str, UINT length ) {
    auto byte_length = static_cast< uint64_t >( length ) * sizeof( OLECHAR );
    return AllocStringBuffer( reinterpret_cast< LPCSTR >( str ), byte_length );
}

BSTR SysAllocStringByteLen( LPCSTR str, UINT length ) {
    return AllocStringBuffer( str, length );
}

void SysFreeString( BSTR bstrString ) { // NOLINT(readability-non-const-parameter)
    if ( bstrString != nullptr ) {
        // We must delete the original memory buffer, which starts from the BSTR length prefix
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto* bstr_buffer = reinterpret_cast< byte_t* >( bstrString ) - sizeof( bstr_prefix_t );

        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        std::free( static_cast< void* >( bstr_buffer ) );
    }
}

UINT SysStringByteLen( BSTR bstrString ) { // NOLINT(readability-non-const-parameter)
    if ( bstrString == nullptr ) {
        return 0;
    }
    // If the string is non-null, we return the value stored in the length prefix of the BSTR.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return *( reinterpret_cast< const bstr_prefix_t* >( bstrString ) - 1 );
}

UINT SysStringLen( BSTR bstrString ) { // NOLINT(readability-non-const-parameter)
    // Same as SysStringByteLen, but we count how many OLECHARs are stored in the BSTR.
    return SysStringByteLen( bstrString ) / sizeof( OLECHAR );
}

#endif