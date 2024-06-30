# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.14 )

option( BIT7Z_TESTS_FILESYSTEM "Enable or disable testing filesystem-related classes" ON )
message( STATUS "Testing filesystem-related classes: ${BIT7Z_TESTS_FILESYSTEM}" )

# base source files
set( SOURCE_FILES
     src/utils/archive.cpp
     src/utils/filesystem.cpp
     src/utils/crc.cpp
     src/main.cpp )

# public API test sources
set( PUBLIC_API_SOURCE_FILES
     src/test_bit7zlibrary.cpp
     src/test_bitabstractarchivecreator.cpp
     src/test_bitarchiveeditor.cpp
     src/test_bitarchiveiteminfo.cpp
     src/test_bitarchivereader.cpp
     src/test_biterror.cpp
     src/test_bitexception.cpp
     src/test_bitfilecompressor.cpp
     src/test_bitfileextractor.cpp
     src/test_bitinputarchive.cpp
     src/test_bitmemcompressor.cpp
     src/test_bitmemextractor.cpp
     src/test_bitnestedarchivereader.cpp
     src/test_bitoutputarchive.cpp
     src/test_bitpropvariant.cpp
     src/test_bitstreamcompressor.cpp
     src/test_bitstreamextractor.cpp )

# internal API sources
set( INTERNAL_API_SOURCE_FILES
     src/test_bititemsvector.cpp # BitItemsVector is not meant to be used by the user
     src/test_cbufferinstream.cpp
     src/test_dateutil.cpp
     src/test_fsutil.cpp
     src/test_util.cpp
     src/test_stringutil.cpp
     src/test_windows.cpp
     src/test_formatdetect.cpp )

set( TESTS_TARGET bit7z-tests )
add_executable( ${TESTS_TARGET} ${SOURCE_FILES} ${PUBLIC_API_SOURCE_FILES} ${INTERNAL_API_SOURCE_FILES} )

set( TESTS_TARGET_PUBLIC bit7z-tests-public )
add_executable( ${TESTS_TARGET_PUBLIC} ${SOURCE_FILES} ${PUBLIC_API_SOURCE_FILES} )

if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
    set( TESTS_TARGET_PUBLIC_CPP20 bit7z-tests-public-cpp20 )
    add_executable( ${TESTS_TARGET_PUBLIC_CPP20} ${SOURCE_FILES} ${PUBLIC_API_SOURCE_FILES} )
endif()

if( BIT7Z_TESTS_FILESYSTEM )
    CPMAddPackage( NAME bit7z_test_data
                   GITHUB_REPOSITORY "rikyoz/bit7z-test-data"
                   VERSION 1.8
                   DOWNLOAD_ONLY YES )
    if( bit7z_test_data_ADDED )
        set( BIT7Z_TESTS_DATA_DIR ${bit7z_test_data_SOURCE_DIR} )
        message( STATUS "Tests data directory: ${BIT7Z_TESTS_DATA_DIR}" )
    endif()

    target_compile_definitions( ${TESTS_TARGET} PRIVATE
                                BIT7Z_TESTS_FILESYSTEM
                                BIT7Z_TESTS_DATA_DIR="${BIT7Z_TESTS_DATA_DIR}" )
    target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE
                                BIT7Z_TESTS_PUBLIC_API_ONLY
                                BIT7Z_TESTS_FILESYSTEM
                                BIT7Z_TESTS_DATA_DIR="${BIT7Z_TESTS_DATA_DIR}" )

    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE
                                    BIT7Z_TESTS_PUBLIC_API_ONLY
                                    BIT7Z_TESTS_FILESYSTEM
                                    BIT7Z_TESTS_DATA_DIR="${BIT7Z_TESTS_DATA_DIR}" )
    endif()

    if( NOT EXISTS ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/empty )
        file( MAKE_DIRECTORY ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/empty )
    endif()

    if( NOT EXISTS ${CMAKE_SOURCE_DIR}/test_filesystem/folder/subfolder )
        file( MAKE_DIRECTORY ${BIT7Z_TESTS_DATA_DIR}/test_filesystem/folder/subfolder )
    endif()

    if( NOT USE_STANDARD_FILESYSTEM OR NOT STANDARD_FILESYSTEM_COMPILES )
        target_link_libraries( ${TESTS_TARGET} PRIVATE ghc_filesystem )
    endif()

    set_property( TARGET ${TESTS_TARGET_PUBLIC} PROPERTY CXX_STANDARD 11 )
    set_property( TARGET ${TESTS_TARGET_PUBLIC} PROPERTY CXX_STANDARD_REQUIRED ON )
    target_link_libraries( ${TESTS_TARGET_PUBLIC} PRIVATE ghc_filesystem )

    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        set_property( TARGET ${TESTS_TARGET_PUBLIC_CPP20} PROPERTY CXX_STANDARD 20 )
        set_property( TARGET ${TESTS_TARGET_PUBLIC_CPP20} PROPERTY CXX_STANDARD_REQUIRED ON )
        target_link_libraries( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE ghc_filesystem )
    endif()
else()
    # Bit7z employs the ghc::filesystem library on compilers that don't support C++17's filesystem.
    # To avoid linking errors due to mismatched filesystem libraries, it's important to use the same C++ standard
    # as the Bit7z library when testing any internal code related to the filesystem.
    # However, if filesystem-related code isn't being tested, we recommend using the C++11 standard
    # to ensure we test the maximum compatibility with C++ projects.
    set( CMAKE_CXX_STANDARD 11 )
    set( CMAKE_CXX_STANDARD_REQUIRED ON )

    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        set_property( TARGET ${TESTS_TARGET_PUBLIC_CPP20} PROPERTY CXX_STANDARD 20 )
        set_property( TARGET ${TESTS_TARGET_PUBLIC_CPP20} PROPERTY CXX_STANDARD_REQUIRED ON )
        target_link_libraries( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE ghc_filesystem )
    endif()
endif()

message( STATUS "Language standard used for the tests: C++${CMAKE_CXX_STANDARD}" )

option( BIT7Z_TESTS_USE_SYSTEM_7ZIP "Enable or disable using system's 7-zip shared library when executing tests" ON )
message( STATUS "Use system 7-zip for tests: ${BIT7Z_TESTS_USE_SYSTEM_7ZIP}" )
if( BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    target_compile_definitions( ${TESTS_TARGET} PRIVATE BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    endif()
endif()

# Avoiding linking unnecessary libraries.
# The main project's CMakeLists.txt should provide the needed libraries to link!
set( CMAKE_CXX_STANDARD_LIBRARIES "" )

target_link_libraries( ${TESTS_TARGET} PRIVATE ${LIB_TARGET} 7-zip )
target_link_libraries( ${TESTS_TARGET_PUBLIC} PRIVATE ${LIB_TARGET} 7-zip )
if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
    target_link_libraries( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE ${LIB_TARGET} 7-zip )
endif()
target_include_directories( ${TESTS_TARGET} PRIVATE
                            "${PROJECT_SOURCE_DIR}/include/bit7z"
                            "${PROJECT_SOURCE_DIR}/src" )
target_include_directories( ${TESTS_TARGET_PUBLIC} PRIVATE
                            "${PROJECT_SOURCE_DIR}/include/bit7z"
                            "${PROJECT_SOURCE_DIR}/src" )
if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
    target_include_directories( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE
                                "${PROJECT_SOURCE_DIR}/include/bit7z"
                                "${PROJECT_SOURCE_DIR}/src" )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
    target_link_libraries( ${TESTS_TARGET} PRIVATE ${LIB_TARGET} pthread )
    target_link_libraries( ${TESTS_TARGET_PUBLIC} PRIVATE ${LIB_TARGET} pthread )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wno-inconsistent-missing-override )
        target_compile_options( ${TESTS_TARGET_PUBLIC} PRIVATE -Wno-inconsistent-missing-override )
        if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
            target_compile_options( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE -Wno-inconsistent-missing-override )
        endif()
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 3.8 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wdouble-promotion )
        target_compile_options( ${TESTS_TARGET_PUBLIC} PRIVATE -Wdouble-promotion )
        if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
            target_compile_options( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE -Wdouble-promotion )
        endif()
    endif()
endif()

if( CMAKE_GENERATOR MATCHES "Visual Studio" )
    set_property( DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TESTS_TARGET} )
endif()

if( MSVC )
    target_compile_options( ${TESTS_TARGET} PRIVATE /utf-8 )
    target_compile_options( ${TESTS_TARGET_PUBLIC} PRIVATE /utf-8 )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_options( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE /utf-8 )
    endif()

    target_compile_definitions( ${TESTS_TARGET} PRIVATE _CRT_DECLARE_NONSTDC_NAMES=1 )
    target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE _CRT_DECLARE_NONSTDC_NAMES=1 )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE _CRT_DECLARE_NONSTDC_NAMES=1 )
    endif()
else()
    target_compile_options( ${TESTS_TARGET} PRIVATE -Wno-deprecated -Wno-deprecated-declarations )
    target_compile_options( ${TESTS_TARGET_PUBLIC} PRIVATE -Wno-deprecated -Wno-deprecated-declarations )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_options( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE -Wno-deprecated -Wno-deprecated-declarations )
    endif()
endif()

if( WIN32 )
    if( MINGW )
        set( CMAKE_RC_COMPILER_INIT windres )
        SET( CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>" )
        ENABLE_LANGUAGE( RC )
        target_compile_definitions( ${TESTS_TARGET} PRIVATE WINDRES NTDDI_VERSION=NTDDI_VISTA )
        target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE WINDRES NTDDI_VERSION=NTDDI_VISTA )
        if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
            target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE WINDRES NTDDI_VERSION=NTDDI_VISTA )
        endif()
        set( RES_EXTENSION "rc" )
    else()
        set( RES_EXTENSION "manifest" )
    endif()

    message( STATUS "Using legacy string encoding for tests: ON" )
    target_sources( ${TESTS_TARGET} PRIVATE res/encodingLegacy.${RES_EXTENSION} )
    target_sources( ${TESTS_TARGET_PUBLIC} PRIVATE res/encodingLegacy.${RES_EXTENSION} )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_sources( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE res/encodingLegacy.${RES_EXTENSION} )
    endif()

    target_compile_definitions( ${TESTS_TARGET} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX )
    target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX )
    endif()

    target_link_libraries( ${TESTS_TARGET} PRIVATE ${LIB_TARGET} shell32 ole32 )
    target_link_libraries( ${TESTS_TARGET_PUBLIC} PRIVATE ${LIB_TARGET} shell32 ole32 )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_link_libraries( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE ${LIB_TARGET} shell32 ole32 )
    endif()
endif()

if ( APPLE AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64" )
    target_link_options( ${TESTS_TARGET} PRIVATE -Wl,-ld_classic )
    target_link_options( ${TESTS_TARGET_PUBLIC} PRIVATE -Wl,-ld_classic )
    if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
        target_link_options( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE -Wl,-ld_classic )
    endif()
endif()

# Catch2
CPMAddPackage( NAME Catch2
               GIT_TAG 359e3ea5c87f6226686c70e48e576db21e07ed94
               OPTIONS "CATCH_CONFIG_CONSOLE_WIDTH 512" "CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS ON"
               GITHUB_REPOSITORY "rikyoz/Catch2" )
list( APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/contrib" )

include( CTest )
include( Catch )
target_link_libraries( ${TESTS_TARGET} PRIVATE Catch2::Catch2 )
target_link_libraries( ${TESTS_TARGET_PUBLIC} PRIVATE Catch2::Catch2 )
if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
    target_link_libraries( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE Catch2::Catch2 )
endif()
target_compile_definitions( ${TESTS_TARGET} PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=512 )
target_compile_definitions( ${TESTS_TARGET_PUBLIC} PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=512 )
if( "cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES )
    target_compile_definitions( ${TESTS_TARGET_PUBLIC_CPP20} PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=512 )
endif()
catch_discover_tests( ${TESTS_TARGET} )