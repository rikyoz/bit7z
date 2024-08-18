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

#include "bitexception.hpp"
#include "internal/com.hpp"
#include "internal/filehandle.hpp"
#include "internal/guids.hpp"
#include "internal/macros.hpp"

#include <7zip/IStream.h>

namespace bit7z {

class CFileOutStream : public IOutStream, public CMyUnknownImp {
    public:
        explicit CFileOutStream( const fs::path& filePath, bool createAlways = false );

        CFileOutStream( const CFileOutStream& ) = delete;

        CFileOutStream( CFileOutStream&& ) = delete;

        auto operator=( const CFileOutStream& ) -> CFileOutStream& = delete;

        auto operator=( CFileOutStream&& ) -> CFileOutStream& = delete;

        MY_UNKNOWN_VIRTUAL_DESTRUCTOR( ~CFileOutStream() ) = default;

        // IOutStream
        BIT7Z_STDMETHOD( Write, void const* data, UInt32 size, UInt32* processedSize );

        BIT7Z_STDMETHOD( Seek, Int64 offset, UInt32 seekOrigin, UInt64* newPosition );

        BIT7Z_STDMETHOD( SetSize, UInt64 newSize );

        // NOLINTNEXTLINE(modernize-use-noexcept, modernize-use-trailing-return-type, readability-identifier-length)
        MY_UNKNOWN_IMP1( IOutStream ) //-V2507 //-V2511 //-V835 //-V3504

    private:
        OutputFile mFile;
        fs::path mFilePath;
};

}  // namespace bit7z

#endif // CFILEOUTSTREAM_HPP
