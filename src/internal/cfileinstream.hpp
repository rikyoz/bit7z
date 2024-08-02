/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef CFILEINSTREAM_HPP
#define CFILEINSTREAM_HPP

#include "bitexception.hpp"
#include "internal/com.hpp"
#include "internal/filehandle.hpp"
#include "internal/fs.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

namespace bit7z {

class CFileInStream : public IInStream, public CMyUnknownImp {
    public:
        explicit CFileInStream( const fs::path& filePath );

        CFileInStream( const CFileInStream& ) = delete;

        CFileInStream( CFileInStream&& ) = delete;

        auto operator=( const CFileInStream& ) -> CFileInStream& = delete;

        auto operator=( CFileInStream&& ) -> CFileInStream& = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CFileInStream() ) = default;

        // IInStream
        BIT7Z_STDMETHOD( Read, void* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IInStream ) //-V2507 //-V2511 //-V835 //-V3504

    private:
        InputFile mFile;
};

}  // namespace bit7z

#endif // CFILEINSTREAM_HPP
