# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# sources
set( SOURCE_FILES
     src/main.cpp
     src/test_bit7zlibrary.cpp )

set( TEST_TARGET bit7z${ARCH_POSTFIX}-tests )
add_executable( ${TEST_TARGET} ${SOURCE_FILES} )

option( BIT7Z_USE_SYSTEM_7ZIP "Enable or disable using system's 7-zip shared library when executing tests" )
message( STATUS "Use system 7-zip: ${BIT7Z_USE_SYSTEM_7ZIP}" )
if( BIT7Z_USE_SYSTEM_7ZIP )
    target_compile_definitions( ${TEST_TARGET} PRIVATE BIT7Z_USE_SYSTEM_7ZIP )
endif()

target_link_libraries( ${TEST_TARGET} PRIVATE ${LIB_TARGET} )

# Catch2
include( cmake/Catch2.cmake )
target_link_libraries( ${TEST_TARGET} PRIVATE Catch2::Catch2 )

include( CTest )
include( Catch )
catch_discover_tests( ${TEST_TARGET} )