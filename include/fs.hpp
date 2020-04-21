#ifndef FS_HPP
#define FS_HPP

#if defined(__cpp_lib_filesystem) || \
    ( defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>) )

#include <fstream>
#include <filesystem>

namespace fs {
    using namespace std::filesystem;
    using ifstream = std::ifstream;
    using ofstream = std::ofstream;
    using fstream = std::fstream;
}
#else
#define GHC_WIN_WSTRING_STRING_TYPE
#include "ghc/filesystem.hpp"

namespace fs {
    using namespace ghc::filesystem;
    using ifstream = ghc::filesystem::ifstream;
    using ofstream = ghc::filesystem::ofstream;
    using fstream = ghc::filesystem::fstream;
}
#endif

#endif // FS_HPP
