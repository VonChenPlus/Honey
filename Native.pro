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

HEADERS += \
    $$P/BASE/BasicTypes.h \
    $$P/BASE/Mutex.h \
    $$P/INPUT/InputState.h \
    $$P/MATH/Vector.h \
    $$P/MATH/Matrix.h \
    $$P/MATH/Quaternion.h \
    $$P/UI/Screen.h \
    $$P/UI/ScreenManager.h \
    $$P/UI/UIScreen.h
