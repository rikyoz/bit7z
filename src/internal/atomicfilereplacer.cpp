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
#include "internal/fsutil.hpp"
#include "internal/stringutil.hpp"
#include "internal/util.hpp"

#include <system_error>
#include <utility>

namespace bit7z {

namespace {
/* Opens a CFileOutStream at "<target>.tmp", retrying with numeric postfixes
 * "<target>.tmp1", ".tmp2", ... on collision.
 * Mirrors 7-Zip's retry-on-collision behavior (1 + 65535 retries). */
auto openUniqueTempStream( const fs::path& targetPath ) -> CMyComPtr< CFileOutStream > {
    constexpr auto kMaxTempPathRetries = std::numeric_limits< std::uint16_t >::max();
    fs::path tmpCandidatePath = targetPath;
    tmpCandidatePath += BIT7Z_NATIVE_STRING( ".tmp" );
    std::uint32_t i = 0u; // Note: wider than kMaxTempPathRetries so that we can detect when we pass the limit.
    do {
        try {
            return make_com< CFileOutStream >( tmpCandidatePath );
        } catch ( const BitException& ex ) {
            if ( ex.code() != std::errc::file_exists ) {
                throw;
            }
        }
        if ( ++i > kMaxTempPathRetries ) {
            break;
        }
        tmpCandidatePath.replace_extension( BIT7Z_NATIVE_STRING( ".tmp" ) + to_native_string( i ) );
    } while ( true );
    throw BitException( "Could not allocate a unique temporary file name",
                        std::make_error_code( std::errc::file_exists ),
                        path_to_tstring( targetPath ) );
}
} // namespace

AtomicFileReplacer::AtomicFileReplacer( const fs::path& targetPath )
    : mTargetPath{ targetPath },
      mStream{ openUniqueTempStream( targetPath ) } {}

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