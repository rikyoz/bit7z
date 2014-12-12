#TEMPLATE = app
#CONFIG += console
TEMPLATE = lib
TARGET = bit7z
VERSION  = 1.0
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += lib/7zSDK/win/CPP/Windows/Error.cpp \
           lib/7zSDK/win/CPP/Windows/DLL.cpp \
           lib/7zSDK/win/CPP/Windows/FileIO.cpp \
           lib/7zSDK/win/CPP/Windows/FileDir.cpp \
           lib/7zSDK/win/CPP/Windows/FileName.cpp \
           lib/7zSDK/win/CPP/Windows/FileFind.cpp \
           lib/7zSDK/win/CPP/Windows/COM.cpp \
           lib/7zSDK/win/CPP/Windows/PropVariant.cpp \
           lib/7zSDK/win/CPP/7zip/Common/FileStreams.cpp \
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
           src/fsitem.cpp \
           src/fsindexer.cpp \
           src/fsutil.cpp \
           src/callback.cpp \
           src/bitformat.cpp

INCLUDEPATH += lib/7zSDK/win/CPP/ \
               include/

CONFIG  += c++11 embed_manifest_dll

LIBS += -loleaut32 -lole32 -luuid -luser32

DEFINES += _UNICODE

DESTDIR  = $$PWD/bin/

HEADERS += include/bitcompressor.hpp \
           include/bit7zlibrary.hpp \
           include/bitexception.hpp \
           include/bitguids.hpp \
           include/bitextractor.hpp \
           include/extractcallback.hpp \
           include/opencallback.hpp \
           include/updatecallback.hpp \
           include/fsitem.hpp \
           include/fsindexer.hpp \
           include/fsutil.hpp \
           include/bit7z.hpp \
           include/callback.hpp \
           include/bitformat.hpp

win32 {
  contains(QMAKE_HOST.arch, x86_64) {
    QMAKE_LFLAGS         += /MACHINE:X64
  } else {
    QMAKE_LFLAGS         += /MACHINE:X86
  }
}
