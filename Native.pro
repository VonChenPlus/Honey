TARGET = Native
QT += opengl
QT -= gui
TEMPLATE = lib
CONFIG += staticlib

P = $$_PRO_FILE_PWD_/../Native

INCLUDEPATH += $$P

SOURCES += $$P/BASE/*.cpp \
    $$P/UI/*.cpp

HEADERS += \
    $$P/BASE/BasicTypes.h \
    $$P/UI/Screen.h \
    $$P/UI/ScreenManager.h
