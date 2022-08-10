/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RENAMEDITEM_HPP
#define RENAMEDITEM_HPP

#include "bitinputarchive.hpp"
#include "internal/genericinputitem.hpp"

namespace bit7z {
class RenamedItem final : public GenericInputItem {
    public:
        explicit RenamedItem( const BitInputArchive& input_archive, uint32_t index, const tstring& new_path );

        BIT7Z_NODISCARD tstring name() const override;

        BIT7Z_NODISCARD bool isDir() const override;

        BIT7Z_NODISCARD uint64_t size() const override;

        BIT7Z_NODISCARD FILETIME creationTime() const override;

        BIT7Z_NODISCARD FILETIME lastAccessTime() const override;

        BIT7Z_NODISCARD FILETIME lastWriteTime() const override;

        BIT7Z_NODISCARD uint32_t attributes() const override;

        BIT7Z_NODISCARD tstring path() const override;

        BIT7Z_NODISCARD fs::path inArchivePath() const override;

        BIT7Z_NODISCARD HRESULT getStream( ISequentialInStream** inStream ) const noexcept override;

        BIT7Z_NODISCARD bool hasNewData() const noexcept override;

    private:
        const BitInputArchive& mInputArchive;
        uint32_t mIndex;
        const fs::path mNewPath;
};
}  // namespace bit7z
#endif //RENAMEDITEM_HPP
