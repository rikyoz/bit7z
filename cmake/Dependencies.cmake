# Downloading the CPM.cmake package manager
set( CPM_DOWNLOAD_VERSION 0.38.2 )
set( CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake" )
if( NOT ( EXISTS ${CPM_DOWNLOAD_LOCATION} ))
    message( STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}" )
    file( DOWNLOAD
          https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
          ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include( ${CPM_DOWNLOAD_LOCATION} )

CPMAddPackage( NAME 7-zip
               GITHUB_REPOSITORY "rikyoz/7-Zip"
               VERSION 23.01
               DOWNLOAD_ONLY YES )
if( 7-zip_ADDED )
    message( STATUS "7-zip source code available at ${7-zip_SOURCE_DIR}" )
    add_library( 7-zip INTERFACE )
    target_include_directories( 7-zip INTERFACE "${7-zip_SOURCE_DIR}/CPP/" )
endif()