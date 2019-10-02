cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(bit7z)

# headers
set(HEADER_FILES
    ${PROJECT_SOURCE_DIR}/include/bit7z.hpp
    ${PROJECT_SOURCE_DIR}/include/bit7zlibrary.hpp
    ${PROJECT_SOURCE_DIR}/include/bitarchivecreator.hpp
    ${PROJECT_SOURCE_DIR}/include/bitarchivehandler.hpp
    ${PROJECT_SOURCE_DIR}/include/bitarchiveinfo.hpp
    ${PROJECT_SOURCE_DIR}/include/bitarchiveitem.hpp
    ${PROJECT_SOURCE_DIR}/include/bitarchiveopener.hpp
    ${PROJECT_SOURCE_DIR}/include/bitcompressionlevel.hpp
    ${PROJECT_SOURCE_DIR}/include/bitcompressionmethod.hpp
    ${PROJECT_SOURCE_DIR}/include/bitcompressor.hpp
    ${PROJECT_SOURCE_DIR}/include/bitexception.hpp
    ${PROJECT_SOURCE_DIR}/include/bitextractor.hpp
    ${PROJECT_SOURCE_DIR}/include/bitformat.hpp
    ${PROJECT_SOURCE_DIR}/include/bitguids.hpp
    ${PROJECT_SOURCE_DIR}/include/bitinputarchive.hpp
    ${PROJECT_SOURCE_DIR}/include/bitmemcompressor.hpp
    ${PROJECT_SOURCE_DIR}/include/bitmemextractor.hpp
    ${PROJECT_SOURCE_DIR}/include/bitpropvariant.hpp
    ${PROJECT_SOURCE_DIR}/include/bitstreamcompressor.hpp
    ${PROJECT_SOURCE_DIR}/include/bitstreamextractor.hpp
    ${PROJECT_SOURCE_DIR}/include/bittypes.hpp
    ${PROJECT_SOURCE_DIR}/include/bufferextractcallback.hpp
    ${PROJECT_SOURCE_DIR}/include/bufferupdatecallback.hpp
    ${PROJECT_SOURCE_DIR}/include/callback.hpp
    ${PROJECT_SOURCE_DIR}/include/cbufoutstream.hpp
    ${PROJECT_SOURCE_DIR}/include/cmultivoloutstream.hpp
    ${PROJECT_SOURCE_DIR}/include/cstdinstream.hpp
    ${PROJECT_SOURCE_DIR}/include/cstdoutstream.hpp
    ${PROJECT_SOURCE_DIR}/include/extractcallback.hpp
    ${PROJECT_SOURCE_DIR}/include/fileextractcallback.hpp
    ${PROJECT_SOURCE_DIR}/include/fileupdatecallback.hpp
    ${PROJECT_SOURCE_DIR}/include/fsindexer.hpp
    ${PROJECT_SOURCE_DIR}/include/fsitem.hpp
    ${PROJECT_SOURCE_DIR}/include/fsutil.hpp
    ${PROJECT_SOURCE_DIR}/include/opencallback.hpp
    ${PROJECT_SOURCE_DIR}/include/streamextractcallback.hpp
    ${PROJECT_SOURCE_DIR}/include/streamupdatecallback.hpp
    ${PROJECT_SOURCE_DIR}/include/updatecallback.hpp
)

# sources
set(SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/C/Alloc.c
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Windows/FileDir.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Windows/FileFind.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Windows/FileIO.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Windows/FileName.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/7zip/Common/FileStreams.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Common/IntToString.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Common/MyString.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/Common/MyVector.cpp
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/7zip/Common/StreamObjects.cpp
    ${PROJECT_SOURCE_DIR}/src/bit7zlibrary.cpp
    ${PROJECT_SOURCE_DIR}/src/bitarchivecreator.cpp
    ${PROJECT_SOURCE_DIR}/src/bitarchivehandler.cpp
    ${PROJECT_SOURCE_DIR}/src/bitarchiveinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/bitarchiveitem.cpp
    ${PROJECT_SOURCE_DIR}/src/bitarchiveopener.cpp
    ${PROJECT_SOURCE_DIR}/src/bitcompressor.cpp
    ${PROJECT_SOURCE_DIR}/src/bitexception.cpp
    ${PROJECT_SOURCE_DIR}/src/bitextractor.cpp
    ${PROJECT_SOURCE_DIR}/src/bitformat.cpp
    ${PROJECT_SOURCE_DIR}/src/bitguids.cpp
    ${PROJECT_SOURCE_DIR}/src/bitinputarchive.cpp
    ${PROJECT_SOURCE_DIR}/src/bitmemcompressor.cpp
    ${PROJECT_SOURCE_DIR}/src/bitmemextractor.cpp
    ${PROJECT_SOURCE_DIR}/src/bitpropvariant.cpp
    ${PROJECT_SOURCE_DIR}/src/bitstreamcompressor.cpp
    ${PROJECT_SOURCE_DIR}/src/bitstreamextractor.cpp
    ${PROJECT_SOURCE_DIR}/src/bufferextractcallback.cpp
    ${PROJECT_SOURCE_DIR}/src/bufferupdatecallback.cpp
    ${PROJECT_SOURCE_DIR}/src/callback.cpp
    ${PROJECT_SOURCE_DIR}/src/cbufoutstream.cpp
    ${PROJECT_SOURCE_DIR}/src/cmultivoloutstream.cpp
    ${PROJECT_SOURCE_DIR}/src/cstdinstream.cpp
    ${PROJECT_SOURCE_DIR}/src/cstdoutstream.cpp
    ${PROJECT_SOURCE_DIR}/src/extractcallback.cpp
    ${PROJECT_SOURCE_DIR}/src/fileextractcallback.cpp
    ${PROJECT_SOURCE_DIR}/src/fileupdatecallback.cpp
    ${PROJECT_SOURCE_DIR}/src/fsindexer.cpp
    ${PROJECT_SOURCE_DIR}/src/fsitem.cpp
    ${PROJECT_SOURCE_DIR}/src/fsutil.cpp
    ${PROJECT_SOURCE_DIR}/src/opencallback.cpp
    ${PROJECT_SOURCE_DIR}/src/streamextractcallback.cpp
    ${PROJECT_SOURCE_DIR}/src/streamupdatecallback.cpp
    ${PROJECT_SOURCE_DIR}/src/updatecallback.cpp
)

# enable only debug/release configurations for generated VS project file
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we need"
        FORCE)
endif()

# by default, use release build
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: using Release by default.")
endif()

# debug library file name must end with "_d" (e.g. bit7z_d.lib)
set(CMAKE_DEBUG_POSTFIX "_d")

# per-architecture options
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(STATUS "Architecture: x86")
    set(ARCH_POSTFIX "")
    set(ARCH_DIR "x86")
else()
    message(STATUS "Architecture: x64")
    set(ARCH_POSTFIX 64)
    set(ARCH_DIR "x64")
    add_definitions(-DWIN64)
endif()

# forcing output directory to ${BIT7Z_DIR}/bin/${ARCH_DIR}/ (e.g. ./bin/x64/)
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin/${ARCH_DIR}/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
foreach( OUTPUT_CONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUT_CONFIG} OUTPUT_CONFIG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUT_CONFIG} ${OUTPUT_DIR})
endforeach()

# c++11 required
set(CMAKE_CXX_STANDARD 11)

set(TARGET_NAME bit7z${ARCH_POSTFIX})

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

# macros
target_compile_definitions(${TARGET_NAME} PUBLIC UNICODE _UNICODE _7Z_VOL _WINDOWS)
if(BIT7Z_AUTO_FORMAT)
    target_compile_definitions(${TARGET_NAME} PUBLIC BIT7Z_AUTO_FORMAT)
endif()
if(BIT7Z_REGEX_MATCHING)
    target_compile_definitions(${TARGET_NAME} PUBLIC BIT7Z_REGEX_MATCHING)
endif()

# includes
target_include_directories(${TARGET_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP/
)

# compiler options
if(MSVC)
    # setting pdb file name for debug builds (otherwise it is not generated!)
    set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_PDB_NAME_DEBUG ${TARGET_NAME}${CMAKE_DEBUG_POSTFIX})
    # release builds should be optimized also for size
    target_compile_options(${TARGET_NAME} PRIVATE "$<$<CONFIG:RELEASE>:/Os>")
    # remove default warning level
    string (REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string (REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # warning level
    target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    # standard conformance options of MSVC
    target_compile_options(${TARGET_NAME} PRIVATE /Zc:rvalueCast /Zc:inline)
    # standard conformance options for MSVC > 2013 (i.e. >= 2015)
    if(MSVC_VERSION GREATER 1800) 
        target_compile_options(${TARGET_NAME} PRIVATE /Zc:strictStrings /Zc:throwingNew /Zc:referenceBinding)
    endif()
    # static runtime option
    if(STATIC_RUNTIME)
        set(CompilerFlags
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE)
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
            message("MSVC flags: ${CompilerFlag}:${${CompilerFlag}}")
        endforeach()
    endif()
endif()
