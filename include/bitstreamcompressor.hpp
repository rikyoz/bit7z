/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2020  Riccardo Ostani - All Rights Reserved.
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

#ifndef BITSTREAMCOMPRESSOR_HPP
#define BITSTREAMCOMPRESSOR_HPP

#include <istream>

#include "../include/bitarchivecreator.hpp"

namespace bit7z {
    using std::istream;

    class BitStreamCompressor : public BitArchiveCreator {
        public:
            /**
             * @brief Constructs a BitStreamCompressor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInOutFormat is required in order to know the
             * format of the input archive.
             *
             * @param lib       the 7z library used.
             * @param format    the input archive format.
             */
            BitStreamCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format );

            /**
             * @brief Compresses the given standard istream to the standard ostream.
             *
             * @param in_stream         the (binary) stream to be compressed.
             * @param out_stream        the (binary) stream where the archive will be output.
             * @param in_stream_name    (optional) the name to be used for the content of the archive.
             */
            void compress( istream& in_stream,
                           ostream& out_stream,
                           const tstring& in_stream_name = TSTRING( "" ) ) const;

            /**
             * @brief Compresses the given standard istream to the output buffer.
             *
             * @param in_stream         the (binary) stream to be compressed.
             * @param out_buffer        the buffer going to contain the output archive.
             * @param in_stream_name    (optional) the name to be used for the content of the archive.
             */
            void compress( istream& in_stream,
                           vector< byte_t >& out_buffer,
                           const tstring& in_stream_name = TSTRING( "" ) ) const;

            /**
             * @brief Compresses the given standard istream to an archive on the filesystem.
             *
             * @param in_stream         the (binary) stream to be compressed.
             * @param out_file          the output archive file path.
             * @param in_stream_name    (optional) the name to be used for the content of the archive.
             */
            void compress( istream& in_stream,
                           const tstring& out_file,
                           const tstring& in_stream_name = TSTRING( "" ) ) const;
    };
}

#endif // BITSTREAMCOMPRESSOR_HPP
