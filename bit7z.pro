#VERSION  = 3.1.3
TEMPLATE = lib
CONFIG  += staticlib
CONFIG  -= app_bundle
CONFIG  -= qt

SOURCES += lib/7zSDK/C/Alloc.c \
           lib/7zSDK/CPP/Windows/FileIO.cpp \
           lib/7zSDK/CPP/Windows/FileDir.cpp \
           lib/7zSDK/CPP/Windows/FileName.cpp \
           lib/7zSDK/CPP/Windows/FileFind.cpp \
           lib/7zSDK/CPP/7zip/Common/FileStreams.cpp \
           lib/7zSDK/CPP/7zip/Common/StreamObjects.cpp \
           lib/7zSDK/CPP/Common/IntToString.cpp \
           lib/7zSDK/CPP/Common/MyString.cpp \
           lib/7zSDK/CPP/Common/MyVector.cpp \
           src/bit7zlibrary.cpp \
           src/bitarchivecreator.cpp \
           src/bitarchivehandler.cpp \
           src/bitarchiveinfo.cpp \
           src/bitarchiveitem.cpp \
           src/bitarchiveopener.cpp \
           src/bitcompressor.cpp \
           src/bitexception.cpp \
           src/bitextractor.cpp \
           src/bitformat.cpp \
           src/bitguids.cpp \
           src/bitinputarchive.cpp \
           src/bitmemcompressor.cpp \
           src/bitmemextractor.cpp \
           src/bitpropvariant.cpp \
           src/bitstreamcompressor.cpp \
           src/bitstreamextractor.cpp \
           src/bufferextractcallback.cpp \
           src/bufferupdatecallback.cpp \
           src/callback.cpp \
           src/cbufoutstream.cpp \
           src/cmultivoloutstream.cpp \
           src/cstdinstream.cpp \
           src/cstdoutstream.cpp \
           src/extractcallback.cpp \
           src/fileextractcallback.cpp \
           src/fileupdatecallback.cpp \
           src/fsindexer.cpp \
           src/fsitem.cpp \
           src/fsutil.cpp \
           src/opencallback.cpp \
           src/streamextractcallback.cpp \
           src/streamupdatecallback.cpp \
           src/updatecallback.cpp

INCLUDEPATH += lib/7zSDK/CPP/

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CXXFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4 #-analyze
QMAKE_CXXFLAGS_WARN_ON += -W4 #-analyze

DEFINES += _UNICODE _7Z_VOL

HEADERS += include/bit7z.hpp \
           include/bit7zlibrary.hpp \
           include/bitarchivecreator.hpp \
           include/bitarchivehandler.hpp \
           include/bitarchiveinfo.hpp \
           include/bitarchiveitem.hpp \
           include/bitarchiveopener.hpp \
           include/bitcompressionlevel.hpp \
           include/bitcompressionmethod.hpp \
           include/bitcompressor.hpp \
           include/bitexception.hpp \
           include/bitextractor.hpp \
           include/bitformat.hpp \
           include/bitguids.hpp \
           include/bitinputarchive.hpp \
           include/bitmemcompressor.hpp \
           include/bitmemextractor.hpp \
           include/bitpropvariant.hpp \
           include/bitstreamcompressor.hpp \
           include/bitstreamextractor.hpp \
           include/bittypes.hpp \
           include/bufferextractcallback.hpp \
           include/bufferupdatecallback.hpp \
           include/callback.hpp \
           include/cbufoutstream.hpp \
           include/cmultivoloutstream.hpp \
           include/cstdinstream.hpp \
           include/cstdoutstream.hpp \
           include/extractcallback.hpp \
           include/fileextractcallback.hpp \
           include/fileupdatecallback.hpp \
           include/fsindexer.hpp \
           include/fsitem.hpp \
           include/fsutil.hpp \
           include/opencallback.hpp \
           include/streamextractcallback.hpp \
           include/streamupdatecallback.hpp \
           include/updatecallback.hpp

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
    QMAKE_CXXFLAGS_RELEASE += /O2 /Os
    BUILD = release
}

TARGET =  bit7z$${ARCH_SUFFIX}$${BUILD_SUFFIX}

DESTDIR  = $$PWD/bin/$${PLATFORM}/
OBJECTS_DIR = $$PWD/build/$${PLATFORM}/$${BUILD}/.obj
RCC_DIR  = $$PWD/build/$${PLATFORM}/$${BUILD}/.rcc
