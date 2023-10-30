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

#include <catch2/catch_test_macros.hpp>

#include "utils/shared_lib.hpp"

#include <bit7z/bitformat.hpp>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitfilecompressor.hpp>

using namespace bit7z;
using bit7z::Bit7zLibrary;
using bit7z::BitFileCompressor;
using bit7z::BitInOutFormat;

TEST_CASE( "BitFileCompressor: TODO", "[bitfilecompressor]" ) {
    const Bit7zLibrary lib{ test::sevenzip_lib_path() };

    const BitFileCompressor fileCompressor{ lib, BitFormat::SevenZip};
    REQUIRE( fileCompressor.compressionFormat() == BitFormat::SevenZip ); // Just a placeholder test.
}

