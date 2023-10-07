/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include "filesystem.hpp"
#include "format.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

using filesystem::ArchiveContent;

class TestInputArchive {
        TestInputFormat mFormat;
        std::size_t mPackedSize;
        const ArchiveContent& mContent;

    public:
        TestInputArchive( std::string extension,
                          const BitInFormat& format,
                          std::size_t packedSize,
                          const ArchiveContent& content );

        BIT7Z_NODISCARD
        auto format() const -> const BitInFormat&;

        BIT7Z_NODISCARD
        auto extension() const -> const std::string&;

        BIT7Z_NODISCARD
        auto packedSize() const -> std::size_t;

        BIT7Z_NODISCARD
        auto content() const -> const ArchiveContent&;
};

} // namespace test
} // namespace bit7z

#endif //ARCHIVE_HPP
