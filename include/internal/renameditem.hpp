/*
 * bit7z - A C++ static library to interface with the 7-zip DLLs.
 * Copyright (c) 2014-2021  Riccardo Ostani - All Rights Reserved.
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
}
#endif //RENAMEDITEM_HPP
