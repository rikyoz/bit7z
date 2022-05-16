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

#ifndef PROCESSEDITEM_HPP
#define PROCESSEDITEM_HPP

#include "bitinputarchive.hpp"
#include "internal/fs.hpp"
#include "internal/windows.hpp"

namespace bit7z {
    class ProcessedItem final {
        public:
            ProcessedItem();

            void loadItemInfo( const BitInputArchive& input_archive, std::uint32_t item_index );

            BIT7Z_NODISCARD fs::path path() const ;

            BIT7Z_NODISCARD uint32_t attributes() const;

            BIT7Z_NODISCARD bool areAttributesDefined() const;

            BIT7Z_NODISCARD FILETIME modifiedTime() const;

            BIT7Z_NODISCARD bool isModifiedTimeDefined() const;

        private:
            fs::path mFilePath;

            FILETIME mModifiedTime;
            bool mIsModifiedTimeDefined;

            uint32_t mAttributes;
            bool mAreAttributesDefined;

            void loadFilePath( const BitInputArchive& input_archive, uint32_t item_index );

            void loadAttributes( const BitInputArchive& input_archive, uint32_t item_index );

            void loadModifiedTime( const BitInputArchive& input_archive, uint32_t item_index );
    };
}

#endif //PROCESSEDITEM_HPP
