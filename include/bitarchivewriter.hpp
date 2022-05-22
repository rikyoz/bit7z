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

#ifndef BITARCHIVEWRITER_HPP
#define BITARCHIVEWRITER_HPP

#include "bitoutputarchive.hpp"

namespace bit7z {
/**
 * @brief The BitArchiveWriter class allows to create new archives or update old ones with new items.
 */
class BitArchiveWriter : public BitAbstractArchiveCreator, public BitOutputArchive {
    public:
        /**
         * @brief Constructs an empty BitArchiveWriter object that can write archives of the specified format.
         *
         * @param lib    the 7z library to use.
         * @param format the output archive format.
         */
        BitArchiveWriter( const Bit7zLibrary& lib, const BitInOutFormat& format );

        /**
         * @brief Constructs a BitArchiveWriter object, reading the given archive file path.
         *
         * @param lib      the 7z library to use.
         * @param in_file  the path to an input archive file.
         * @param format   the input/output archive format.
         * @param password (optional) the password needed to read the input archive.
         */
        BitArchiveWriter( const Bit7zLibrary& lib,
                          const tstring& in_file,
                          const BitInOutFormat& format,
                          const tstring& password = {} );

        /**
         * @brief Constructs a BitArchiveWriter object, reading the archive in the given buffer.
         *
         * @param lib       the 7z library to use.
         * @param in_buffer the buffer containing the input archive.
         * @param format    the input/output archive format.
         * @param password  (optional) the password needed to read the input archive.
         */
        BitArchiveWriter( const Bit7zLibrary& lib,
                          const vector< byte_t >& in_buffer,
                          const BitInOutFormat& format,
                          const tstring& password = {} );

        /**
         * @brief Constructs a BitArchiveWriter object, reading the archive from the given standard input stream.
         *
         * @param lib       the 7z library to use.
         * @param in_stream the standard stream of the input archive.
         * @param format    the input/output archive format.
         * @param password  (optional) the password needed to read the input archive.
         */
        BitArchiveWriter( const Bit7zLibrary& lib,
                          std::istream& in_stream,
                          const BitInOutFormat& format,
                          const tstring& password = {} );
};
}  // namespace bit7z

#endif //BITARCHIVEWRITER_HPP
