# Downloading the CPM.cmake package manager
set( CPM_DOWNLOAD_VERSION 0.40.2 )
set( CPM_DOWNLOAD_HASH "c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d" )
if( CPM_SOURCE_CACHE )
    set( CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake" )
elseif( DEFINED ENV{CPM_SOURCE_CACHE} )
    set( CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake" )
else()
    set( CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake" )
endif()
get_filename_component( CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE )
set( CPM_USE_NAMED_CACHE_DIRECTORIES ON )
if( NOT ( EXISTS ${CPM_DOWNLOAD_LOCATION} ) )
    message( STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}" )
    file( DOWNLOAD
          https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
          ${CPM_DOWNLOAD_LOCATION}
          SHOW_PROGRESS
          EXPECTED_HASH SHA256=${CPM_DOWNLOAD_HASH}
    )
endif()
include( ${CPM_DOWNLOAD_LOCATION} )

# No custom path to 7-Zip's source code was specified by the user, so we download it.
if( BIT7Z_CUSTOM_7ZIP_PATH STREQUAL "" )
    if ( BIT7Z_BUILD_FOR_P7ZIP )
        # p7zip source code (calling the package as 7-zip just for simplify the code).
        CPMAddPackage( NAME 7-zip
                       GITHUB_REPOSITORY "rikyoz/p7zip"
                       VERSION ${BIT7Z_7ZIP_VERSION}
                       GIT_PROGRESS ON
                       DOWNLOAD_ONLY YES )
    else()
        # 7-zip source code
        CPMAddPackage( NAME 7-zip
                       GITHUB_REPOSITORY "rikyoz/7-Zip"
                       VERSION ${BIT7Z_7ZIP_VERSION}
                       GIT_PROGRESS ON
                       DOWNLOAD_ONLY YES )
    endif()
    if( 7-zip_ADDED )
        message( STATUS "7-zip source code available at ${7-zip_SOURCE_DIR}" )
        add_library( 7-zip INTERFACE IMPORTED )
        target_include_directories( 7-zip INTERFACE "${7-zip_SOURCE_DIR}/CPP/" )
    endif()
endif()

# ghc::filesystem library
# Note: the public API tests app compiles using the C++11 standard, so it will always needs ghc::filesystem
if( NOT USE_STANDARD_FILESYSTEM OR BIT7Z_BUILD_TESTS )
    CPMAddPackage( NAME ghc_filesystem
                   GITHUB_REPOSITORY rikyoz/filesystem
                   GIT_TAG b99c2aebd5ddd6fb2f190731ba80b949fc3842b5
                   GIT_PROGRESS ON
                   DOWNLOAD_ONLY YES )
    if( ghc_filesystem_ADDED )
        message( STATUS "ghc::filesystem source code available at ${ghc_filesystem_SOURCE_DIR}" )
        add_library( ghc_filesystem INTERFACE IMPORTED )
        target_include_directories( ghc_filesystem SYSTEM INTERFACE ${ghc_filesystem_SOURCE_DIR}/include )
        if( BIT7Z_USE_PCH AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0" )
            target_precompile_headers( ghc_filesystem INTERFACE
                                       "${ghc_filesystem_SOURCE_DIR}/include/ghc/filesystem.hpp" )
        endif()
    endif()
endif()

if( USE_STANDARD_FILESYSTEM )
    add_library( filesystem_lib INTERFACE IMPORTED )
    if( STANDARD_FILESYSTEM_LINKS )
        # No extra library to link.
    elseif( STANDARD_FILESYSTEM_NEEDS_LIBSTDC++FS )
        target_link_libraries( filesystem_lib INTERFACE stdc++fs )
    elseif( STANDARD_FILESYSTEM_NEEDS_LIBC++FS )
        target_link_libraries( filesystem_lib INTERFACE c++fs )
    endif()
else()
    add_library( filesystem_lib ALIAS ghc_filesystem )
endif()
