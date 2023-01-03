/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

        BIT7Z_NODISCARD fs::path path() const;

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

}  // namespace bit7z

#endif //PROCESSEDITEM_HPP
