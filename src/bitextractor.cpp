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
#include <regex>

#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/fileextractcallback.hpp"
#include "../include/memextractcallback.hpp"
#include "../include/streamextractcallback.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

using std::wstring;
using std::wregex;

CONSTEXPR auto kNoMatchingFile = "No matching file was found in the archive";

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
    if ( item_filter.empty() ) {
        throw BitException( "Empty wildcard filter", E_INVALIDARG );
    }

    extractMatchingFilter( in_file, out_dir, [ &item_filter ]( const wstring& item_path ) -> bool {
        return fsutil::wildcardMatch( item_filter, item_path );
    });
}

void BitExtractor::extractMatchingRegex( const wstring& in_file, const wstring& regex, const wstring& out_dir ) const {
    if ( regex.empty() ) {
        throw BitException( "Empty regex filter", E_INVALIDARG );
    }

    const wregex regex_filter( regex, std::regex::ECMAScript | std::regex::optimize );
    extractMatchingFilter( in_file, out_dir, [ &regex_filter ]( const wstring& item_path ) -> bool {
        return std::regex_match( item_path, regex_filter );
    });
}

void BitExtractor::extractMatchingFilter( const wstring& in_file,
                                          const wstring& out_dir,
                                          function< bool( const wstring& ) > filter ) const {
    BitInputArchive in_archive( *this, in_file );

    vector< uint32_t > matched_indices;
    //Searching for files inside the archive that match the given regex filter
    uint32_t items_count = in_archive.itemsCount();
    for ( uint32_t index = 0; index < items_count; ++index ) {
        BitPropVariant item_path = in_archive.getItemProperty( index, BitProperty::Path );
        if ( item_path.isString() && filter( item_path.getString() ) ) {
            matched_indices.push_back( index );
        }
    }

    if ( matched_indices.empty() ) {
        throw BitException( kNoMatchingFile, ERROR_FILE_NOT_FOUND );
    }

    extractToFileSystem( in_archive, in_file, out_dir, matched_indices );
}

void BitExtractor::extractItems( const wstring& in_file,
                                 const vector< uint32_t >& indices,
                                 const wstring& out_dir ) const {
    BitInputArchive in_archive( *this, in_file );

    uint32_t number_items = in_archive.itemsCount();
    if ( std::any_of( indices.begin(), indices.end(), [ & ]( uint32_t index ) { return index >= number_items; } ) ) {
        /* if any of the indices is greater than the number of items in the archive we throw an exception, since it is
           an invalid index! */
        throw BitException( "Some index is not valid", E_INVALIDARG );
    }
    extractToFileSystem( in_archive, in_file, out_dir, indices );
}

void BitExtractor::extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_file );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    map< wstring, vector< byte_t > > buffersMap;
    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, buffersMap );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    const vector< uint32_t > indices( 1, index );
    HRESULT res = in_archive.extract( indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() +
                            L" (error code: " + std::to_wstring( res ) + L")", res );
    }
    out_buffer = std::move( buffersMap.begin()->second );
}

void BitExtractor::extract( const std::wstring& in_file, std::ostream& out_stream, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_file );

    uint32_t number_items = in_archive.itemsCount();
    if ( index >= number_items ) {
        throw BitException( L"Index " + std::to_wstring( index ) + L" is out of range", E_INVALIDARG );
    }

    if ( in_archive.isItemFolder( index ) ) { //Consider only files, not folders
        throw BitException( kCannotExtractFolderToBuffer, E_INVALIDARG );
    }

    auto* extract_callback_spec = new StreamExtractCallback( *this, in_archive, out_stream );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    const vector< uint32_t > indices( 1, index );
    HRESULT res = in_archive.extract( indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() +
                            L" (error code: " + std::to_wstring( res ) + L")", res );
    }
}

void BitExtractor::extract( const wstring& in_file, map< wstring, vector< byte_t > >& out_map ) const {
    BitInputArchive in_archive( *this, in_file );

    uint32_t number_items = in_archive.itemsCount();
    vector< uint32_t > files_indices;
    for ( uint32_t i = 0; i < number_items; ++i ) {
        if ( !in_archive.isItemFolder( i ) ) { //Consider only files, not folders
            files_indices.push_back( i );
        }
    }

    auto* extract_callback_spec = new MemExtractCallback( *this, in_archive, out_map );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive.extract( files_indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage(), res );
    }
}


void BitExtractor::test( const wstring& in_file ) const {
    BitInputArchive in_archive( *this, in_file );

    auto* extract_callback_spec = new FileExtractCallback( *this, in_archive, in_file, L"" );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );
    HRESULT res = in_archive.test( extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() +
                            L" (error code: " + std::to_wstring( res ) + L")", res );
    }
}

void BitExtractor::extractToFileSystem( const BitInputArchive& in_archive,
                                        const wstring& in_file,
                                        const wstring& out_dir,
                                        const vector< uint32_t >& indices ) const {
    auto* extract_callback_spec = new FileExtractCallback( *this, in_archive, in_file, out_dir );
    CMyComPtr< IArchiveExtractCallback > extract_callback( extract_callback_spec );

    HRESULT res = in_archive.extract( indices, extract_callback );
    if ( res != S_OK ) {
        throw BitException( extract_callback_spec->getErrorMessage() +
                            L" (error code: " + std::to_wstring( res ) + L")", res );
    }
}
