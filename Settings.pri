VERSION = 1.0.0.0

# Global specific
win32:CONFIG(release, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/release)
else:win32:CONFIG(debug, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/debug)
else:CONFIG_DIR=$$OUT_PWD
OBJECTS_DIR = $$CONFIG_DIR/.obj/$$TARGET
MOC_DIR = $$CONFIG_DIR/.moc/$$TARGET
UI_DIR = $$CONFIG_DIR/.ui/$$TARGET
RCC_DIR = $$CONFIG_DIR/.rcc/$$TARGET
QMAKE_CLEAN += -r $$OBJECTS_DIR $$MOC_DIR $$UI_DIR $$RCC_DIR

# Optimisations
win32-msvc* {
	DEFINES += _MBCS GLEW_STATIC _CRT_SECURE_NO_WARNINGS "_VARIADIC_MAX=10"
	contains(DEFINES, UNICODE): DEFINES += _UNICODE
	QMAKE_ALLFLAGS_RELEASE += /O2 /fp:fast
        LIBS += $$P/EXTERNALS/freetype2/library/windows/freetype26MT.lib
} else {
	DEFINES += __STDC_CONSTANT_MACROS
	QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -fno-strict-aliasing -Wno-unused-parameter -Wno-multichar -Wno-uninitialized -Wno-ignored-qualifiers -Wno-missing-field-initializers
	greaterThan(QT_MAJOR_VERSION,4): CONFIG+=c++11
	else: QMAKE_CXXFLAGS += -std=c++11
	QMAKE_CFLAGS_RELEASE ~= s/-O.*/
	QMAKE_CXXFLAGS_RELEASE ~= s/-O.*/
	QMAKE_ALLFLAGS_RELEASE += -O3 -ffast-math
}

# Handle flags for both C and C++
QMAKE_CFLAGS += $$QMAKE_ALLFLAGS
QMAKE_CXXFLAGS += $$QMAKE_ALLFLAGS
QMAKE_CFLAGS_DEBUG += $$QMAKE_ALLFLAGS_DEBUG
QMAKE_CXXFLAGS_DEBUG += $$QMAKE_ALLFLAGS_DEBUG
QMAKE_CFLAGS_RELEASE += $$QMAKE_ALLFLAGS_RELEASE
QMAKE_CXXFLAGS_RELEASE += $$QMAKE_ALLFLAGS_RELEASE
