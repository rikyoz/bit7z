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

#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include "../include/bittypes.hpp"
#include "../include/cstdinstream.hpp"
#include "../include/fs.hpp"

namespace bit7z {
    class CFileInStream : public CStdInStream {
        public:
            explicit CFileInStream( const fs::path& filePath );

            void open( const fs::path& filePath );

            bool fail();

        private:
            fs::ifstream mFileStream;
    };
}

#endif // CFILEINSTREAM_HPP
