TARGET = Honey
QT += opengl
QT -= gui
TEMPLATE = lib
CONFIG += staticlib
CONFIG -= flat

P = $$_PRO_FILE_PWD_/../Honey

include($$P/Settings.pri)

INCLUDEPATH += $$P

SOURCES += \
    $$P/BASE/*.cpp \
    $$P/GRAPH/*.cpp \
    $$P/GRAPH/GFX/*.cpp \
    $$P/GRAPH/THIN3D/*.cpp \
    $$P/GRAPH/RENDERER/*.cpp \
    $$P/GRAPH/RENDERER/SHADER/*.* \
    $$P/GRAPH/UI/*.cpp \
    $$P/GRAPH/UI/CONTROLS/*.cpp \
    $$P/IMAGE/*.cpp \
    $$P/MATH/*.cpp \
    $$P/IO/*.cpp \
    $$P/UTILS/HASH/*.cpp \
    $$P/UTILS/STRING/*.cpp \
    $$P/UTILS/TIME/*.cpp

HEADERS += \
    $$P/BASE/*.h \
    $$P/GRAPH/*.h \
    $$P/GRAPH/GFX/*.h \
    $$P/GRAPH/THIN3D/*.h \
    $$P/GRAPH/RENDERER/*.h \
    $$P/GRAPH/UI/*.h \
    $$P/GRAPH/UI/CONTROLS/*.h \
    $$P/IMAGE/*.h \
    $$P/MATH/*.h \
    $$P/IO/*.h \
    $$P/UTILS/HASH/*.h \
    $$P/UTILS/STRING/*.h \
    $$P/UTILS/TIME/*.h \
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

# d3des
SOURCES += $$P/EXTERNALS/d3des/*.c
HEADERS += $$P/EXTERNALS/d3des/*.h

#tinyxml2
SOURCES += $$P/EXTERNALS/tinyxml2/*.cpp
HEADERS += $$P/EXTERNALS/tinyxml2/*.h
