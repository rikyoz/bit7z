/*
 * bit7z - A C++ static library to interface with the 7-zip shared libraries.
 * Copyright (c) 2014-2022 Riccardo Ostani - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <bitfs.hpp>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#include <climits> // for PATH_MAX
#include <unistd.h> // for readlink (Linux) or getpid (macOS)
#ifdef __APPLE__
#include <libproc.h> // for proc_pidpath and PROC_PIDPATHINFO_MAXSIZE
#endif
#endif

#include <internal/fs.hpp>

namespace bit7z { // NOLINT(modernize-concat-nested-namespaces)
namespace test {
namespace filesystem {

inline auto exe_path() -> fs::path {
#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW( nullptr, path, MAX_PATH );
    return path;
#else
#ifdef __APPLE__
    char result[PROC_PIDPATHINFO_MAXSIZE];
    ssize_t result_size = proc_pidpath(getpid(), result, sizeof(result));
#else
    char result[PATH_MAX];
    ssize_t result_size = readlink("/proc/self/exe", result, PATH_MAX);
#endif
    return result_size > 0 ? std::string(result, result_size) : "";
#endif
}

} // namespace filesystem
} // namespace test
} // namespace bit7z

#endif //FILESYSTEM_HPP
