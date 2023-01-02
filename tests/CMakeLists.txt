# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# sources
set( SOURCE_FILES
     src/main.cpp
     src/test_bit7zlibrary.cpp
     src/test_bitexception.cpp
     src/test_bitpropvariant.cpp
     src/test_cbufferinstream.cpp
     src/test_dateutil.cpp
     src/test_fsutil.cpp
     src/test_windows.cpp )

set( TESTS_TARGET bit7z${ARCH_POSTFIX}-tests )
add_executable( ${TESTS_TARGET} ${SOURCE_FILES} )

option( BIT7Z_TESTS_USE_SYSTEM_7ZIP "Enable or disable using system's 7-zip shared library when executing tests" ON )
message( STATUS "Use system 7-zip for tests: ${BIT7Z_TESTS_USE_SYSTEM_7ZIP}" )
if( BIT7Z_TESTS_USE_SYSTEM_7ZIP )
    target_compile_definitions( ${TESTS_TARGET} PRIVATE BIT7Z_TESTS_USE_SYSTEM_7ZIP )
endif()

target_link_libraries( ${TESTS_TARGET} PRIVATE ${LIB_TARGET} )
target_include_directories( ${TESTS_TARGET} PRIVATE "${EXTERNAL_LIBS_DIR}" "${7ZIP_SOURCE_DIR}/CPP/")

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wno-inconsistent-missing-override )
    endif()
    if( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 3.8 )
        target_compile_options( ${TESTS_TARGET} PRIVATE -Wdouble-promotion )
    endif()
endif()

include( cmake/Sanitizers.cmake )

# Catch2
include( cmake/Catch2.cmake )
target_link_libraries( ${TESTS_TARGET} PRIVATE Catch2::Catch2 )

include( CTest )
include( Catch )
catch_discover_tests( ${TESTS_TARGET} )