// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2019  Riccardo Ostani - All Rights Reserved.
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

#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/fileextractcallback.hpp"
#include "../include/fsutil.hpp"

using namespace bit7z;
using namespace bit7z::filesystem;

#ifdef BIT7Z_REGEX_MATCHING
using std::wregex;
#endif

constexpr auto kNoMatchingFile = "No matching file was found in the archive";

BitExtractor::BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format ) : BitArchiveOpener( lib, format ) {}

void BitExtractor::extract( const tstring& in_file, const tstring& out_dir ) const {
    BitInputArchive in_archive( *this, in_file );
    extractToFileSystem( in_archive, in_file, out_dir, vector< uint32_t >() );
}

void BitExtractor::extractMatching( const tstring& in_file, const tstring& item_filter, const tstring& out_dir ) const {
    if ( item_filter.empty() ) {
        throw BitException(  "Empty wildcard filter", std::make_error_code( std::errc::invalid_argument  ) );
    }

    extractMatchingFilter( in_file, out_dir, [ &item_filter ]( const tstring& item_path ) -> bool {
        return fsutil::wildcardMatch( item_filter, item_path );
    } );
}

#ifdef BIT7Z_REGEX_MATCHING

void BitExtractor::extractMatchingRegex( const tstring& in_file, const tstring& regex, const tstring& out_dir ) const {
    if ( regex.empty() ) {
        throw BitException(  "Empty regex filter", std::make_error_code( std::errc::invalid_argument ) );
    }

    const tregex regex_filter( regex, std::regex::ECMAScript | std::regex::optimize );
    extractMatchingFilter( in_file, out_dir, [ &regex_filter ]( const tstring& item_path ) -> bool {
        return std::regex_match( item_path, regex_filter );
    } );
}

#endif

void BitExtractor::extractMatchingFilter( const tstring& in_file,
                                          const tstring& out_dir,
                                          const function< bool( const tstring& ) >& filter ) const {
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
        throw BitException(  kNoMatchingFile, std::make_error_code( std::errc::no_such_file_or_directory ) );
    }

    extractToFileSystem( in_archive, in_file, out_dir, matched_indices );
}

void BitExtractor::extractItems( const tstring& in_file,
                                 const vector< uint32_t >& indices,
                                 const tstring& out_dir ) const {
    if ( indices.empty() ) {
        throw BitException(  "Empty indices vector", std::make_error_code( std::errc::invalid_argument ) );
    }

    BitInputArchive in_archive( *this, in_file );
    uint32_t n_items = in_archive.itemsCount();
    const auto find_res = std::find_if( indices.cbegin(), indices.cend(), [ &n_items ]( uint32_t index ) -> bool {
        return index >= n_items;
    } );
    if ( find_res != indices.cend() ) {
        throw BitException(  "Index " + std::to_string( *find_res ) + " is not valid", std::make_error_code( std::errc::invalid_argument ) );
    }

    extractToFileSystem( in_archive, in_file, out_dir, indices );
}

void BitExtractor::extract( const tstring& in_file, vector< byte_t >& out_buffer, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_file );
    extractToBuffer( in_archive, out_buffer, index );
}

void BitExtractor::extract( const tstring& in_file, std::ostream& out_stream, unsigned int index ) const {
    BitInputArchive in_archive( *this, in_file );
    extractToStream( in_archive, out_stream, index );
}

void BitExtractor::extract( const tstring& in_file, map< tstring, vector< byte_t > >& out_map ) const {
    BitInputArchive in_archive( *this, in_file );
    extractToBufferMap( in_archive, out_map );
}

void BitExtractor::test( const tstring& in_file ) const {
    BitInputArchive in_archive( *this, in_file );

    CMyComPtr< ExtractCallback > extract_callback = new FileExtractCallback( *this, in_archive, in_file, TSTRING(""), true );
    in_archive.test( extract_callback );
}
