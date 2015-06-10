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
    $$P/SMARTGRAPH/GFX/*.cpp \
    $$P/SMARTGRAPH/THIN3D/*.cpp \
    $$P/SMARTGRAPH/UI/*.cpp \
    $$P/SMARTGRAPH/UI/CONTROLS/*.cpp \
    $$P/IMAGE/*.cpp \
    $$P/MATH/*.cpp \
    $$P/IO/*.cpp \
    $$P/THREAD/*.cpp \
    $$P/UTILS/TEXT/*.cpp \
    $$P/UTILS/HASH/*.cpp \
    $$P/UTILS/STRING/*.cpp \
    $$P/UTILS/TIME/*.cpp \
    $$P/UTILS/COLOR/*.cpp

HEADERS += \
    $$P/BASE/*.h \
    $$P/SMARTGRAPH/GFX/*.h \
    $$P/SMARTGRAPH/THIN3D/*.h \
    $$P/SMARTGRAPH/UI/*.h \
    $$P/SMARTGRAPH/UI/CONTROLS/*.h \
    $$P/IMAGE/*.h \
    $$P/MATH/*.h \
    $$P/IO/*.h \
    $$P/THREAD/*.h \
    $$P/UTILS/TEXT/*.h \
    $$P/UTILS/STRING/*.h \
    $$P/UTILS/TIME/*.h \
    $$P/UTILS/COLOR/*.h \
    $$P/UTILS/RANDOM/*.h
    
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
