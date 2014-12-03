TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp \
           lib/7zSDK/win/CPP/Windows/DLL.cpp \
           lib/7zSDK/win/CPP/Windows/FileIO.cpp \
           lib/7zSDK/win/CPP/Windows/FileDir.cpp \
           lib/7zSDK/win/CPP/Windows/FileName.cpp \
           lib/7zSDK/win/CPP/Windows/FileFind.cpp \
           lib/7zSDK/win/CPP/Windows/COM.cpp \
           lib/7zSDK/win/CPP/Windows/PropVariant.cpp \
           lib/7zSDK/win/CPP/Windows/PropVariantConversions.cpp \
           lib/7zSDK/win/CPP/7zip/Common/FileStreams.cpp \
           lib/7zSDK/win/CPP/Common/StringConvert.cpp \
           lib/7zSDK/win/CPP/Common/IntToString.cpp \
           lib/7zSDK/win/CPP/Common/MyVector.cpp \
           src/bitextractor.cpp \
           src/bitcompressor.cpp \
           src/bit7zlibrary.cpp \
           src/bitexception.cpp \
           src/bitguids.cpp \
    src/extractcallback.cpp \
    src/opencallback.cpp \
    src/updatecallback.cpp \
    src/bitfilesystem.cpp

INCLUDEPATH += lib/7zSDK/win/CPP/ \
               include/
#include(lib/7z.pri)

CONFIG  += c++11

LIBS += -loleaut32 -lole32 -luuid -luser32

DEFINES += _UNICODE

DESTDIR  = $$PWD/bin/

HEADERS += \
           include/bitcompressor.hpp \
           include/bit7zlibrary.hpp \
           include/bitexception.hpp \
           include/bitguids.hpp \
           include/bitextractor.hpp \
    include/extractcallback.hpp \
    include/opencallback.hpp \
    include/updatecallback.hpp \
    include/bitfilesystem.hpp
