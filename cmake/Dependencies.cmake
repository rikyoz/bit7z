# Downloading the CPM.cmake package manager
set( CPM_DOWNLOAD_VERSION 0.38.5 )
set( CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake" )
if( NOT ( EXISTS ${CPM_DOWNLOAD_LOCATION} ))
    message( STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}" )
    file( DOWNLOAD
          https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
          ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include( ${CPM_DOWNLOAD_LOCATION} )

if( BIT7Z_7ZIP_VERSION )
    # 7-zip source code
    CPMAddPackage( NAME 7-zip
                   GITHUB_REPOSITORY "rikyoz/7-Zip"
                   VERSION ${BIT7Z_7ZIP_VERSION}
                   DOWNLOAD_ONLY YES )
    if( 7-zip_ADDED )
        message( STATUS "7-zip source code available at ${7-zip_SOURCE_DIR}" )
        add_library( 7-zip INTERFACE IMPORTED )
        target_include_directories( 7-zip INTERFACE "${7-zip_SOURCE_DIR}/CPP/" )
    endif()
endif()

# ghc::filesystem library
if( NOT USE_STANDARD_FILESYSTEM OR NOT STANDARD_FILESYSTEM_COMPILES OR BIT7Z_BUILD_TESTS )
    CPMAddPackage( NAME ghc_filesystem
                   GITHUB_REPOSITORY gulrak/filesystem
                   VERSION 1.5.14
                   DOWNLOAD_ONLY YES )
    if( ghc_filesystem_ADDED )
        message( STATUS "ghc::filesystem source code available at ${ghc_filesystem_SOURCE_DIR}" )
        add_library( ghc_filesystem INTERFACE IMPORTED )
        target_include_directories( ghc_filesystem SYSTEM INTERFACE ${ghc_filesystem_SOURCE_DIR}/include )
    endif()
endif()
