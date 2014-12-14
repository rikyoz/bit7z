TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  -= qt
QT      -= core
TARGET   = Bit7zTest

SOURCES += main.cpp

contains(QMAKE_HOST.arch, x86_64) {
    DESTDIR  = $$PWD/bin/x64/
} else {
    DESTDIR  = $$PWD/bin/x86/
}

contains(QMAKE_HOST.arch, x86_64) {
    LIBS += -L$$PWD/../bin/x64/
} else {
    LIBS += -L$$PWD/../bin/x86/
}

LIBS += -lbit7z -loleaut32 -lole32 -luuid

DEFINES += _UNICODE

INCLUDEPATH += ../lib/7zSDK/win/CPP/ \
               ../include/

CONFIG  += embed_manifest_exe
