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

#ifndef BITEXTRACTOR_HPP
#define BITEXTRACTOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitarchiveopener.hpp"

struct IInArchive;

namespace bit7z {
    using std::wstring;
    using std::vector;

    /**
     * @brief The BitExtractor class allows to extract the content of file archives.
     */
    class BitExtractor : public BitArchiveOpener {
        public:
            /**
             * @brief Constructs a BitExtractor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitExtractor( const Bit7zLibrary& lib, const BitInFormat& format );

            /**
             * @brief Extracts the given archive into the choosen directory.

             * @param in_file       the input archive file.
             * @param out_dir       the output directory where extracted files will be put.
             */
            void extract( const wstring& in_file, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the matching files in the given archive into the choosen directory.

             * @param in_file       the input archive file.
             * @param out_dir       the output directory where extracted files will be put.
             * @param item_filter   only files with (archive) paths matching the filter will be extracted.
             */
            void extractMatching( const wstring& in_file, const wstring& item_filter, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the specified items in the given archive into the choosen directory.

             * @param in_file   the input archive file.
             * @param out_dir   the output directory where extracted files will be put.
             * @param indices   the array of indices of the files in the archive that must be extracted.
             */
            void extractItems( const wstring& in_file, const vector<uint32_t>& indices, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the given archive into the output buffer.

             * @param in_file      the input archive file.
             * @param out_buffer   the output buffer where the content of the archive will be put.
             * @param index        the index of the file to be extracted from in_file.
             */
            void extract( const wstring& in_file, vector< byte_t >& out_buffer, unsigned int index = 0 );

            /**
             * @brief Tests the given archive without extracting its content.
             *
             * If the input archive is not valid, a BitException is thrown!
             *
             * @param in_file   the input archive file.
             */
            void test( const wstring& in_file );

        private:
            void extractToFileSystem( IInArchive* in_archive, const wstring& in_file,
                                      const wstring& out_dir, const vector<uint32_t>& indices ) const;
    };
}
#endif // BITEXTRACTOR_HPP
