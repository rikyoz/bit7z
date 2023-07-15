/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef BIT7Z_ARCHIVE_HPP
#define BIT7Z_ARCHIVE_HPP

#include "filesystem.hpp"
#include "format.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

using filesystem::ArchiveContent;

class TestInputArchive {
    TestInputFormat m_format;
    std::size_t m_packedSize;
    const ArchiveContent& m_content;

public:
    TestInputArchive( std::string extension,
                      const BitInFormat& format,
                      std::size_t packedSize,
                      const ArchiveContent& content );

    auto format() const -> const BitInFormat&;

    auto extension() const -> const std::string&;

    auto packedSize() const -> std::size_t;

    auto content() const -> const ArchiveContent&;
};

} // namespace test
} // namespace bit7z

#endif //BIT7Z_ARCHIVE_HPP
