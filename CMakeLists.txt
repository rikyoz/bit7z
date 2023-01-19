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
     include/bit7z/bit7z.hpp
     include/bit7z/bit7zlibrary.hpp
     include/bit7z/bitabstractarchivecreator.hpp
     include/bit7z/bitabstractarchivehandler.hpp
     include/bit7z/bitabstractarchiveopener.hpp
     include/bit7z/bitarchiveeditor.hpp
     include/bit7z/bitarchiveitem.hpp
     include/bit7z/bitarchiveiteminfo.hpp
     include/bit7z/bitarchiveitemoffset.hpp
     include/bit7z/bitarchivereader.hpp
     include/bit7z/bitarchivewriter.hpp
     include/bit7z/bitcompressionlevel.hpp
     include/bit7z/bitcompressionmethod.hpp
     include/bit7z/bitcompressor.hpp
     include/bit7z/bitdefines.hpp
     include/bit7z/biterror.hpp
     include/bit7z/bitexception.hpp
     include/bit7z/bitextractor.hpp
     include/bit7z/bitfilecompressor.hpp
     include/bit7z/bitfileextractor.hpp
     include/bit7z/bitformat.hpp
     include/bit7z/bitfs.hpp
     include/bit7z/bitgenericitem.hpp
     include/bit7z/bitinputarchive.hpp
     include/bit7z/bititemsvector.hpp
     include/bit7z/bitmemcompressor.hpp
     include/bit7z/bitmemextractor.hpp
     include/bit7z/bitoutputarchive.hpp
     include/bit7z/bitpropvariant.hpp
     include/bit7z/bitstreamcompressor.hpp
     include/bit7z/bitstreamextractor.hpp
     include/bit7z/bittypes.hpp
     include/bit7z/bitwindows.hpp )

# header files
set( HEADERS
     src/internal/archiveproperties.hpp
     src/internal/bufferextractcallback.hpp
     src/internal/bufferitem.hpp
     src/internal/bufferutil.hpp
     src/internal/callback.hpp
     src/internal/cbufferinstream.hpp
     src/internal/cbufferoutstream.hpp
     src/internal/cfileinstream.hpp
     src/internal/cfileoutstream.hpp
     src/internal/cfixedbufferoutstream.hpp
     src/internal/cmultivolumeinstream.hpp
     src/internal/cmultivolumeoutstream.hpp
     src/internal/cstdinstream.hpp
     src/internal/cstdoutstream.hpp
     src/internal/cvolumeinstream.hpp
     src/internal/cvolumeoutstream.hpp
     src/internal/dateutil.hpp
     src/internal/extractcallback.hpp
     src/internal/fileextractcallback.hpp
     src/internal/fixedbufferextractcallback.hpp
     src/internal/formatdetect.hpp
     src/internal/fsindexer.hpp
     src/internal/fsitem.hpp
     src/internal/fsutil.hpp
     src/internal/fs.hpp
     src/internal/genericinputitem.hpp
     src/internal/guiddef.hpp
     src/internal/guids.hpp
     src/internal/hresultcategory.hpp
     src/internal/internalcategory.hpp
     src/internal/macros.hpp
     src/internal/opencallback.hpp
     src/internal/processeditem.hpp
     src/internal/renameditem.hpp
     src/internal/stdinputitem.hpp
     src/internal/streamextractcallback.hpp
     src/internal/streamutil.hpp
     src/internal/updatecallback.hpp
     src/internal/util.hpp
     src/internal/windows.hpp )

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

# public includes
target_include_directories( ${LIB_TARGET} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                                 "$<INSTALL_INTERFACE:include>" )

# private includes
target_include_directories( ${LIB_TARGET} PRIVATE "${PROJECT_SOURCE_DIR}/include/bit7z"
                                                  "${PROJECT_SOURCE_DIR}/src"
                                                  "${EXTERNAL_LIBS_DIR}"
                                                  "${7ZIP_SOURCE_DIR}/CPP/" )

# 7-zip compilation definitions
target_compile_definitions( ${LIB_TARGET} PRIVATE UNICODE _UNICODE )
if( WIN32 )
    target_compile_definitions( ${LIB_TARGET} PRIVATE _WINDOWS )
    target_link_libraries( ${LIB_TARGET} PUBLIC oleaut32 )
endif()

if( UNIX )
    target_compile_definitions( ${LIB_TARGET} PRIVATE EXTERNAL_CODECS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT
                                                      ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_link_libraries( ${LIB_TARGET} PUBLIC ${CMAKE_DL_LIBS} )
endif()

# bit7z build options
include( cmake/BuildOptions.cmake )

# compiler-specific options
include( cmake/CompilerOptions.cmake )

if( BIT7Z_BUILD_TESTS )
    # tests
    add_subdirectory( tests )
endif()

if( BIT7Z_BUILD_DOCS )
    # docs
    add_subdirectory( docs )
endif()