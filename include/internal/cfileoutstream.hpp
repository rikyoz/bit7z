/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CFILEOUTSTREAM_HPP
#define CFILEOUTSTREAM_HPP

#include <array>

#include "bitdefines.hpp"
#include "internal/cstdoutstream.hpp"
#include "internal/fs.hpp"

namespace bit7z {
class CFileOutStream : public CStdOutStream {
    public:
        explicit CFileOutStream( const fs::path& filePath, bool createAlways = false );

        BIT7Z_NODISCARD bool fail();

    private:
        fs::ofstream mFileStream;

        static constexpr auto buffer_size = 1024 * 1024; // 1 MiB
        std::array< char, buffer_size > mBuffer;
};
}  // namespace bit7z

#endif // CFILEOUTSTREAM_HPP
