// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "archive.hpp"

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

TestInputArchive::TestInputArchive( std::string extension, const BitInFormat& format,
                                    std::size_t packedSize, const ArchiveContent& content )
    : mFormat{ std::move( extension ), format }, mPackedSize{ packedSize }, mContent{ content } {}

auto TestInputArchive::format() const -> const BitInFormat& {
    return mFormat.format;
}

auto TestInputArchive::packedSize() const -> std::size_t {
    return mPackedSize;
}

auto TestInputArchive::content() const -> const ArchiveContent& {
    return mContent;
}

auto TestInputArchive::extension() const -> const std::string& {
    return mFormat.extension;
}

} // namespace test
} // namespace bit7z