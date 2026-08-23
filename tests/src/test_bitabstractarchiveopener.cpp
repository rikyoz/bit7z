// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <catch2/catch.hpp>

#include "utils/format.hpp"
#include "utils/shared_lib.hpp"

#include <bit7z/bitabstractarchivehandler.hpp>
#include <bit7z/bitfileextractor.hpp>
#include <bit7z/bitformat.hpp>
#include <bit7z/bitmemextractor.hpp>
#include <bit7z/bitstreamextractor.hpp>

#include <tuple>

using namespace bit7z;
using test::TestInputFormat;

// Note: we don't include BitArchiveReader here because it needs an input archive to be constructed.
using OpenerTypes = std::tuple< BitFileExtractor, BitMemExtractor, BitStreamExtractor >;

TEMPLATE_LIST_TEST_CASE(
    "BitAbstractArchiveOpener: format() / extractionFormat()",
    "[bitabstractarchiveopener]",
    OpenerTypes
) {
    const auto testFormat = GENERATE( as<  TestInputFormat >(),
        TestInputFormat{ "zip", BitFormat::Zip },
        TestInputFormat{ "bz2", BitFormat::BZip2 },
        TestInputFormat{ "7z", BitFormat::SevenZip },
        TestInputFormat{ "xz", BitFormat::Xz },
        TestInputFormat{ "wim", BitFormat::Wim },
        TestInputFormat{ "tar", BitFormat::Tar },
        TestInputFormat{ "gz", BitFormat::GZip },
        TestInputFormat{ "rar", BitFormat::Rar5 },
        TestInputFormat{ "iso", BitFormat::Iso }
    );
    DYNAMIC_SECTION( "Format: " << testFormat.extension ) {
        const TestType opener{ test::sevenzipLib(), testFormat.format };
        REQUIRE( opener.extractionFormat() == testFormat.format );
        REQUIRE( opener.format() == testFormat.format );
    }
}

TEMPLATE_LIST_TEST_CASE(
    "BitAbstractArchiveOpener: the default handler settings",
    "[bitabstractarchiveopener]",
    OpenerTypes
) {
    const TestType opener( test::sevenzipLib(), BitFormat::SevenZip );

    // The archive openers overwrite any already existing output file,
    // unlike the archive creators, which default to OverwriteMode::None.
    REQUIRE( opener.overwriteMode() == OverwriteMode::Overwrite );

    // The archive openers keep the directory retention enabled by BitAbstractArchiveHandler,
    // unlike the archive creators, which disable it.
    REQUIRE( opener.retainDirectories() );
}
