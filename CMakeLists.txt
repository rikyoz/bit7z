# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.11 )

project( bit7z
         VERSION 4.0.0
         DESCRIPTION "A C++ static library offering a clean and simple interface to the 7-zip/p7zip shared libraries"
         HOMEPAGE_URL "https://github.com/rikyoz/bit7z/" )
set( CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" )

cmake_policy( SET CMP0022 NEW )
if( POLICY CMP0076 )
    cmake_policy( SET CMP0076 NEW )
endif()

set( EXTERNAL_LIBS_DIR ${PROJECT_SOURCE_DIR}/third_party )
set( 7ZIP_SOURCE_DIR ${EXTERNAL_LIBS_DIR}/7-zip )

# public headers
set( PUBLIC_HEADERS
     include/bit7z.hpp
     include/bit7zlibrary.hpp
     include/bitabstractarchivecreator.hpp
     include/bitabstractarchivehandler.hpp
     include/bitabstractarchiveopener.hpp
     include/bitarchiveeditor.hpp
     include/bitarchiveitem.hpp
     include/bitarchiveiteminfo.hpp
     include/bitarchiveitemoffset.hpp
     include/bitarchivereader.hpp
     include/bitarchivewriter.hpp
     include/bitcompressionlevel.hpp
     include/bitcompressionmethod.hpp
     include/bitcompressor.hpp
     include/bitdefines.hpp
     include/biterror.hpp
     include/bitexception.hpp
     include/bitextractor.hpp
     include/bitfilecompressor.hpp
     include/bitfileextractor.hpp
     include/bitformat.hpp
     include/bitfs.hpp
     include/bitgenericitem.hpp
     include/bitinputarchive.hpp
     include/bititemsvector.hpp
     include/bitmemcompressor.hpp
     include/bitmemextractor.hpp
     include/bitoutputarchive.hpp
     include/bitpropvariant.hpp
     include/bitstreamcompressor.hpp
     include/bitstreamextractor.hpp
     include/bittypes.hpp
     include/bitwindows.hpp )

# header files
set( HEADERS
     include/internal/bufferextractcallback.hpp
     include/internal/bufferitem.hpp
     include/internal/bufferutil.hpp
     include/internal/callback.hpp
     include/internal/cbufferinstream.hpp
     include/internal/cbufferoutstream.hpp
     include/internal/cfileinstream.hpp
     include/internal/cfileoutstream.hpp
     include/internal/cfixedbufferoutstream.hpp
     include/internal/cmultivolumeinstream.hpp
     include/internal/cmultivolumeoutstream.hpp
     include/internal/cstdinstream.hpp
     include/internal/cstdoutstream.hpp
     include/internal/cvolumeinstream.hpp
     include/internal/cvolumeoutstream.hpp
     include/internal/dateutil.hpp
     include/internal/extractcallback.hpp
     include/internal/fileextractcallback.hpp
     include/internal/fixedbufferextractcallback.hpp
     include/internal/formatdetect.hpp
     include/internal/fsindexer.hpp
     include/internal/fsitem.hpp
     include/internal/fsutil.hpp
     include/internal/fs.hpp
     include/internal/genericinputitem.hpp
     include/internal/guiddef.hpp
     include/internal/guids.hpp
     include/internal/hresultcategory.hpp
     include/internal/internalcategory.hpp
     include/internal/macros.hpp
     include/internal/opencallback.hpp
     include/internal/processeditem.hpp
     include/internal/renameditem.hpp
     include/internal/stdinputitem.hpp
     include/internal/streamextractcallback.hpp
     include/internal/streamutil.hpp
     include/internal/updatecallback.hpp
     include/internal/util.hpp
     include/internal/windows.hpp )

# source files
set( SOURCES
     src/bit7zlibrary.cpp
     src/bitabstractarchivecreator.cpp
     src/bitabstractarchivehandler.cpp
     src/bitabstractarchiveopener.cpp
     src/bitarchiveeditor.cpp
     src/bitarchiveitem.cpp
     src/bitarchiveiteminfo.cpp
     src/bitarchiveitemoffset.cpp
     src/bitarchivereader.cpp
     src/bitarchivewriter.cpp
     src/biterror.cpp
     src/bitexception.cpp
     src/bitfilecompressor.cpp
     src/bitformat.cpp
     src/bitinputarchive.cpp
     src/bititemsvector.cpp
     src/bitoutputarchive.cpp
     src/bitpropvariant.cpp
     src/internal/bufferextractcallback.cpp
     src/internal/bufferitem.cpp
     src/internal/bufferutil.cpp
     src/internal/callback.cpp
     src/internal/cbufferinstream.cpp
     src/internal/cbufferoutstream.cpp
     src/internal/cfileinstream.cpp
     src/internal/cfileoutstream.cpp
     src/internal/cfixedbufferoutstream.cpp
     src/internal/cmultivolumeinstream.cpp
     src/internal/cmultivolumeoutstream.cpp
     src/internal/cstdinstream.cpp
     src/internal/cstdoutstream.cpp
     src/internal/cvolumeinstream.cpp
     src/internal/cvolumeoutstream.cpp
     src/internal/dateutil.cpp
     src/internal/extractcallback.cpp
     src/internal/fileextractcallback.cpp
     src/internal/fixedbufferextractcallback.cpp
     src/internal/formatdetect.cpp
     src/internal/fsindexer.cpp
     src/internal/fsitem.cpp
     src/internal/fsutil.cpp
     src/internal/genericinputitem.cpp
     src/internal/guids.cpp
     src/internal/hresultcategory.cpp
     src/internal/internalcategory.cpp
     src/internal/opencallback.cpp
     src/internal/processeditem.cpp
     src/internal/renameditem.cpp
     src/internal/stdinputitem.cpp
     src/internal/streamextractcallback.cpp
     src/internal/updatecallback.cpp
     src/internal/util.cpp
     src/internal/windows.cpp )

# library output file name options
include( cmake/OutputOptions.cmake )

# filesystem library
include( cmake/Filesystem.cmake )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# printing library and compiler information (useful for debugging)
message( STATUS "Target Version: ${CMAKE_PROJECT_VERSION}" )
message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Architecture: ${ARCH_DIR}" )
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )
message( STATUS "Language Standard: C++${CMAKE_CXX_STANDARD}" )

# library target options
set( LIB_TARGET bit7z${ARCH_POSTFIX} )
add_library( ${LIB_TARGET} STATIC )
target_sources( ${LIB_TARGET}
                PUBLIC ${PUBLIC_HEADERS}
                PRIVATE ${HEADERS} ${SOURCES} )

target_include_directories( ${LIB_TARGET} PUBLIC "${PROJECT_SOURCE_DIR}/include/" ) # public includes
target_include_directories( ${LIB_TARGET} PRIVATE "${EXTERNAL_LIBS_DIR}" "${7ZIP_SOURCE_DIR}/CPP/" ) # private includes

# 7-zip compilation definitions
target_compile_definitions( ${LIB_TARGET} PRIVATE UNICODE _UNICODE )
if( WIN32 )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _WINDOWS )
endif()

if( UNIX )
    target_compile_definitions( ${LIB_TARGET} PRIVATE EXTERNAL_CODECS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT
                                ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_link_libraries( ${LIB_TARGET} ${CMAKE_DL_LIBS} )
endif()

# bit7z build options
include( cmake/BuildOptions.cmake )

# compiler-specific options
include( cmake/CompilerOptions.cmake )

if( BIT7Z_BUILD_TESTS )
    # tests
    add_subdirectory( tests )
endif()