#ifndef FS_HPP
#define FS_HPP

#define USE_STANDARD_FILESYSTEM \
        defined(__cpp_lib_filesystem) || \
        ( defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>) )

#if USE_STANDARD_FILESYSTEM
#include <fstream>
#include <filesystem>

namespace fs {
    using namespace std::filesystem;
    using ifstream = std::ifstream;
    using ofstream = std::ofstream;
    using fstream = std::fstream;
}
#else
#include "ghc/filesystem.hpp"

namespace fs {
    using namespace ghc::filesystem;
    using ifstream = ghc::filesystem::ifstream;
    using ofstream = ghc::filesystem::ofstream;
    using fstream = ghc::filesystem::fstream;
}
#endif

#endif // FS_HPP
