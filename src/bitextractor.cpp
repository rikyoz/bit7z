// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2018  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitextractor.hpp"

#include <algorithm>

#include "7zip/Archive/IArchive.h"

#include "../include/bitpropvariant.hpp"
#include "../include/bitexception.hpp"
#include "../include/extractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/fsutil.hpp"
#include "../include/util.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;
using namespace bit7z::util;
using namespace NWindows;
using namespace NArchive;

using std::wstring;

BitExtractor::BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format ) : BitArchiveOpener( lib, format ) {}

void BitExtractor::extract( const wstring& in_file, const wstring& out_dir ) const {
    extractItems( in_file, vector< uint32_t >(), out_dir );
}

//TODO: Improve reusing of code (both extract methods do the same operations when opening an archive)

/* Most of this code, though heavily modified, is taken from the main() of Client7z.cpp in the 7z SDK
 * Main changes made:
 *  + Generalized the code to work with any type of format (the original works only with 7z format)
 *  + Use of exceptions instead of error codes */
void BitExtractor::extractMatching( const wstring& in_file, const wstring& item_filter, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_file );

    vector<uint32_t> matched_indices;
    // TODO: Use BitArchiveReader here
    if ( !item_filter.empty() ) {
        //Searching for files inside the archive that match the given filter
        uint32_t items_count;
        HRESULT result = in_archive->GetNumberOfItems( &items_count );
        if ( result == S_OK ) {
            for ( uint32_t index = 0; index < items_count; ++index ) {
                BitPropVariant propvar;
                result = in_archive->GetProperty( index, kpidPath, &propvar );
                if ( result == S_OK && !propvar.isEmpty() && propvar.type() == BitPropVariantType::String &&
                        fsutil::wildcard_match( item_filter, propvar.getString() ) ) {
                    matched_indices.push_back( index );
                }
            }
        }
    }

    if ( !matched_indices.empty() ) {
        extractToFileSystem( in_archive, in_file, out_dir, matched_indices );
    }
}

void BitExtractor::extractItems( const wstring& in_file, const vector<uint32_t>& indices, const wstring& out_dir ) const {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_file );

    uint32_t number_items;
    in_archive->GetNumberOfItems( &number_items );
    if ( std::any_of( indices.begin(), indices.end(), [&]( uint32_t index ) { return index >= number_items; }) ) {
        /* if any of the indices is greater than the number of items in the archive we throw an exception, since it is
           an invalid index! */
        throw BitException( "Some index is not valid" );
    }

    extractToFileSystem( in_archive, in_file, out_dir, indices );
}

void BitExtractor::extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index ) {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_file );

    uint32_t number_items;
    in_archive->GetNumberOfItems( &number_items );
    if ( index >= number_items ) {
        throw BitException( "Index " + std::to_string( index ) + " is out of range"  );
    }

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_buffer );

    const uint32_t indices[] = { index };

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    if ( in_archive->Extract( indices, 1, NExtract::NAskMode::kExtract, extract_callback ) != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() );
    }
}

void BitExtractor::test( const wstring& in_file ) {
    CMyComPtr< IInArchive > in_archive = openArchive( *this, mFormat, in_file );

    auto* extract_callback_spec = new ExtractCallback( *this, in_archive, in_file, L"" );

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive->Extract( nullptr, static_cast< uint32_t >( -1 ), NExtract::NAskMode::kTest, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() + L" (error code: " + std::to_wstring( res ) + L")" );
    }
}


void BitExtractor::extractToFileSystem( IInArchive* in_archive, const wstring& in_file,
                                        const wstring& out_dir, const vector<uint32_t>& indices ) const {

    //pointer to an array of the indices of the files to be extracted
    const uint32_t* item_indices = indices.empty() ? nullptr : indices.data();
    uint32_t num_items = indices.empty() ? static_cast< uint32_t >( -1 ) :
                         static_cast< uint32_t >( indices.size() );

    auto* extract_callback_spec = new ExtractCallback( *this, in_archive, in_file, out_dir );

    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive->Extract( item_indices, num_items, NExtract::NAskMode::kExtract, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() + L" (error code: " + std::to_wstring( res ) + L")" );
    }
}
