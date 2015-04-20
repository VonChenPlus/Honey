TARGET = Native
QT += opengl
QT -= gui
TEMPLATE = lib
CONFIG += staticlib

P = $$_PRO_FILE_PWD_/../Native

include($$P/Settings.pri)

INCLUDEPATH += $$P

SOURCES += \
    $$P/BASE/*.cpp \
    $$P/GFX/*.cpp \
    $$P/IMAGE/*.cpp \
    $$P/MATH/*.cpp \
    $$P/THIN3D/*.cpp \
    $$P/UI/*.cpp \
    $$P/UTILS/TEXT/UTF8.cpp \
    $$P/UTILS/HASH/Hash.cpp \
    $$P/UTILS/STRING/String.cpp \
    $$P/UTILS/TIME/Time.cpp \
    $$P/UTILS/COLOR/Color.cpp \
    $$P/UTILS/IO/*.cpp

HEADERS += \
    $$P/BASE/*.h \
    $$P/GFX/*.h \
    $$P/IMAGE/*.h \
    $$P/MATH/*.h \
    $$P/THIN3D/*.h \
    $$P/UI/*.h \
    $$P/UTILS/TEXT/UTF8.h \
    $$P/UTILS/TEXT/UTF16.h \
    $$P/UTILS/STRING/String.h \
    $$P/UTILS/TIME/Time.h \
    $$P/UTILS/COLOR/Color.h \
    $$P/UTILS/IO/*.h
    
# Zlib
win32|contains(QT_CONFIG, no-zlib)
{
    SOURCES += $$P/EXTERNALS/zlib/*.c
    HEADERS += $$P/EXTERNALS/zlib/*.h
    INCLUDEPATH += $$P/EXTERNALS/zlib
}

# libpng
SOURCES += $$P/EXTERNALS/libpng17/*.c
HEADERS += $$P/EXTERNALS/libpng17/*.h

# jpg
SOURCES += $$P/EXTERNALS/jpge/*.cpp
HEADERS += $$P/EXTERNALS/jpge/*.h

# glew
SOURCES += $$P/EXTERNALS/glew/glew.c
HEADERS += $$P/EXTERNALS/glew/GL/*.h
INCLUDEPATH += $$P/EXTERNALS/glew

# rg_etc1
SOURCES += $$P/EXTERNALS/rg_etc1/*.cpp
HEADERS += $$P/EXTERNALS/rg_etc1/*.h
