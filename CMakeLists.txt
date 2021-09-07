cmake_minimum_required( VERSION 3.9 )

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
     include/internal/callback.hpp
     include/internal/cbufferinstream.hpp
     include/internal/cbufferoutstream.hpp
     include/internal/cfileinstream.hpp
     include/internal/cfileoutstream.hpp
     include/internal/cfixedbufferoutstream.hpp
     include/internal/cmultivoloutstream.hpp
     include/internal/cstdinstream.hpp
     include/internal/cstdoutstream.hpp
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
     include/internal/opencallback.hpp
     include/internal/renameditem.hpp
     include/internal/stdinputitem.hpp
     include/internal/streamextractcallback.hpp
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
     src/internal/callback.cpp
     src/internal/cbufferinstream.cpp
     src/internal/cbufferoutstream.cpp
     src/internal/cfileinstream.cpp
     src/internal/cfileoutstream.cpp
     src/internal/cfixedbufferoutstream.cpp
     src/internal/cmultivoloutstream.cpp
     src/internal/cstdinstream.cpp
     src/internal/cstdoutstream.cpp
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
     src/internal/renameditem.cpp
     src/internal/stdinputitem.cpp
     src/internal/streamextractcallback.cpp
     src/internal/updatecallback.cpp
     src/internal/util.cpp )

if( UNIX ) # BitPropVariant needs the implementation of some Windows functions on Unix systems
    set( SOURCES ${SOURCES} ${7ZIP_SOURCE_DIR}/CPP/Common/MyWindows.cpp )
endif()

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

message( STATUS "Target Version: ${CMAKE_PROJECT_VERSION}" )
message( STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}" )
message( STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Architecture: ${ARCH_DIR}" )
message( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )

# checking if compiler supports the standard filesystem library
set( CMAKE_CXX_STANDARD 17 )
include( CheckIncludeFileCXX )
check_include_file_cxx( "filesystem" USE_STANDARD_FILESYSTEM )

if( USE_STANDARD_FILESYSTEM )
    include( CheckCXXSourceCompiles )
    string( CONFIGURE [[
        #include <cstdlib>
        #include <filesystem>

        int main() {
            auto cwd = std::filesystem::current_path();
            printf("%s", cwd.c_str());
            return EXIT_SUCCESS;
        }
    ]] code @ONLY )
    check_cxx_source_compiles( "${code}" STANDARD_FILESYSTEM_COMPILES )
endif()

if( NOT USE_STANDARD_FILESYSTEM OR NOT STANDARD_FILESYSTEM_COMPILES )
    set( CMAKE_CXX_STANDARD 14 ) # if standard filesystem lib is not supported, revert to C++14 standard

    if( NOT EXISTS ${EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp )
        include( ExternalProject )

        # downloading ghc::filesystem as an alternative to std::filesystem
        file( DOWNLOAD
              https://raw.githubusercontent.com/gulrak/filesystem/master/include/ghc/filesystem.hpp
              ${EXTERNAL_LIBS_DIR}/ghc/filesystem.hpp
              SHOW_PROGRESS
              TLS_VERIFY ON )
    endif()
    message( STATUS "Standard filesystem: NO (using ghc::filesystem)" )
else()
    message( STATUS "Standard filesystem: YES" )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )

message( STATUS "Language Standard: C++${CMAKE_CXX_STANDARD}" )

set( TARGET_NAME bit7z${ARCH_POSTFIX} )
add_library( ${TARGET_NAME} STATIC )
target_sources( ${TARGET_NAME}
                PUBLIC ${PUBLIC_HEADERS}
                PRIVATE ${HEADERS} ${SOURCES} )

# 7-zip compilation macros
target_compile_definitions( ${TARGET_NAME} PRIVATE UNICODE _UNICODE )
if( WIN32 )
    target_compile_definitions( ${TARGET_NAME} PRIVATE _WINDOWS )
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

option( BIT7Z_USE_STD_BYTE "Enable or disable using type safe byte type (like std::byte) for buffers" )
message( STATUS "Use std::byte: ${BIT7Z_USE_STD_BYTE}" )
if( BIT7Z_USE_STD_BYTE )
    target_compile_definitions( ${TARGET_NAME} PUBLIC BIT7Z_USE_STD_BYTE )
endif()

target_include_directories( ${TARGET_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/include/" ) # public includes
target_include_directories( ${TARGET_NAME} PRIVATE "${EXTERNAL_LIBS_DIR}" "${7ZIP_SOURCE_DIR}/CPP/" ) # private includes

if( UNIX )
    target_compile_definitions( ${TARGET_NAME} PRIVATE EXTERNAL_CODECS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT
                                ENV_UNIX BREAK_HANDLER USE_WIN_FILE )
    target_link_libraries( ${TARGET_NAME} ${CMAKE_DL_LIBS} )
endif()

if( MSVC ) # compiler-specific options
    # setting a pdb file name for debug builds (otherwise it is not generated!)
    set_target_properties( ${TARGET_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG ${TARGET_NAME}${CMAKE_DEBUG_POSTFIX} )

    # release builds should be optimized (e.g., for size)
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Os>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Oi>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/GS>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Gy>" )

    option( BIT7Z_ANALYZE_CODE "Enable or disable code analysis" )
    message( STATUS "Code analysis: ${BIT7Z_ANALYZE_CODE}" )
    if( BIT7Z_ANALYZE_CODE )
        target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/analyze>" )
    endif()

    # remove CMake default warning level
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

    # warning level option
    target_compile_options( ${TARGET_NAME} PRIVATE /W4 )

    # C++ standard conformance options of MSVC
    target_compile_options( ${TARGET_NAME} PRIVATE /fp:precise /Zc:wchar_t /Zc:rvalueCast /Zc:inline
                            /Zc:forScope /Zc:strictStrings /Zc:throwingNew /Zc:referenceBinding )

    if( CMAKE_GENERATOR MATCHES "Visual Studio" )
        target_compile_options( ${TARGET_NAME} PRIVATE /MP$ENV{NUMBER_OF_PROCESSORS} )
    endif()

    # linker flags
    set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /nologo" )

    # options only for MSVC versions equal or greater than 15.0 (VS 2017)
    if( MSVC_VERSION GREATER_EQUAL 1910 )
        target_compile_options( ${TARGET_NAME} PRIVATE /Zc:__cplusplus )
        # treating warnings as errors
        target_compile_options( ${TARGET_NAME} PRIVATE /WX )
    else() # MSVC < 15.0 (i.e., <= VS 2015)
        # ignoring C4127 warning
        target_compile_options( ${TARGET_NAME} PRIVATE /wd4127 )
    endif()

    # static runtime option
    option( BIT7Z_STATIC_RUNTIME "Enable or disable using /MT MSVC flag" )
    message( STATUS "Static runtime: ${BIT7Z_STATIC_RUNTIME}" )
    if( BIT7Z_STATIC_RUNTIME )
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

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.6 )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-inconsistent-missing-override )
endif()

if( APPLE )
    set( CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>" )
    set( CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>" )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-missing-field-initializers )
endif()