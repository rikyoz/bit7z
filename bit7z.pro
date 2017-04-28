#VERSION  = 2.1
TEMPLATE = lib
CONFIG  += staticlib
CONFIG  -= app_bundle
CONFIG  -= qt

SOURCES += lib/7zSDK/C/Alloc.c \
           lib/7zSDK/CPP/Windows/DLL.cpp \
           lib/7zSDK/CPP/Windows/FileIO.cpp \
           lib/7zSDK/CPP/Windows/FileDir.cpp \
           lib/7zSDK/CPP/Windows/FileName.cpp \
           lib/7zSDK/CPP/Windows/FileFind.cpp \
           lib/7zSDK/CPP/Windows/COM.cpp \
           lib/7zSDK/CPP/Windows/PropVariant.cpp \
           lib/7zSDK/CPP/7zip/Common/FileStreams.cpp \
           lib/7zSDK/CPP/7zip/Common/StreamObjects.cpp \
           lib/7zSDK/CPP/Common/IntToString.cpp \
           lib/7zSDK/CPP/Common/MyString.cpp \
           lib/7zSDK/CPP/Common/MyVector.cpp \
           src/bitextractor.cpp \
           src/bitcompressor.cpp \
           src/bitmemcompressor.cpp \
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
           src/bitformat.cpp \
           src/memextractcallback.cpp \
           src/memupdatecallback.cpp \
           src/bitmemextractor.cpp \
           src/coutmemstream.cpp \
           src/coutmultivolstream.cpp \
           src/bitarchivecreator.cpp \
           src/bitarchiveopener.cpp \
           src/bitarchivehandler.cpp \
           src/util.cpp

INCLUDEPATH += lib/7zSDK/CPP/

QMAKE_CFLAGS_WARN_ON = -W4
QMAKE_CXXFLAGS_WARN_ON = -W4

DEFINES += _UNICODE _7Z_VOL

HEADERS += include/bitcompressor.hpp \
           include/bitmemcompressor.hpp \
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
           include/bitformat.hpp \
           include/bitcompressionlevel.hpp \
           include/memextractcallback.hpp \
           include/bittypes.hpp \
           include/memupdatecallback.hpp \
           include/bitmemextractor.hpp \
           include/coutmemstream.hpp \
           include/coutmultivolstream.hpp \
           include/bitarchivecreator.hpp \
           include/bitarchiveopener.hpp \
           include/bitarchivehandler.hpp \
           include/util.hpp

contains(QT_ARCH, i386) {
    QMAKE_LFLAGS         += /MACHINE:X86
    PLATFORM = x86
} else {
    QMAKE_LFLAGS         += /MACHINE:X64
    PLATFORM = x64
    ARCH_SUFFIX = 64
}

CONFIG(debug, debug|release) {
    BUILD = debug
    BUILD_SUFFIX = _d
} else {
    BUILD = release
}

TARGET =  bit7z$${ARCH_SUFFIX}$${BUILD_SUFFIX}

DESTDIR  = $$PWD/bin/$${PLATFORM}/
OBJECTS_DIR = $$PWD/build/$${PLATFORM}/$${BUILD}/.obj
RCC_DIR  = $$PWD/build/$${PLATFORM}/$${BUILD}/.rcc
