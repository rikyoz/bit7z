/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef STDINPUTITEM_HPP
#define STDINPUTITEM_HPP

#include "internal/genericinputitem.hpp"

namespace bit7z {
using std::istream;

class StdInputItem final : public GenericInputItem {
    public:
        explicit StdInputItem( istream& stream, const tstring& path );

        BIT7Z_NODISCARD tstring name() const override;

        BIT7Z_NODISCARD bool isDir() const noexcept override;

        BIT7Z_NODISCARD uint64_t size() const override;

        BIT7Z_NODISCARD FILETIME creationTime() const noexcept override;

        BIT7Z_NODISCARD FILETIME lastAccessTime() const noexcept override;

        BIT7Z_NODISCARD FILETIME lastWriteTime() const noexcept override;

        BIT7Z_NODISCARD uint32_t attributes() const noexcept override;

        BIT7Z_NODISCARD tstring path() const override;

        BIT7Z_NODISCARD fs::path inArchivePath() const override;

        BIT7Z_NODISCARD HRESULT getStream( ISequentialInStream** inStream ) const override;

    private:
        istream& mStream;
        const fs::path mStreamPath;
};
}  // namespace bit7z

#endif //STDINPUTITEM_HPP
