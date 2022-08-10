/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <algorithm>

#include "bitabstractarchiveopener.hpp"
#include "biterror.hpp"
#include "bitexception.hpp"
#include "bitinputarchive.hpp"

namespace bit7z {
namespace filesystem {
namespace fsutil {
bool wildcardMatch( const tstring& pattern, const tstring& str );
}
}

enum class FilterPolicy { Include, Exclude };

/**
 * @brief The BitExtractor template class allows to extract the content of archives from supported input types.
 */
template< typename Input >
class BitExtractor final : public BitAbstractArchiveOpener {
    public:
        /**
         * @brief Constructs a BitExtractor object.
         *
         * The Bit7zLibrary parameter is needed to have access to the functionalities
         * of the 7z DLLs. On the contrary, the BitInFormat is required to know the
         * format of the in_file archives.
         *
         * @note When bit7z is compiled using the BIT7Z_AUTO_FORMAT macro define, the format
         * argument has default value BitFormat::Auto (automatic format detection of the in_file archive).
         * Otherwise, when BIT7Z_AUTO_FORMAT is not defined (i.e., no auto format detection available)
         * the format argument must be specified.
         *
         * @param lib       the 7z library to use.
         * @param format    the in_file archive format.
         */
        explicit BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format BIT7Z_DEFAULT_FORMAT )
            : BitAbstractArchiveOpener( lib, format ) {}

        /**
         * @brief Extracts the given archive to the chosen directory.
         *
         * @param in_file       the input archive file.
         * @param out_dir       the output directory where extracted files will be put.
         */
        void extract( Input in_file, const tstring& out_dir = {} ) const {
            BitInputArchive in_archive( *this, in_file );
            in_archive.extract( out_dir );
        }

        /**
         * @brief Extracts a file from the given archive to the output buffer.
         *
         * @param in_file      the input archive file.
         * @param out_buffer   the output buffer where the content of the archive will be put.
         * @param index        the index of the file to be extracted from in_file.
         */
        void extract( Input in_file, vector< byte_t >& out_buffer, uint32_t index = 0 ) const {
            BitInputArchive in_archive( *this, in_file );
            in_archive.extract( out_buffer, index );
        }

        /**
         * @brief Extracts a file from the given archive to the output stream.
         *
         * @param in_file      the input archive file.
         * @param out_stream   the (binary) stream where the content of the archive will be put.
         * @param index        the index of the file to be extracted from in_file.
         */
        void extract( Input in_file, std::ostream& out_stream, uint32_t index = 0 ) const {
            BitInputArchive in_archive( *this, in_file );
            in_archive.extract( out_stream, index );
        }

        /**
         * @brief Extracts the content of the given archive into a map of memory buffers, where the keys are
         * the paths of the files (inside the archive), and the values are their decompressed contents.
         *
         * @param in_file   the input archive file.
         * @param out_map   the output map.
         */
        void extract( Input in_file, std::map< tstring, vector< byte_t > >& out_map ) const {
            BitInputArchive in_archive( *this, in_file );
            in_archive.extract( out_map );
        }

        /**
         * @brief Extracts the files in the archive that match the given wildcard pattern to the chosen directory.
         *
         * @param in_file       the input archive file.
         * @param item_filter   the wildcard pattern used for matching the paths of files inside the archive.
         * @param out_dir       the output directory where extracted files will be put.
         * @param policy        the filtering policy to be applied to the matched items.
         */
        void extractMatching( Input in_file, const tstring& item_filter,
                              const tstring& out_dir = {},
                              FilterPolicy policy = FilterPolicy::Include ) const {
            using namespace filesystem;

            if ( item_filter.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
            }

            extractMatchingFilter( in_file, out_dir, policy, [ &item_filter ]( const tstring& item_path ) -> bool {
                return fsutil::wildcardMatch( item_filter, item_path );
            } );
        }

        /**
         * @brief Extracts to the output buffer the first file in the archive matching the given wildcard pattern.
         *
         * @param in_file       the input archive file.
         * @param item_filter   the wildcard pattern used for matching the paths of files inside the archive.
         * @param out_buffer    the output buffer where to extract the file.
         * @param policy        the filtering policy to be applied to the matched items.
         */
        void extractMatching( Input in_file, const tstring& item_filter, vector< byte_t >& out_buffer,
                              FilterPolicy policy = FilterPolicy::Include ) const {
            using namespace filesystem;

            if ( item_filter.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
            }

            extractMatchingFilter( in_file, out_buffer, policy,
                                   [ &item_filter ]( const tstring& item_path ) -> bool {
                                       return fsutil::wildcardMatch( item_filter, item_path );
                                   } );
        }

        /**
         * @brief Extracts the specified items in the given archive to the chosen directory.
         *
         * @param in_file   the input archive file.
         * @param indices   the array of indices of the files in the archive that must be extracted.
         * @param out_dir   the output directory where the extracted files will be put.
         */
        void extractItems( Input in_file, const vector< uint32_t >& indices,
                           const tstring& out_dir = {} ) const {
            if ( indices.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::IndicesNotSpecified ) );
            }

            BitInputArchive in_archive( *this, in_file );
            uint32_t n_items = in_archive.itemsCount();
            // Find if any index passed by the user is not in the valid range [0, itemsCount() - 1]
            const auto find_res = std::find_if( indices.cbegin(),
                                                indices.cend(),
                                                [ &n_items ]( uint32_t index ) -> bool {
                                                    return index >= n_items;
                                                } );
            if ( find_res != indices.cend() ) {
                throw BitException( "Cannot extract item at the index " + std::to_string( *find_res ),
                                    make_error_code( BitError::InvalidIndex ) );
            }

            in_archive.extract( out_dir, indices );
        }

#ifdef BIT7Z_REGEX_MATCHING

        /**
         * @brief Extracts the files in the archive that match the given regex pattern to the chosen directory.
         *
         * @note Available only when compiling bit7z using the BIT7Z_REGEX_MATCHING preprocessor define.
         *
         * @param in_file       the input archive file.
         * @param regex         the regex used for matching the paths of files inside the archive.
         * @param out_dir       the output directory where extracted files will be put.
         * @param policy        the filtering policy to be applied to the matched items.
         */
        void extractMatchingRegex( Input in_file, const tstring& regex,
                                   const tstring& out_dir = {},
                                   FilterPolicy policy = FilterPolicy::Include ) const {
            if ( regex.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
            }

            const tregex regex_filter( regex, tregex::ECMAScript | tregex::optimize );
            extractMatchingFilter( in_file, out_dir, policy, [ &regex_filter ]( const tstring& item_path ) -> bool {
                return std::regex_match( item_path, regex_filter );
            } );
        }

        /**
         * @brief Extracts the first file in the archive that matches the given regex pattern to the output buffer.
         *
         * @note Available only when compiling bit7z using the BIT7Z_REGEX_MATCHING preprocessor define.
         *
         * @param in_file       the input archive file.
         * @param regex         the regex used for matching the paths of files inside the archive.
         * @param out_dir       the output directory where extracted files will be put.
         * @param policy        the filtering policy to be applied to the matched items.
         */
        void extractMatchingRegex( Input in_file, const tstring& regex, vector< byte_t >& out_buffer,
                                   FilterPolicy policy = FilterPolicy::Include ) const {
            if ( regex.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::FilterNotSpecified ) );
            }

            const tregex regex_filter( regex, tregex::ECMAScript | tregex::optimize );
            return extractMatchingFilter( in_file, out_buffer, policy,
                                          [ &regex_filter ]( const tstring& item_path ) -> bool {
                                              return std::regex_match( item_path, regex_filter );
                                          } );
        }

#endif

        /**
         * @brief Tests the given archive without extracting its content.
         *
         * If the in_file archive is not valid, a BitException is thrown!
         *
         * @param in_file   the in_file archive file to be tested.
         */
        void test( Input in_file ) const {
            BitInputArchive in_archive( *this, in_file );
            in_archive.test();
        }

    private:
        void extractMatchingFilter( Input in_file, const tstring& out_dir, FilterPolicy policy,
                                    const function< bool( const tstring& ) >& filter ) const {
            BitInputArchive in_archive( *this, in_file );

            vector< uint32_t > matched_indices;
            bool should_include_matched_items = policy == FilterPolicy::Include;
            //Searching for files inside the archive that match the given filter
            for ( const auto& item : in_archive ) {
                bool item_matches = filter( item.path() );
                if ( item_matches == should_include_matched_items ) {
                    /* Note: the if-condition is equivalent to an exclusive NOR (negated XOR) between
                     *       item_matches and should_include_matched_items.
                     *       It is true only if either:
                     *       - the current item matches the filter, and we must include it; or
                     *       - the current item doesn't match the filter, and we must exclude those that match. */
                    matched_indices.push_back( item.index() );
                }
            }

            if ( matched_indices.empty() ) {
                throw BitException( "Cannot extract items", make_error_code( BitError::NoMatchingItems ) );
            }

            in_archive.extract( out_dir, matched_indices );
        }

        void extractMatchingFilter( Input in_file, vector< byte_t >& out_buffer, FilterPolicy policy,
                                    const function< bool( const tstring& ) >& filter ) const {
            BitInputArchive in_archive( *this, in_file );

            bool should_extracted_matched_item = policy == FilterPolicy::Include;
            //Searching for files inside the archive that match the given filter
            for ( const auto& item : in_archive ) {
                bool item_matches = filter( item.path() );
                if ( item_matches == should_extracted_matched_item ) {
                    /* Note: the if-condition is equivalent to an exclusive NOR (negated XOR) between
                     *       item_matches and should_extracted_matched_item. */
                    in_archive.extract( out_buffer, item.index() );
                    return;
                }
            }

            throw BitException( "Failed to extract items", make_error_code( BitError::NoMatchingItems ) );
        }
};
}  // namespace bit7z

#endif //BITEXTRACTOR_HPP
