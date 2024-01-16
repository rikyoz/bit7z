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

#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp> // IMPORTANT: it must be included before any other includes.

#include <iostream>

#include "utils/compiler.hpp"
#include "utils/filesystem.hpp"
#include "utils/flags.hpp"
#include "utils/shared_lib.hpp"

auto main( int argc, char* argv[] ) -> int try {
    using namespace bit7z::test;

    std::clog << "[Compiler]\n";
    std::clog << "Name: " << compiler::name << '\n';
    std::clog << "Version: " << compiler::version << '\n';
    std::clog << "Target Architecture: " << compiler::target_arch << "\n\n";

    std::clog << "[C Runtime Library]\n";
    std::clog << "Name: " << compiler::c_runtime << "\n\n";

    std::clog << "[C++ Standard Library]\n";
    std::clog << "Name: " << compiler::standard_library << '\n';
    std::clog << "Version: " << compiler::standard_library_version << "\n\n";

    std::clog << "[Runtime]\n";
    std::clog << "Executable path: " << filesystem::exe_path().string() << '\n';
    std::clog << "7-zip shared library: ";
#if defined( BIT7Z_USE_NATIVE_STRING ) && defined( _WIN32 )
    std::wclog << sevenzip_lib_path() << "\n";
#else
    std::clog << sevenzip_lib_path() << '\n';
#endif
#ifdef BIT7Z_TESTS_FILESYSTEM
    std::clog << "Test data path: " << filesystem::test_data_dir << '\n';
#endif
#ifdef _WIN32
    std::clog << "Code page: " << GetACP() << '\n';
#endif
    std::clog << '\n';

    std::clog << "[Flags]\n";
    std::clog << "BIT7Z_AUTO_FORMAT: " << flags::auto_format << '\n';
    std::clog << "BIT7Z_REGEX_MATCHING: " << flags::regex_matching << '\n';
    std::clog << "BIT7Z_USE_NATIVE_STRING: " << flags::native_string << '\n';
    std::clog << "BIT7Z_USE_STANDARD_FILESYSTEM: " << flags::standard_filesystem << "\n\n";
    std::clog.flush();

    return Catch::Session().run( argc, argv );
} catch( ... ) {
    return EXIT_FAILURE;
}