cmake_minimum_required( VERSION 3.8.2 )
project( bit7z )

set( CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" )
cmake_policy( SET CMP0022 NEW )
set( EXTERNAL_LIBS_DIR ${PROJECT_SOURCE_DIR}/third_party )

if( WIN32 )
    set( 7ZIP_SOURCE_DIR ${EXTERNAL_LIBS_DIR}/7-zip )
else()
    set( 7ZIP_SOURCE_DIR ${EXTERNAL_LIBS_DIR}/p7zip )
endif()

# headers
set( HEADERS
     ${PROJECT_SOURCE_DIR}/include/bit7z.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7zlibrary.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivecreator.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveeditor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivehandler.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivereader.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveiteminfo.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveitemoffset.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveopener.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivewriter.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressionlevel.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressionmethod.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitexception.hpp
     ${PROJECT_SOURCE_DIR}/include/bitextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfilecompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfileextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitformat.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfs.hpp
     ${PROJECT_SOURCE_DIR}/include/bitgenericitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bitguids.hpp
     ${PROJECT_SOURCE_DIR}/include/bitinputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bititemsvector.hpp
     ${PROJECT_SOURCE_DIR}/include/bitmemcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitmemextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitoutputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bitpropvariant.hpp
     ${PROJECT_SOURCE_DIR}/include/bitstreamcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitstreamextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bittypes.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/bufferextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/bufferitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/callback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cbufferinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cbufferoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cfileinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cfileoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cmultivoloutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cstdinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cstdoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/extractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fileextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/formatdetect.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsindexer.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsutil.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fs.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/genericstreamitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/hresultcategory.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/opencallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/renameditem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/streamextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/stdstreamitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/updatecallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/util.hpp )

# sources
set( SOURCES
     ${PROJECT_SOURCE_DIR}/src/bit7zlibrary.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivecreator.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveeditor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivehandler.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivereader.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitem.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveiteminfo.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitemoffset.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveopener.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivewriter.cpp
     ${PROJECT_SOURCE_DIR}/src/bitexception.cpp
     ${PROJECT_SOURCE_DIR}/src/bitfilecompressor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitformat.cpp
     ${PROJECT_SOURCE_DIR}/src/bitgenericitem.cpp
     ${PROJECT_SOURCE_DIR}/src/bitguids.cpp
     ${PROJECT_SOURCE_DIR}/src/bitinputarchive.cpp
     ${PROJECT_SOURCE_DIR}/src/bititemsvector.cpp
     ${PROJECT_SOURCE_DIR}/src/bitoutputarchive.cpp
     ${PROJECT_SOURCE_DIR}/src/bitpropvariant.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/bufferitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/callback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cbufferoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cfileoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivoloutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/extractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fileextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/formatdetect.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsindexer.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/genericstreamitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/hresultcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/opencallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/renameditem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/streamextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/stdstreamitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/updatecallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/util.cpp )

# enable only debug/release configurations for generated VS project file
if( CMAKE_CONFIGURATION_TYPES )
    set( CMAKE_CONFIGURATION_TYPES Debug Release )
    set( CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
         "Reset the configurations to what we need"
         FORCE )
endif()

# by default, use release build
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE "Release" )
    message( STATUS "Build type not specified: using Release by default." )
endif()

# per-architecture options
if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    set( ARCH_POSTFIX "" )
    set( ARCH_DIR "x86" )
else()
    set( ARCH_POSTFIX 64 )
    set( ARCH_DIR "x64" )
    if( WIN32 )
        add_definitions( -DWIN64 )
    endif()
endif()

# forcing output directory to ${BIT7Z_DIR}/lib/${ARCH_DIR}/ (e.g. ./lib/x64/)
set( OUTPUT_DIR ${PROJECT_SOURCE_DIR}/lib/${ARCH_DIR}/ )

# Note: not applied in generated Visual Studio project files (e.g., .vcxproj)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR} )

if( NOT CMAKE_GENERATOR MATCHES "Visual Studio" )
    # debug library file name must end with "_d" (e.g. bit7z_d.lib)
    set( CMAKE_DEBUG_POSTFIX "_d" )
endif()

message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Architecture: ${ARCH_DIR}" )
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )

# checking if compiler supports the standard filesystem library
set( CMAKE_CXX_STANDARD 17 )
include( CheckIncludeFileCXX )
check_include_file_cxx( "filesystem" USE_STANDARD_FILESYSTEM )

if( NOT USE_STANDARD_FILESYSTEM )
    set( CMAKE_CXX_STANDARD 14 ) # if standard filesystem lib is not supported, revert to C++14 standard

    if( NOT EXISTS ${EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp )
        include( ExternalProject )

        file( DOWNLOAD
              https://raw.githubusercontent.com/gulrak/filesystem/master/include/ghc/filesystem.hpp
              ${EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp
              SHOW_PROGRESS
              )
    endif()
    message( STATUS "Standard filesystem: NO (using ghc::filesystem)" )
else()
    message( STATUS "Standard filesystem: YES" )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )

message( STATUS "Language Standard: C++${CMAKE_CXX_STANDARD}" )

set( TARGET_NAME bit7z${ARCH_POSTFIX} )
add_library( ${TARGET_NAME} STATIC ${SOURCES} ${HEADERS} )

# macros
target_compile_definitions( ${TARGET_NAME} PUBLIC UNICODE _UNICODE _7Z_VOL )
if( WIN32 )
    target_compile_definitions( ${TARGET_NAME} PUBLIC _WINDOWS )
endif()

option( BIT7Z_AUTO_FORMAT "Enable or disable auto format detection" )
message( STATUS "Auto format detection: ${BIT7Z_AUTO_FORMAT}" )
if( BIT7Z_AUTO_FORMAT )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_AUTO_FORMAT )
endif()

option( BIT7Z_REGEX_MATCHING "Enable or disable regex matching of archived files" )
message( STATUS "Regex matching extraction: ${BIT7Z_REGEX_MATCHING}" )
if( BIT7Z_REGEX_MATCHING )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_REGEX_MATCHING )
endif()

# includes
target_include_directories( ${TARGET_NAME} PRIVATE
                            ${PROJECT_SOURCE_DIR}/include/
                            ${EXTERNAL_LIBS_DIR}
                            ${7ZIP_SOURCE_DIR}/CPP/
                            )

if( UNIX )
    target_include_directories( ${TARGET_NAME} PRIVATE
                                ${7ZIP_SOURCE_DIR}/CPP/7zip
                                ${7ZIP_SOURCE_DIR}/CPP/myWindows
                                ${7ZIP_SOURCE_DIR}/CPP/include_windows/ )
    target_compile_definitions( ${TARGET_NAME} PUBLIC EXTERNAL_CODECS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT
                                ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_link_libraries( ${TARGET_NAME} ${CMAKE_DL_LIBS} )
endif()

# compiler options
if( MSVC )
    # setting pdb file name for debug builds (otherwise it is not generated!)
    set_target_properties( ${TARGET_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG ${TARGET_NAME}${CMAKE_DEBUG_POSTFIX} )
    # release builds should be optimized (e.g., for size)
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Os>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Oi>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/analyze>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/GS>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Gy>" )
    # remove default warning level
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    # warning level
    target_compile_options( ${TARGET_NAME} PRIVATE /W4 /WX )
    # standard conformance options of MSVC
    target_compile_options( ${TARGET_NAME} PRIVATE /nologo /fp:precise /Zc:wchar_t /Zc:rvalueCast /Zc:inline
                            /Zc:forScope /Zc:strictStrings /Zc:throwingNew /Zc:referenceBinding )
    # enabling multi-threaded compiling
    set( MSVC_COMPILE_FLAGS "/MP" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}" )
    set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /nologo" )
    # static runtime option
    if( STATIC_RUNTIME )
        set( CompilerFlags
             CMAKE_CXX_FLAGS_DEBUG
             CMAKE_CXX_FLAGS_RELEASE
             CMAKE_C_FLAGS_DEBUG
             CMAKE_C_FLAGS_RELEASE )
        foreach( CompilerFlag ${CompilerFlags} )
            string( REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}" )
            set( ${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE )
            message( STATUS "MSVC flags: ${CompilerFlag}:${${CompilerFlag}}" )
        endforeach()
    endif()
elseif( MINGW )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wall )
else()
    target_compile_options( ${TARGET_NAME} PRIVATE -Wall -Wextra -Werror )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-inconsistent-missing-override )
endif()