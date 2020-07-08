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

#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include "../include/bitarchiveopener.hpp"
#include "../include/bitinputarchive.hpp"
#include "../include/bitexception.hpp"
#include "../include/bittypes.hpp"

namespace bit7z {
    constexpr auto kNoMatchingFile = "No matching file was found in the archive";

    namespace filesystem {
        namespace fsutil {
            bool wildcardMatch( const tstring& pattern, const tstring& str );
        }
    }

    template< typename Input >
    class BitExtractor : public BitArchiveOpener {
        public:
            explicit BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format DEFAULT_FORMAT )
                : BitArchiveOpener( lib, format ) {}

            void extract( Input input, const tstring& out_dir = TSTRING( "" ) ) const {
                BitInputArchive in_archive( *this, input );
                extractToFileSystem( in_archive, out_dir, vector< uint32_t >() );
            }

            void extract( Input input, vector< byte_t >& out_buffer, unsigned int index = 0 ) const {
                BitInputArchive in_archive( *this, input );
                extractToBuffer( in_archive, out_buffer, index );
            }

            void extract( Input input, std::ostream& out_stream, unsigned int index = 0 ) const {
                BitInputArchive in_archive( *this, input );
                extractToStream( in_archive, out_stream, index );
            }

            void extract( Input input, map< tstring, vector< byte_t > >& out_map ) const {
                BitInputArchive in_archive( *this, input );
                extractToBufferMap( in_archive, out_map );
            }

            void extractMatching( Input input, const tstring& item_filter, const tstring& out_dir ) const {
                using namespace filesystem;

                if ( item_filter.empty() ) {
                    throw BitException( "Empty wildcard filter", std::make_error_code( std::errc::invalid_argument  ) );
                }

                extractMatchingFilter( input, out_dir, [ &item_filter ]( const tstring& item_path ) -> bool {
                    return fsutil::wildcardMatch( item_filter, item_path );
                } );
            }

            void extractItems( Input input, const vector< uint32_t >& indices, const tstring& out_dir ) const {
                if ( indices.empty() ) {
                    throw BitException(  "Empty indices vector", std::make_error_code( std::errc::invalid_argument ) );
                }

                BitInputArchive in_archive( *this, input );
                uint32_t n_items = in_archive.itemsCount();
                const auto find_res = std::find_if( indices.cbegin(), indices.cend(), [ &n_items ]( uint32_t index ) -> bool {
                    return index >= n_items;
                } );
                if ( find_res != indices.cend() ) {
                    throw BitException(  "Index " + std::to_string( *find_res ) + " is not valid", std::make_error_code( std::errc::invalid_argument ) );
                }

                extractToFileSystem( in_archive, out_dir, indices );
            }

#ifdef BIT7Z_REGEX_MATCHING

            void extractMatchingRegex( Input input, const tstring& regex, const tstring& out_dir ) const {
                if ( regex.empty() ) {
                    throw BitException( "Empty regex filter", std::make_error_code( std::errc::invalid_argument ) );
                }

                const tregex regex_filter( regex, tregex::ECMAScript | tregex::optimize );
                extractMatchingFilter( input, out_dir, [ &regex_filter ]( const tstring& item_path ) -> bool {
                    return std::regex_match( item_path, regex_filter );
                } );
            }

#endif

            /**
             * @brief Tests the given archive without extracting its content.
             *
             * If the input archive is not valid, a BitException is thrown!
             *
             * @param in_file   the input archive file to be tested.
             */
            void test( Input input ) const {
                BitInputArchive in_archive( *this, input );
                BitArchiveOpener::test( in_archive );
            }

        private:
            void extractMatchingFilter( Input input, const tstring& out_dir,
                                        const function< bool( const tstring& ) >& filter ) const {
                BitInputArchive in_archive( *this, input );

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
                    throw BitException( kNoMatchingFile, std::make_error_code( std::errc::no_such_file_or_directory ) );
                }

                extractToFileSystem( in_archive, out_dir, matched_indices );
            }
    };
}

#endif //BITEXTRACTOR_HPP
