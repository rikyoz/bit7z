TEMPLATE = app
TARGET = Bit7zTest
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  -= qt
QT      -= core

SOURCES += main.cpp

DEFINES += _UNICODE

CONFIG  += embed_manifest_exe

contains(QMAKE_HOST.arch, x86_64) {
    QMAKE_LFLAGS         += /MACHINE:X64
    PLATFORM = x64
} else {
    QMAKE_LFLAGS         += /MACHINE:X86
    PLATFORM = x86
}

CONFIG(debug, debug|release) { BUILD = debug } else { BUILD = release }

LIBS += -L$$PWD/../bin/$${PLATFORM}/
DESTDIR  = $$PWD/bin/$${PLATFORM}/
OBJECTS_DIR = $$PWD/build/$${PLATFORM}/$${BUILD}/.obj
RCC_DIR  = $$PWD/build/$${PLATFORM}/$${BUILD}/.rcc

LIBS += -lbit7z -loleaut32 -lole32 -luuid
