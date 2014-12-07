TEMPLATE = app
CONFIG  += console
CONFIG  -= app_bundle
CONFIG  -= qt
QT      -= core
TARGET   = Bit7zTest

SOURCES += main.cpp

LIBS += -L$$PWD/../bin -lbit7z -loleaut32 -lole32 -luuid

DEFINES += _UNICODE

INCLUDEPATH += ../lib/7zSDK/win/CPP/ \
               ../include/

DESTDIR  = $$PWD/../bin/

CONFIG  += embed_manifest_exe
