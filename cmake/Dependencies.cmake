# Downloading the CPM.cmake package manager
set( CPM_DOWNLOAD_VERSION 0.42.3 )
set( CPM_DOWNLOAD_HASH "a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844" )
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

# When enabled, dependencies are resolved with find_package() first (e.g., as provided by a package
# manager such as vcpkg) and only downloaded via CPM if not found. Disabled by default so that a
# plain checkout builds out-of-the-box by fetching its pinned dependencies.
option( BIT7Z_USE_SYSTEM_DEPENDENCIES "Resolve dependencies via find_package when available" OFF )
message( STATUS "Use system dependencies: ${BIT7Z_USE_SYSTEM_DEPENDENCIES}" )
if( BIT7Z_USE_SYSTEM_DEPENDENCIES )
    set( CPM_USE_LOCAL_PACKAGES ON )
endif()

# No custom path to 7-Zip's source code was specified by the user, so we resolve it ourselves.
if( BIT7Z_CUSTOM_7ZIP_PATH STREQUAL "" )
    if( BIT7Z_USE_SYSTEM_DEPENDENCIES )
        # When using system dependencies, look for a 7-Zip package via find_package first. Unlike the
        # other dependencies, 7-Zip is searched for *without* a version: bit7z compiles against a specific
        # pinned 7-Zip source for the download path, but a system 7-Zip package (e.g., vcpkg's) may expose
        # a different version and may not even ship a config version file, in which case a versioned
        # find_package would be rejected. We therefore accept whatever 7-Zip the integrator provides.
        # (This is why 7-Zip cannot ride the generic CPM_USE_LOCAL_PACKAGES path, which would forward the
        # CPMAddPackage VERSION to find_package.)
        find_package( 7zip CONFIG REQUIRED )
    elseif ( BIT7Z_BUILD_FOR_P7ZIP )
        # p7zip source code (calling the package as 7-zip just for simplify the code).
        CPMAddPackage(
            NAME 7zip
            GITHUB_REPOSITORY "rikyoz/p7zip"
            VERSION ${BIT7Z_7ZIP_VERSION}
            GIT_PROGRESS ON
            DOWNLOAD_ONLY YES
        )
    else()
        # 7-zip source code
        CPMAddPackage(
            NAME 7zip
            GITHUB_REPOSITORY "rikyoz/7-Zip"
            VERSION ${BIT7Z_7ZIP_VERSION}
            GIT_PROGRESS ON
            DOWNLOAD_ONLY YES
        )
    endif()

    # Normalize to the "7-zip" target expected by the rest of the build, whether the source was
    # downloaded by CPM or 7zip was found via find_package.
    if( 7zip_ADDED )
        message( STATUS "7-zip source code available at ${7zip_SOURCE_DIR}" )
        add_library( 7-zip INTERFACE IMPORTED )
        target_include_directories( 7-zip INTERFACE "${7zip_SOURCE_DIR}/CPP/" )
    elseif( TARGET 7zip::7zip )
        message( STATUS "Using system 7-zip (7zip::7zip)" )
        add_library( 7-zip INTERFACE IMPORTED )
        target_link_libraries( 7-zip INTERFACE 7zip::7zip )
    else()
        message( FATAL_ERROR "7zip was not resolved, neither via CPM.cmake nor via find_package" )
    endif()
endif()

# ghc::filesystem library
# Note: the public API tests app compiles using the C++11 standard, so it always needs ghc::filesystem.
if( NOT USE_STANDARD_FILESYSTEM OR BIT7Z_BUILD_TESTS )
    CPMAddPackage(
        NAME ghc_filesystem
        GITHUB_REPOSITORY rikyoz/filesystem
        GIT_TAG b99c2aebd5ddd6fb2f190731ba80b949fc3842b5
        GIT_PROGRESS ON
        DOWNLOAD_ONLY YES
    )

    # Normalize to a plain "ghc_filesystem" target that the rest of the build and the tests link,
    # regardless of whether it came from the downloaded sources or from find_package.
    if( ghc_filesystem_ADDED )
        # Downloaded sources: expose the header-only library straight from its source tree.
        message( STATUS "ghc::filesystem source code available at ${ghc_filesystem_SOURCE_DIR}" )
        add_library( ghc_filesystem INTERFACE IMPORTED )
        target_include_directories( ghc_filesystem SYSTEM INTERFACE ${ghc_filesystem_SOURCE_DIR}/include )
        if( BIT7Z_USE_PCH AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16.0" )
            target_precompile_headers(
                ghc_filesystem INTERFACE "${ghc_filesystem_SOURCE_DIR}/include/ghc/filesystem.hpp"
            )
        endif()
    elseif( TARGET ghcFilesystem::ghc_filesystem )
        # Found via find_package: wrap the namespaced config target under the plain "ghc_filesystem"
        # name expected by the rest of the build. An INTERFACE wrapper (not an ALIAS) is used because
        # aliasing an imported target requires it to be GLOBAL, which find_package targets are not by
        # default.
        message( STATUS "Using system ghc::filesystem (ghcFilesystem::ghc_filesystem)" )
        add_library( ghc_filesystem INTERFACE IMPORTED )
        target_link_libraries( ghc_filesystem INTERFACE ghcFilesystem::ghc_filesystem )
    else()
        message( FATAL_ERROR "ghc_filesystem was not resolved, neither via CPM.cmake nor via find_package" )
    endif()
endif()

# Filesystem usage requirements for the bit7z target:
#  - modern standard libraries need no extra library;
#  - older libstdc++/libc++ require the consumer to also link the stdc++fs/c++fs support library.
#    This is a genuine consumer requirement, so it is linked directly to the bit7z target as a plain
#    library name (PUBLIC) so that it propagates to consumers and survives installation/export;
#  - libraries without a usable std::filesystem fall back to ghc::filesystem, which is only needed
#    while compiling bit7z's own sources (the public API merely forward-declares its types). It is
#    exposed through the build-only filesystem_lib target, which the test targets also consume.
add_library( filesystem_lib INTERFACE IMPORTED )
if( USE_STANDARD_FILESYSTEM )
    if( STANDARD_FILESYSTEM_NEEDS_LIBSTDC++FS )
        target_link_libraries( ${LIB_TARGET} PUBLIC stdc++fs )
    elseif( STANDARD_FILESYSTEM_NEEDS_LIBC++FS )
        target_link_libraries( ${LIB_TARGET} PUBLIC c++fs )
    endif()
else()
    target_link_libraries( filesystem_lib INTERFACE ghc_filesystem )
endif()
