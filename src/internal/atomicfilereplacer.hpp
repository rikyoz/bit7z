/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef ATOMICFILEREPLACER_HPP
#define ATOMICFILEREPLACER_HPP

#include "internal/cfileoutstream.hpp"
#include "internal/com.hpp"
#ifndef _WIN32
#include "internal/securetempdir.hpp"
#endif

namespace bit7z {

/**
 * @brief RAII wrapper that writes to a temporary file via an owned CFileOutStream
 *        and atomically replaces the target path with it on commit().
 *
 * Used when updating an archive in place.
 *
 * - POSIX: the temp file lives in an owner-only directory created via mkdtemp,
 *   which is removed when this object is destroyed. The unpredictable name
 *   and restrictive permissions prevent an attacker with write access to the
 *   parent directory from winning the check-then-open race in CFileOutStream's
 *   symlink rejection. See SecureTempDir for the full rationale.
 * - Windows: the temp file is "<target>.tmp" next to the target. Symlink
 *   redirection is not specifically defended against here; CFileOutStream's
 *   constructor check (createAlways is false here) catches existing symlinks,
 *   and creating one requires SeCreateSymbolicLinkPrivilege anyway.
 */
class AtomicFileReplacer final {
    public:
        explicit AtomicFileReplacer( const fs::path& targetPath );

        AtomicFileReplacer( const AtomicFileReplacer& ) = delete;
        AtomicFileReplacer( AtomicFileReplacer&& ) = delete;
        auto operator=( const AtomicFileReplacer& ) -> AtomicFileReplacer& = delete;
        auto operator=( AtomicFileReplacer&& ) -> AtomicFileReplacer& = delete;

        ~AtomicFileReplacer() = default;

        /**
         * @return a non-owning pointer to the underlying output stream, valid as long
         *         as this AtomicFileReplacer is alive and commit() has not been called.
         */
        BIT7Z_NODISCARD auto stream() const noexcept -> IOutStream*;

        /**
         * @brief Releases the underlying stream (closing the temporary file)
         *        and renames it onto the target path, overwriting any existing destination.
         * @throws BitException on failure.
         */
        void commit();

    private:
#ifndef _WIN32
        SecureTempDir mTempDir;
#endif
        fs::path mTargetPath;
        CMyComPtr< CFileOutStream > mStream;
};

} // namespace bit7z

#endif // ATOMICFILEREPLACER_HPP