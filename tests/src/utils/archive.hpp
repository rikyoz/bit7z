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
#include "sourcelocation.hpp"

#include <bit7z/bitarchiveitem.hpp>
#include <bit7z/bitarchivereader.hpp>
#include <bit7z/bitdefines.hpp>
#include <bit7z/bitformat.hpp>

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

using filesystem::ArchiveContent;
using filesystem::ExpectedItem;

class TestArchiveContent {
        std::size_t mPackedSize;
        const ArchiveContent& mContent;

    public:
        TestArchiveContent( std::size_t packedSize, const ArchiveContent& content )
            : mPackedSize{ packedSize }, mContent{ content } {}

        BIT7Z_NODISCARD
        auto packedSize() const -> std::size_t {
            return mPackedSize;
        }

        BIT7Z_NODISCARD
        auto content() const -> const ArchiveContent& {
            return mContent;
        }
};

template< typename TestFormatType >
class TestArchive : public TestArchiveContent {
        TestFormatType mFormat;

    public:
        using FormatType = typename TestFormatType::Type;

        TestArchive( std::string extension,
                     const FormatType& format,
                     std::size_t packedSize,
                     const ArchiveContent& content )
            : TestArchiveContent{ packedSize, content }, mFormat{ std::move( extension ), format } {}

        BIT7Z_NODISCARD
        auto format() const -> const FormatType& {
            return mFormat.format;
        }

        BIT7Z_NODISCARD
        auto extension() const -> const std::string& {
            return mFormat.extension;
        }
};

using TestInputArchive = TestArchive< TestInputFormat >;
using TestOutputArchive = TestArchive< TestOutputFormat >;

using stream_t = fs::ifstream;

// Note: we cannot use value semantic and return the archive due to old GCC versions not supporting movable fstreams.
inline void getInputArchive( const fs::path& path, tstring& archive ) {
    archive = to_tstring( path );
}

inline void getInputArchive( const fs::path& path, buffer_t& archive ) {
    archive = filesystem::load_file( path );
}

inline void getInputArchive( const fs::path& path, stream_t& archive ) {
    archive.open( path, std::ios::binary );
}

template< typename T >
using is_filesystem_archive = std::is_same< bit7z::tstring, typename std::decay< T >::type >;

void require_archive_item( const BitInFormat& format,
                           const BitArchiveItem& item,
                           const ExpectedItem& expectedItem,
                           const SourceLocation& location );

#define REQUIRE_ARCHIVE_ITEM( format, item, expectedItem ) \
    require_archive_item( format, item, expectedItem, BIT7Z_CURRENT_LOCATION )

void require_archive_content( const BitArchiveReader& info,
                              const TestArchiveContent& input,
                              const SourceLocation& location );

#define REQUIRE_ARCHIVE_CONTENT( info, input ) \
    require_archive_content( info, input, BIT7Z_CURRENT_LOCATION )

void require_filesystem_item( const ExpectedItem& expectedItem, const SourceLocation& location );

#define REQUIRE_FILESYSTEM_ITEM( expectedItem ) require_filesystem_item( expectedItem, BIT7Z_CURRENT_LOCATION )

} // namespace test
} // namespace bit7z

#endif //ARCHIVE_HPP
