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

#ifndef BITCOMPRESSOR_HPP
#define BITCOMPRESSOR_HPP

#include <string>
#include <vector>
#include <ostream>
#include <map>

#include "../include/bitarchivecreator.hpp"
#include "../include/bittypes.hpp"

namespace bit7z {
    using std::vector;
    using std::map;
    using std::ostream;

    namespace filesystem {
        class FSItem;
    }

    using namespace filesystem;

    /**
     * @brief The BitCompressor class allows to compress files and directories into file archives.
     *
     * It let decide various properties of the produced archive file, such as the password
     * protection and the compression level desired.
     */
    class BitCompressor : public BitArchiveCreator {
        public:
            /**
             * @brief Constructs a BitCompressor object.
             *
             * The Bit7zLibrary parameter is needed in order to have access to the functionalities
             * of the 7z DLLs. On the other hand, the BitInOutFormat is required in order to know the
             * format of the output archive.
             *
             * @param lib       the 7z library used.
             * @param format    the output archive format.
             */
            BitCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format );

            /* Compression from file system to file system */

            /**
             * @brief Compresses the given files or directories.
             *
             * The items in the first argument must be the relative or absolute paths to files or
             * directories existing on the filesystem.
             *
             * @param in_paths      a vector of paths.
             * @param out_file   the path (relative or absolute) to the output archive file.
             */
            void compress( const vector< tstring >& in_paths, const tstring& out_file ) const;

            /**
             * @brief Compresses the given files or directories using the specified aliases.
             *
             * The items in the first argument must be the relative or absolute paths to files or
             * directories existing on the filesystem.
             * Each pair of the map must follow the following format:
             *  {L"path to file in the filesystem", L"alias path in the archive"}.
             *
             * @param in_paths      a map of paths and corresponding aliases.
             * @param out_file   the path (relative or absolute) to the output archive file.
             */
            void compress( const map< tstring, tstring >& in_paths, const tstring& out_file ) const;

            /**
             * @brief Compresses a single file.
             *
             * @param in_file       the path (relative or absolute) to the input file.
             * @param out_file   the path (relative or absolute) to the output archive file.
             */
            void compressFile( const tstring& in_file, const tstring& out_file ) const;

            /**
             * @brief Compresses a group of files.
             *
             * @note Any path to a directory or to a not-existing file will be ignored!
             *
             * @param in_files      the path (relative or absolute) to the input files.
             * @param out_file   the path (relative or absolute) to the output archive file.
             */
            void compressFiles( const vector< tstring >& in_files, const tstring& out_file ) const;

            /**
             * @brief Compresses the files contained in a directory.
             *
             * @param in_dir        the path (relative or absolute) to the input directory.
             * @param out_file   the path (relative or absolute) to the output archive file.
             * @param recursive     if true, it searches files inside the sub-folders of in_dir.
             * @param filter        the filter to use when searching files inside in_dir.
             */
            void compressFiles( const tstring& in_dir,
                                const tstring& out_file,
                                bool recursive = true,
                                const tstring& filter = TSTRING( "*.*" ) ) const;

            /**
             * @brief Compresses an entire directory.
             *
             * @note This method is equivalent to compressFiles with filter set to L"".
             *
             * @param in_dir        the path (relative or absolute) to the input directory.
             * @param out_file   the path (relative or absolute) to the output archive file.
             */
            void compressDirectory( const tstring& in_dir, const tstring& out_file ) const;

            /* Compression from file system to memory buffer */

            /**
             * @brief Compresses the input file to the output buffer.
             *
             * @note If the format of the output doesn't support in memory compression, a BitException is thrown.
             *
             * @param in_file           the file to be compressed.
             * @param out_buffer        the buffer going to contain the output archive.
             */
            void compressFile( const tstring& in_file, vector< byte_t >& out_buffer ) const;

            /* Compression from file system to standard stream */

            /**
             * @brief Compresses the given files or directories.
             *
             * The items in the first argument must be the relative or absolute paths to files or
             * directories existing on the filesystem.
             *
             * @param in_paths      a vector of paths.
             * @param out_stream    the standard ostream where the archive will be output.
             */
            void compress( const vector< tstring >& in_paths, ostream& out_stream ) const;

            /**
             * @brief Compresses the given files or directories using the specified aliases.
             *
             * The items in the first argument must be the relative or absolute paths to files or
             * directories existing on the filesystem.
             * Each pair of the map must follow the following format:
             *  {L"path to file in the filesystem", L"alias path in the archive"}.
             *
             * @param in_paths      a map of paths and corresponding aliases.
             * @param out_stream    the standard ostream where to output the archive file.
             */
            void compress( const map< tstring, tstring >& in_paths, ostream& out_stream ) const;

        private:
            void compressOut( const vector< FSItem >& in_items, const tstring& out_file ) const;

            void compressOut( const vector< FSItem >& in_items, ostream& out_stream ) const;
    };
}
#endif // BITCOMPRESSOR_HPP
