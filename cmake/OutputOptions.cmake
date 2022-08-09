# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

if( CMAKE_CONFIGURATION_TYPES ) # enable only debug/release configurations for generated VS project file
    set( CMAKE_CONFIGURATION_TYPES Debug Release )
    set( CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
         "Reset the configurations to what we need" FORCE )
endif()

if( NOT CMAKE_BUILD_TYPE ) # by default, use release build
    set( CMAKE_BUILD_TYPE "Release" )
endif()

option( BIT7Z_VS_LIBNAME_OUTDIR_STYLE
        "Force using Visual Studio output library name and directory structure convention" )

# architecture-specific options
set( ARCH_POSTFIX "" )
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( ARCH_DIR "x64" )
    if( WIN32 )
        add_definitions( -DWIN64 )
    endif()
    if( NOT GENERATOR_IS_MULTI_CONFIG AND NOT BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
        set( ARCH_POSTFIX 64 )
    endif()
else()
    set( ARCH_DIR "x86" )
endif()

if( NOT GENERATOR_IS_MULTI_CONFIG AND BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
    # forcing output directory to ${BIT7Z_DIR}/lib/${ARCH_DIR}/${CMAKE_BUILD_TYPE} (e.g. ./lib/x64/Release)
    set( OUTPUT_DIR lib/${ARCH_DIR}/${CMAKE_BUILD_TYPE}/ )
else()
    # forcing output directory to ${BIT7Z_DIR}/lib/${ARCH_DIR}/ (e.g. ./lib/x64/)
    # Note: Visual Studio will append ${CMAKE_BUILD_TYPE} to ${OUTPUT_DIR}.
    set( OUTPUT_DIR lib/${ARCH_DIR}/ )
endif()

# Note: not applied in generated Visual Studio project files (e.g., .vcxproj)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${OUTPUT_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${OUTPUT_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${OUTPUT_DIR} )

if( NOT GENERATOR_IS_MULTI_CONFIG AND NOT BIT7Z_VS_LIBNAME_OUTDIR_STYLE )
    set( CMAKE_DEBUG_POSTFIX "_d" ) # debug library file name should end with "_d" (e.g. bit7z_d.lib)
endif()