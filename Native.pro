TARGET = Native
QT += opengl
QT -= gui
TEMPLATE = lib
CONFIG += staticlib

P = $$_PRO_FILE_PWD_/../Native

include($$P/Settings.pri)

INCLUDEPATH += $$P

SOURCES += $$P/BASE/NativeApp.cpp \
    $$P/MATH/Vector.cpp \
    $$P/MATH/Matrix.cpp \
    $$P/MATH/Quaternion.cpp \
    $$P/UI/ScreenManager.cpp \
    $$P/UI/View.cpp \
    $$P/UI/UIContext.cpp \
    $$P/GFX/DrawBuffer.cpp \
    $$P/THIN3D/Thin3D.cpp \
    $$P/IMAGE/ZimLoad.cpp \
    $$P/FILE/FileRead.cpp \
    $$P/IMAGE/PNGLoad.cpp \
    $$P/THIN3D/Thin3DGL.cpp \
    $$P/GFX/GLState.cpp \
    $$P/GFX/GLExtensions.cpp \
    $$P/GFX/GfxResourceHolder.cpp \
    $$P/GFX/Texture.cpp \
    $$P/GFX/GLDebug.cpp \
    $$P/BASE/Color.cpp \
    $$P/UTILS/TEXT/UTF8.cpp

HEADERS += \
    $$P/BASE/NativeApp.h \
    $$P/BASE/BasicTypes.h \
    $$P/BASE/Mutex.h \
    $$P/INPUT/InputState.h \
    $$P/MATH/Vector.h \
    $$P/MATH/Matrix.h \
    $$P/MATH/Quaternion.h \
    $$P/UI/Screen.h \
    $$P/UI/ScreenManager.h \
    $$P/UI/UIScreen.h \
    $$P/UI/View.h \
    $$P/MATH/Bounds.h \
    $$P/MATH/Point.h \
    $$P/BASE/SmartPtr.h \
    $$P/UI/UIContext.h \
    $$P/GFX/DrawBuffer.h \
    $$P/THIN3D/Thin3D.h \
    $$P/IMAGE/ZimLoad.h \
    $$P/FILE/FileRead.h \
    $$P/IMAGE/PNGLoad.h \
    $$P/THIN3D/Thin3DGL.h \
    $$P/GFX/GLCommon.h \
    $$P/GFX/GLState.h \
    $$P/GFX/GLExtensions.h \
    $$P/GFX/GfxResourceHolder.h \
    $$P/GFX/Texture.h \
    $$P/GFX/GLDebug.h \
    $$P/BASE/Color.h \
    $$P/MATH/Utils.h \
    $$P/UTILS/TEXT/UTF8.h \
    UTILS/TEXT/UTF16.h

# Zlib
win32|contains(QT_CONFIG, no-zlib)
{
    SOURCES += $$P/EXTERNALS/zlib/*.c \
    HEADERS += $$P/EXTERNALS/zlib/*.h \
}

# libpng
SOURCES += $$P/EXTERNALS/libpng17/*.c
HEADERS += $$P/EXTERNALS/libpng17/*.h

# jpg
SOURCES += $$P/EXTERNALS/jpge/*.c
HEADERS += $$P/EXTERNALS/jpge/*.h

# glew
SOURCES += $$P/EXTERNALS/glew/glew.c
HEADERS += $$P/EXTERNALS/glew/GL/*.h
INCLUDEPATH += $$P/EXTERNALS/glew

# rg_etc1
SOURCES += $$P/EXTERNALS/rg_etc1/*.c
HEADERS += $$P/EXTERNALS/rg_etc1/*.h
