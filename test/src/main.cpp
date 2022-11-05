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
#include <catch2/catch.hpp> //IMPORTANT: it must be included before any other include!

#include <iostream>

#include "compiler.hpp"
#include "flags.hpp"

int main( int argc, char* argv[] ) {
    using namespace bit7z::test;

    std::cout << "[Compiler]" << std::endl;
    std::cout << "Name: " << compiler::name << std::endl;
    std::cout << "Version: " << compiler::version << std::endl;
    std::cout << "Target Architecture: " << compiler::target_arch << std::endl << std::endl;

#ifdef _WIN32
    std::cout << "[Runtime]" << std::endl;
    std::cout << "Code page: " << GetACP() << std::endl << std::endl;
#endif

    std::cout << "[Flags]" << std::endl;
    std::cout << "BIT7Z_AUTO_FORMAT: " << flags::auto_format << std::endl;
    std::cout << "BIT7Z_REGEX_MATCHING: " << flags::regex_matching << std::endl;
    std::cout << "BIT7Z_USE_NATIVE_STRING: " << flags::native_string << std::endl << std::endl;

    return Catch::Session().run( argc, argv );
}