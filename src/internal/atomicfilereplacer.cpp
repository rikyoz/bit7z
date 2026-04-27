// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal/atomicfilereplacer.hpp"

#include "bitexception.hpp"
#include "internal/stringutil.hpp"

#include <system_error>
#include <utility>

namespace bit7z {

#ifdef _WIN32
namespace {
auto makeTempPath( const fs::path& targetPath ) -> fs::path {
    fs::path tmp = targetPath;
    tmp += L".tmp";
    return tmp;
}
} // namespace

AtomicFileReplacer::AtomicFileReplacer( const fs::path& targetPath )
    : mTargetPath{ targetPath },
      mStream{ new CFileOutStream{ makeTempPath( targetPath ) } } {}
#else
AtomicFileReplacer::AtomicFileReplacer( const fs::path& targetPath )
    : mTempDir{ targetPath },
      mTargetPath{ targetPath },
      mStream{ new CFileOutStream{ mTempDir.path() / "data" } } {}
#endif

auto AtomicFileReplacer::stream() const noexcept -> IOutStream* {
    return mStream;
}

void AtomicFileReplacer::commit() {
    // We need to save the temp file path before releasing the stream object.
    const fs::path tempPath = std::move( *mStream ).path();

    /* NOTE: In the following instruction, we use the (dot) operator, not the -> (arrow) operator:
     *       in fact, both CMyComPtr and IOutStream have a Release() method, and we need to call only
     *       the one of CMyComPtr (which in turns calls the one of IOutStream)! */
    mStream.Release(); //Releasing the output stream so that we can rename it as the original file.

    std::error_code error;
#if defined( __MINGW32__ ) && defined( BIT7Z_USE_STANDARD_FILESYSTEM )
    /* MinGW seems to not follow the standard since filesystem::rename does not overwrite an already
     * existing destination file (as it should). So we explicitly remove it before! */
    if ( !fs::remove( mTargetPath, error ) ) {
        throw BitException( "Failed to delete the old archive file",
                            error, path_to_tstring( mTargetPath ) );
    }
#endif

    // Remove the old file and rename the temporary file (move file with overwriting).
    fs::rename( tempPath, mTargetPath, error );
    if ( error ) {
        throw BitException( "Failed to overwrite the old archive file",
                            error, path_to_tstring( mTargetPath ) );
    }
}

} // namespace bit7z