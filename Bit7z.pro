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
		   src/CArchiveExtractCallback.cpp \
		   src/CArchiveOpenCallback.cpp \
		   src/util.cpp \
		   src/CArchiveUpdateCallback.cpp



INCLUDEPATH += lib/7zSDK/win/CPP/ \
               include/
#include(lib/7z.pri)

CONFIG += c++11

DEFINES += _UNICODE

DESTDIR = $$PWD/bin/

HEADERS += \
    include/CArchiveOpenCallback.h \
    include/CArchiveExtractCallback.h \
    include/util.h \
    include/CArchiveUpdateCallback.h
