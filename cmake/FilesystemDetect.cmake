# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# checking if compiler supports the standard filesystem library

if( MINGW OR BIT7Z_DISABLE_USE_STD_FILESYSTEM )
    # Some versions of MinGW have a buggy std::filesystem that doesn't correctly handle paths with unicode characters,
    # so we are always using the ghc::filesystem library.
    set( USE_STANDARD_FILESYSTEM OFF )
else()
    set( CMAKE_CXX_STANDARD 17 )
    include( CheckIncludeFileCXX )
    check_include_file_cxx( "filesystem" USE_STANDARD_FILESYSTEM )
endif()

if( USE_STANDARD_FILESYSTEM )
    macro( check_filesystem_support flag )
        string( CONFIGURE [[
        #include <cstdlib>
        #include <filesystem>

        int main() {
            auto cwd = std::filesystem::current_path();
            printf("%s", cwd.c_str());
            return EXIT_SUCCESS;
        }
    ]] code @ONLY )
        if( CMAKE_CROSSCOMPILING )
            include( CheckCXXSourceCompiles )
            check_cxx_source_compiles( "${code}" ${flag} )
        else()
            include( CheckCXXSourceRuns )
            check_cxx_source_runs( "${code}" ${flag} )
        endif()
    endmacro()

    check_filesystem_support( STANDARD_FILESYSTEM_LINKS )
    if ( NOT STANDARD_FILESYSTEM_LINKS )
        set( ORIGINAL_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )
        set( CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_REQUIRED_LIBRARIES} -lstdc++fs )
        check_filesystem_support( STANDARD_FILESYSTEM_NEEDS_LIBSTDC++FS )
        if ( NOT STANDARD_FILESYSTEM_NEEDS_LIBSTDC++FS )
            set( CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_REQUIRED_LIBRARIES} -lc++fs )
            check_filesystem_support( STANDARD_FILESYSTEM_NEEDS_LIBC++FS )
            if ( NOT STANDARD_FILESYSTEM_NEEDS_LIBC++FS )
                set( USE_STANDARD_FILESYSTEM OFF )
                set( CMAKE_REQUIRED_LIBRARIES ${ORIGINAL_REQUIRED_LIBRARIES} )
            else()
                message( STATUS "Standard filesystem extra linker flags: -lc++fs" )
            endif()
        else()
            message( STATUS "Standard filesystem extra linker flags: -lstdc++fs" )
        endif()
    else()
        message( STATUS "Standard filesystem extra linker flags: None" )
    endif()
endif()

if( NOT USE_STANDARD_FILESYSTEM )
    # if standard filesystem lib is not supported, revert to C++14 standard and use the ghc::filesystem library
    set( CMAKE_CXX_STANDARD 14 )
    message( STATUS "Standard filesystem: NO (using ghc::filesystem)" )
else()
    message( STATUS "Standard filesystem: YES" )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )