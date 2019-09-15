cmake_minimum_required(VERSION 2.6)
project (Bit7z)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(7Z_LIB_SOURCES 
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
)
set(BIT7Z_SOURCES
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

add_library(7z STATIC ${7Z_LIB_SOURCES})
add_library(bit7z STATIC ${BIT7Z_SOURCES})
target_include_directories(7z INTERFACE ${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP)
target_include_directories(bit7z INTERFACE 
${PROJECT_SOURCE_DIR}/include 
${PROJECT_SOURCE_DIR}/lib/7zSDK/CPP
)

target_compile_definitions(7z PRIVATE -D_WINDOWS=1 -DUNICODE=1 -D_UNICODE=1 -D_UNICODE=1 -D_7Z_VOL=1)
target_compile_options(bit7z PRIVATE -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding )
target_compile_options(7z PRIVATE -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding )
target_link_libraries(bit7z  7z)
if(ENABLE_INSTALL)
  install(TARGETS bit7z DESTINATION lib)
  install(TARGETS 7z DESTINATION lib)#I don't know if 7z will be required later
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/  DESTINATION include)
endif()
if(BIT7Z_AUTO_FORMAT)
  target_compile_definitions(bit7z PRIVATE -DBIT7Z_AUTO_FORMAT=1 -D_WINDOWS=1 -DUNICODE=1 -D_UNICODE=1 -D_UNICODE=1 -D_7Z_VOL=1)
else()
  target_compile_definitions(bit7z PRIVATE -D_WINDOWS=1 -DUNICODE=1 -D_UNICODE=1 -D_UNICODE=1 -D_7Z_VOL=1)
endif()

#[[
----How to build wiki-----
mkdir build
cd build
cmake ../ -DBIT7Z_AUTO_FORMAT=True

]]




