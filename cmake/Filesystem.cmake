# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# checking if compiler supports the standard filesystem library

if ( MINGW )
    # Some versions of MinGW have a buggy std::filesystem that doesn't correctly handle paths with unicode characters,
    # so we are always using the ghc::filesystem library.
    set( USE_STANDARD_FILESYSTEM OFF )
else()
    set( CMAKE_CXX_STANDARD 17 )
    include( CheckIncludeFileCXX )
    check_include_file_cxx( "filesystem" USE_STANDARD_FILESYSTEM )
endif()

if( USE_STANDARD_FILESYSTEM )
    include( CheckCXXSourceCompiles )
    string( CONFIGURE [[
        #include <cstdlib>
        #include <filesystem>

        int main() {
            auto cwd = std::filesystem::current_path();
            printf("%s", cwd.c_str());
            return EXIT_SUCCESS;
        }
    ]] code @ONLY )
    check_cxx_source_compiles( "${code}" STANDARD_FILESYSTEM_COMPILES )
endif()

if( NOT USE_STANDARD_FILESYSTEM OR NOT STANDARD_FILESYSTEM_COMPILES )
    set( CMAKE_CXX_STANDARD 14 ) # if standard filesystem lib is not supported, revert to C++14 standard

    if( NOT EXISTS ${BIT7Z_EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp )
        include( ExternalProject )

        # downloading ghc::filesystem as an alternative to std::filesystem
        file( DOWNLOAD
              https://raw.githubusercontent.com/gulrak/filesystem/master/include/ghc/filesystem.hpp
              ${BIT7Z_EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp
              SHOW_PROGRESS
              TLS_VERIFY ON )
    endif()
    message( STATUS "Standard filesystem: NO (using ghc::filesystem)" )
else()
    message( STATUS "Standard filesystem: YES" )
endif()