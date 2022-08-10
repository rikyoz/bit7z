// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BITCOMPRESSOR_HPP
#define BITCOMPRESSOR_HPP

#include <vector>

#include "bitoutputarchive.hpp"

namespace bit7z {
using std::vector;

namespace filesystem {
namespace fsutil {
tstring filename( const tstring& path, bool ext = false );
}
}

using namespace filesystem;

/**
 * @brief The BitCompressor template class allows to compress files into archives.
 *
 * It let decide various properties of the produced archive file, such as the password
 * protection and the compression level desired.
 */
template< typename Input >
class BitCompressor : public BitAbstractArchiveCreator {
    public:
        /**
         * @brief Constructs a BitCompressor object.
         *
         * The Bit7zLibrary parameter is needed to have access to the functionalities
         * of the 7z DLLs. On the contrary, the BitInOutFormat is required to know the
         * format of the output archive.
         *
         * @param lib       the 7z library to use.
         * @param format    the output archive format.
         */
        BitCompressor( Bit7zLibrary const& lib, BitInOutFormat const& format )
            : BitAbstractArchiveCreator( lib, format ) {}

        /**
         * @brief Compresses a single file.
         *
         * @param in_file  the file to be compressed.
         * @param out_file the path (relative or absolute) to the output archive file.
         */
        void compressFile( Input& in_file,
                           const tstring& out_file,
                           const tstring& input_name = {} ) const {
            /* Note: if in_file is a filesystem path (i.e., its type is const tstring&), we can deduce the archived
             * item filename using the original filename. Otherwise, if the user didn't specify the input file name,
             * we use the filename (without extension) of the output file path. */
            tstring name;
#ifdef __cpp_if_constexpr
            if constexpr ( !std::is_same_v< Input, const tstring > ) {
#else
            //There's probably some compile-time SFINAE alternative for C++14, but life is too short ;)
            if ( !std::is_same< Input, const tstring >::value ) {
#endif
                name = input_name.empty() ? fsutil::filename( out_file ) : input_name;
            }

            BitOutputArchive output_archive{ *this, out_file };
            output_archive.addFile( in_file, name );
            output_archive.compressTo( out_file );
        }

        /**
         * @brief Compresses the input file to the output buffer.
         *
         * @param in_file     the file to be compressed.
         * @param out_buffer  the buffer going to contain the output archive.
         * @param input_name  (optional) the name to give to the compressed file inside the output archive.
         */
        void compressFile( Input& in_file,
                           vector< byte_t >& out_buffer,
                           const tstring& input_name = {} ) const {
            BitOutputArchive output_archive{ *this, out_buffer };
            output_archive.addFile( in_file, input_name );
            output_archive.compressTo( out_buffer );
        }

        /**
         * @brief Compresses the input file to the output stream.
         *
         * @param in_file     the file to be compressed.
         * @param out_stream  the output stream.
         * @param input_name  (optional) the name to give to the compressed file inside the output archive.
         */
        void compressFile( Input& in_file,
                           ostream& out_stream,
                           const tstring& input_name = {} ) const {
            BitOutputArchive output_archive{ *this };
            output_archive.addFile( in_file, input_name );
            output_archive.compressTo( out_stream );
        }
};
}  // namespace bit7z

#endif //BITCOMPRESSOR_HPP
