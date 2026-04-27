/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2024 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SECURETEMPDIR_HPP
#define SECURETEMPDIR_HPP

#ifndef _WIN32

#include "bitdefines.hpp"
#include "internal/fs.hpp"

namespace bit7z {

/**
 * @brief RAII owner of a uniquely-named, owner-only (mode 0700) subdirectory
 *        created next to a target path via POSIX mkdtemp. Removed recursively
 *        on destruction.
 *
 * Used to host the temporary file while updating an archive in place.
 * CFileOutStream's constructor rejects a temp path that already resolves to a
 * symlink, but that check and the subsequent open() are not atomic: an
 * attacker with write access to the parent directory could win the
 * check-then-open race by dropping a symlink in between, and a predictable
 * temp filename (e.g. "<target>.tmp") makes the race easy to target. Hosting
 * the temp file inside an owner-only directory created atomically by mkdtemp
 * removes both conditions: the path is unpredictable, and no unprivileged
 * process other than ourselves can create entries inside it, so the TOCTOU
 * window cannot be exploited.
 *
 * Not provided on Windows: creating a symlink there normally requires admin
 * privileges or Developer Mode, so an unprivileged attacker cannot stage the
 * race in the first place. The Windows update path writes to "<target>.tmp"
 * directly and relies on the symlink-rejection check in CFileOutStream's
 * constructor to reject any pre-placed symlink.
 */
class SecureTempDir {
        fs::path mDirectory;

    public:
        explicit SecureTempDir( const fs::path& nearPath );

        SecureTempDir( const SecureTempDir& ) = delete;
        SecureTempDir& operator=( const SecureTempDir& ) = delete;

        SecureTempDir( SecureTempDir&& other ) noexcept;
        SecureTempDir& operator=( SecureTempDir&& other ) noexcept;

        ~SecureTempDir();

        BIT7Z_NODISCARD auto path() const noexcept -> const fs::path& { return mDirectory; }
};

} // namespace bit7z

#endif // !_WIN32

#endif // SECURETEMPDIR_HPP