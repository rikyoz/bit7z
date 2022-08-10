/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITFILECOMPRESSOR_HPP
#define BITFILECOMPRESSOR_HPP

#include <map>
#include <ostream>
#include <vector>

#include "bitcompressor.hpp"

namespace bit7z {
using std::vector;
using std::map;
using std::ostream;

namespace filesystem {
class FSItem;
}

using namespace filesystem;

/**
 * @brief The BitFileCompressor class allows to compress files and directories into file archives.
 *
 * It let decide various properties of the produced archive file, such as the password
 * protection and the compression level desired.
 */
class BitFileCompressor final : public BitCompressor< const tstring > {
    public:
        /**
         * @brief Constructs a BitFileCompressor object.
         *
         * The Bit7zLibrary parameter is needed to have access to the functionalities
         * of the 7z DLLs. On the contrary, the BitInOutFormat is required to know the
         * format of the output archive.
         *
         * @param lib       the 7z library used.
         * @param format    the output archive format.
         */
        BitFileCompressor( const Bit7zLibrary& lib, const BitInOutFormat& format );

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
                            const tstring& filter = BIT7Z_STRING( "*.*" ) ) const;

        /**
         * @brief Compresses an entire directory.
         *
         * @note This method is equivalent to compressFiles with filter set to L"".
         *
         * @param in_dir        the path (relative or absolute) to the input directory.
         * @param out_file   the path (relative or absolute) to the output archive file.
         */
        void compressDirectory( const tstring& in_dir, const tstring& out_file ) const;

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
};
}  // namespace bit7z
#endif // BITFILECOMPRESSOR_HPP
