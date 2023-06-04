# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.14 )

option( BIT7Z_TESTS_FILESYSTEM "Enable or disable testing filesystem-related classes" ON )
message( STATUS "Testing filesystem-related classes: ${BIT7Z_TESTS_FILESYSTEM}" )

# sources
set( SOURCE_FILES
     src/main.cpp
     src/test_bit7zlibrary.cpp
     src/test_bitexception.cpp
     src/test_bitpropvariant.cpp
     src/test_bititemsvector.cpp
     src/test_cbufferinstream.cpp
     src/test_dateutil.cpp
     src/test_fsutil.cpp
     src/test_util.cpp
     src/test_windows.cpp )

set( TESTS_TARGET bit7z${ARCH_POSTFIX}-tests )
add_executable( ${TESTS_TARGET} ${SOURCE_FILES} )

if( BIT7Z_TESTS_FILESYSTEM )
    set( BIT7Z_TESTS_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data )

    # TODO: use execute_process to run git submodule directly
    if( NOT EXISTS ${BIT7Z_TESTS_DATA_DIR}/.git )
        include( FetchContent )
        FetchContent_Declare( bit7z-test-data
                              GIT_REPOSITORY https://github.com/rikyoz/bit7z-test-data.git
                              GIT_TAG main
                              SOURCE_DIR ${BIT7Z_TESTS_DATA_DIR} )

        FetchContent_MakeAvailable( bit7z-test-data )
    endif()

    message( STATUS "Tests data directory: ${BIT7Z_TESTS_DATA_DIR}" )
    target_compile_definitions( ${TESTS_TARGET} PRIVATE
                                BIT7Z_TESTS_FILESYSTEM
                                BIT7Z_TESTS_DATA_DIR="${BIT7Z_TESTS_DATA_DIR}" )
    if( NOT EXISTS ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/empty )
        file( MAKE_DIRECTORY ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/empty )
    endif()

    if( NOT EXISTS ${CMAKE_SOURCE_DIR}/test_filesystem/folder/subfolder )
        file( MAKE_DIRECTORY ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/folder/subfolder )
    endif()
else()
    # Bit7z employs the ghc::filesystem library on compilers that don't support C++17's filesystem.
    # To avoid linking errors due to mismatched filesystem libraries, it's important to use the same C++ standard
    # as the Bit7z library when testing any internal code related to the filesystem.
    # However, if filesystem-related code isn't being tested, we recommend using the C++11 standard
    # to ensure we test the maximum compatibility with C++ projects.
    set( CMAKE_CXX_STANDARD 11 )
    set( CMAKE_CXX_STANDARD_REQUIRED ON )
endif()

message( STATUS "Language standard used for the tests: C++${CMAKE_CXX_STANDARD}" )

option( BIT7Z_TESTS_USE_SYSTEM_7ZIP "Enable or disable using system's 7-zip shared library when executing tests" ON )
message( STATUS "Use system 7-zip for tests: ${BIT7Z_TESTS_USE_SYSTEM_7ZIP}" )
if( BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    target_compile_definitions( ${TESTS_TARGET} PRIVATE BIT7Z_TESTS_USE_SYSTEM_7ZIP )
endif()

# Avoiding linking unnecessary libraries.
# The main project's CMakeLists.txt should provide the needed libraries to link!
set( CMAKE_CXX_STANDARD_LIBRARIES "" )

target_link_libraries( ${TESTS_TARGET} PRIVATE ${LIB_TARGET} )
target_include_directories( ${TESTS_TARGET} PRIVATE
                            "${PROJECT_SOURCE_DIR}/include/bit7z"
                            "${PROJECT_SOURCE_DIR}/src"
                            "${BIT7Z_EXTERNAL_LIBS_DIR}"
                            "${BIT7Z_7ZIP_SOURCE_DIR}/CPP/" )

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wno-inconsistent-missing-override )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 3.8 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wdouble-promotion )
    endif()
endif()

include( cmake/Sanitizers.cmake )

include( CheckIPOSupported )
check_ipo_supported( RESULT ipo_supported OUTPUT ipo_error )
if(ipo_supported)
    message(STATUS "IPO is supported!")
    set_property(TARGET ${TESTS_TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${ipo_error}")
endif()

# Catch2
include( cmake/Catch2.cmake )
target_link_libraries( ${TESTS_TARGET} PRIVATE Catch2::Catch2 )

include( CTest )
include( Catch )
catch_discover_tests( ${TESTS_TARGET} )