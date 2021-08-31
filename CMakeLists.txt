cmake_minimum_required( VERSION 3.9 )

project( bit7z
         VERSION 4.0.0
         DESCRIPTION "A C++ static library offering a clean and simple interface to the 7-zip/p7zip shared libraries"
         HOMEPAGE_URL "https://github.com/rikyoz/bit7z/" )

set( CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON" )
cmake_policy( SET CMP0022 NEW )

set( EXTERNAL_LIBS_DIR ${PROJECT_SOURCE_DIR}/third_party )
set( 7ZIP_SOURCE_DIR ${EXTERNAL_LIBS_DIR}/7-zip )

# public headers
set( PUBLIC_HEADERS
     ${PROJECT_SOURCE_DIR}/include/bit7z.hpp
     ${PROJECT_SOURCE_DIR}/include/bit7zlibrary.hpp
     ${PROJECT_SOURCE_DIR}/include/bitabstractarchivecreator.hpp
     ${PROJECT_SOURCE_DIR}/include/bitabstractarchivehandler.hpp
     ${PROJECT_SOURCE_DIR}/include/bitabstractarchiveopener.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveeditor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveiteminfo.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchiveitemoffset.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivereader.hpp
     ${PROJECT_SOURCE_DIR}/include/bitarchivewriter.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressionlevel.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressionmethod.hpp
     ${PROJECT_SOURCE_DIR}/include/bitcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitdefines.hpp
     ${PROJECT_SOURCE_DIR}/include/biterror.hpp
     ${PROJECT_SOURCE_DIR}/include/bitexception.hpp
     ${PROJECT_SOURCE_DIR}/include/bitextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfilecompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfileextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitformat.hpp
     ${PROJECT_SOURCE_DIR}/include/bitfs.hpp
     ${PROJECT_SOURCE_DIR}/include/bitgenericitem.hpp
     ${PROJECT_SOURCE_DIR}/include/bitinputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bititemsvector.hpp
     ${PROJECT_SOURCE_DIR}/include/bitmemcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitmemextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitoutputarchive.hpp
     ${PROJECT_SOURCE_DIR}/include/bitpropvariant.hpp
     ${PROJECT_SOURCE_DIR}/include/bitstreamcompressor.hpp
     ${PROJECT_SOURCE_DIR}/include/bitstreamextractor.hpp
     ${PROJECT_SOURCE_DIR}/include/bittypes.hpp
     ${PROJECT_SOURCE_DIR}/include/bitwindows.hpp )

# headers
set( HEADERS
     ${PROJECT_SOURCE_DIR}/include/internal/bufferextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/bufferitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/callback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cbufferinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cbufferoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cfileinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cfileoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cfixedbufferoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cmultivoloutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cstdinstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/cstdoutstream.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/dateutil.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/extractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fileextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fixedbufferextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/formatdetect.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsindexer.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fsutil.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/fs.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/genericinputitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/guiddef.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/guids.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/hresultcategory.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/internalcategory.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/opencallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/renameditem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/stdinputitem.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/streamextractcallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/updatecallback.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/util.hpp
     ${PROJECT_SOURCE_DIR}/include/internal/windows.hpp )

# sources
set( SOURCES
     ${PROJECT_SOURCE_DIR}/src/bit7zlibrary.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchivecreator.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchivehandler.cpp
     ${PROJECT_SOURCE_DIR}/src/bitabstractarchiveopener.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveeditor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitem.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveiteminfo.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchiveitemoffset.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivereader.cpp
     ${PROJECT_SOURCE_DIR}/src/bitarchivewriter.cpp
     ${PROJECT_SOURCE_DIR}/src/biterror.cpp
     ${PROJECT_SOURCE_DIR}/src/bitexception.cpp
     ${PROJECT_SOURCE_DIR}/src/bitfilecompressor.cpp
     ${PROJECT_SOURCE_DIR}/src/bitformat.cpp
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
     ${PROJECT_SOURCE_DIR}/src/internal/cfixedbufferoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cmultivoloutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdinstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/cstdoutstream.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/dateutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/extractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fileextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fixedbufferextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/formatdetect.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsindexer.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/fsutil.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/genericinputitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/guids.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/hresultcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/internalcategory.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/opencallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/renameditem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/stdinputitem.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/streamextractcallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/updatecallback.cpp
     ${PROJECT_SOURCE_DIR}/src/internal/util.cpp )

# BitPropVariant needs the implementation of some Windows functions on Unix systems
if( UNIX )
    set( SOURCES ${SOURCES} ${7ZIP_SOURCE_DIR}/CPP/Common/MyWindows.cpp )
endif()

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

option( BIT7Z_VS_LIBNAME_OUTDIR_STYLE
        "Force using Visual Studio output library name and directory structure convention" )
# per-architecture options
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
    # debug library file name must end with "_d" (e.g. bit7z_d.lib)
    set( CMAKE_DEBUG_POSTFIX "_d" )
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

# macros
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

# includes
target_include_directories( ${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/ )
target_include_directories( ${TARGET_NAME} PRIVATE
                            ${EXTERNAL_LIBS_DIR}
                            ${7ZIP_SOURCE_DIR}/CPP/ )

if( UNIX )
    target_compile_definitions( ${TARGET_NAME} PRIVATE EXTERNAL_CODECS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT
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
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/GS>" )
    target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/Gy>" )

    option( BIT7Z_ANALYZE_CODE "Enable or disable code analysis" )
    message( STATUS "Code analysis: ${BIT7Z_ANALYZE_CODE}" )
    if( BIT7Z_ANALYZE_CODE )
        target_compile_options( ${TARGET_NAME} PRIVATE "$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:MINSIZEREL>>:/analyze>" )
    endif()

    # remove default warning level
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
    string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
    string( REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

    # warning level
    target_compile_options( ${TARGET_NAME} PRIVATE /W4 )

    # standard conformance options of MSVC
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

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-inconsistent-missing-override )
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0 )
    target_compile_options( ${TARGET_NAME} PRIVATE -Wno-missing-field-initializers )
endif()