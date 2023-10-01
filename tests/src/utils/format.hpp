/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2023 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <string>

#include <bit7z/bitformat.hpp>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {

struct TestInputFormat {
    std::string extension;
    const BitInFormat& format;
};

inline auto format_has_path_metadata( const BitInFormat& format ) -> bool {
    return ( format != BitFormat::BZip2 ) && ( format != BitFormat::Lzma ) && ( format != BitFormat::Xz );
}

inline auto format_has_size_metadata( const BitInFormat& format ) -> bool {
    return ( format != BitFormat::BZip2 ) && ( format != BitFormat::Lzma );
}

inline auto format_has_crc( const BitInFormat& format ) -> bool {
    return ( format != BitFormat::BZip2 ) &&
           ( format != BitFormat::Iso ) &&
           ( format != BitFormat::Lzh ) && // Actually, Lzh provides a CRC, but it is not a CRC32 like other formats!
           ( format != BitFormat::Lzma ) &&
           ( format != BitFormat::Tar ) &&
           ( format != BitFormat::Wim ) &&
           ( format != BitFormat::Xz );
}

inline auto format_compresses_files( const BitInFormat& format ) -> bool {
    return ( format != BitFormat::Iso ) && ( format != BitFormat::Tar ) && ( format != BitFormat::Wim );
}

} // namespace test
} // namespace bit7z

#endif //FORMAT_HPP
