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

#ifndef BITMEMEXTRACTOR_HPP
#define BITMEMEXTRACTOR_HPP

#include <iostream>
#include <vector>

#include "../include/bit7zlibrary.hpp"
#include "../include/bitguids.hpp"
#include "../include/bittypes.hpp"
#include "../include/bitarchiveopener.hpp"

namespace bit7z {
    using std::wstring;
    using std::vector;

    /**
     * @brief The BitMemExtractor class allows to extract the content of in-memory archives.
     */
    class BitMemExtractor : public BitArchiveOpener {
        public:
            /**
             * @brief Constructs a BitMemExtractor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitMemExtractor( const Bit7zLibrary& lib, const BitInFormat& format );

            /**
             * @brief Extracts the given buffer archive into the choosen directory.
             *
             * @param in_buffer     the buffer containing the archive to be extracted.
             * @param out_dir       the output directory where to put the file extracted.
             */
            void extract( const vector< byte_t >& in_buffer, const wstring& out_dir = L"" ) const;

            /**
             * @brief Extracts the given buffer archive into the output buffer.
             *
             * @param in_buffer    the buffer containing the archive to be extracted.
             * @param out_buffer   the output buffer where the content of the archive will be put.
             * @param index        the index of the file to be extracted from in_buffer.
             */
            void extract( const vector< byte_t >& in_buffer, vector< byte_t >& out_buffer,
                          unsigned int index = 0 ) const;
    };
}

#endif // BITMEMEXTRACTOR_HPP
