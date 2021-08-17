// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#ifndef BITCOMPRESSOR_HPP
#define BITCOMPRESSOR_HPP

#include <vector>

#include "bitabstractarchivecreator.hpp"
#include "bitoutputarchive.hpp"
#include "bittypes.hpp"

namespace bit7z {
    using std::vector;

    namespace filesystem {
        namespace fsutil {
            tstring filename( const tstring& path, bool ext = false );
        }
    }

    using namespace filesystem;

    template< typename Input >
    class BitCompressor : public BitAbstractArchiveCreator {
        public:
            BitCompressor( Bit7zLibrary const& lib, BitInOutFormat const& format )
                : BitAbstractArchiveCreator( lib, format ) {}

            void compressFile( Input& in_file,
                               const tstring& out_file,
                               const tstring& input_name = TSTRING( "" ) ) const {
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

            void compressFile( Input& in_file,
                               vector< byte_t >& out_buffer,
                               const tstring& input_name = TSTRING( "" ) ) const {
                BitOutputArchive output_archive{ *this, out_buffer };
                output_archive.addFile( in_file, input_name );
                output_archive.compressTo( out_buffer );
            }

            void compressFile( Input& in_file,
                               ostream& out_stream,
                               const tstring& input_name = TSTRING( "" ) ) const {
                BitOutputArchive output_archive{ *this };
                output_archive.addFile( in_file, input_name );
                output_archive.compressTo( out_stream );
            }
    };
}

#endif //BITCOMPRESSOR_HPP
