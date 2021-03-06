TARGET   = freg
TEMPLATE = app
VERSION  = 0.3
DEFINES += VER=$$VERSION
QMAKE_TARGET_COPYRIGHT   = (C) 2012-2015 Alexander \'m8f\' Kromm
QMAKE_TARGET_DESCRIPTION = Freg, 3D game with TUI

CONFIG += warn_on console rtti_off exceptions_off
CONFIG( release, debug|release ):DEFINES += QT_NO_DEBUG_OUTPUT NDEBUG
DEFINES *= QT_USE_QSTRINGBUILDER
DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

#QMAKE_CXXFLAGS_DEBUG += -fno-inline
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS += \
    -std=c++14 \
    -O3 \
    -Wfloat-equal -Woverloaded-virtual -Wundef -fstrict-enums -pedantic \
    -Werror -Wno-error=strict-overflow \
    -Wdisabled-optimization -Wcast-align -Wcast-qual \
    -Wextra \
    -Wformat=2 \
    -Wuninitialized \
    -Wmissing-include-dirs -Wredundant-decls -Wshadow

gcc {
!clang {
    QMAKE_CXXFLAGS_RELEASE += -s
    QMAKE_CXXFLAGS += -Wdouble-promotion -Wlogical-op
}
}

unix {
    QMAKE_CXXFLAGS += -Wold-style-cast
    QMAKE_CXXFLAGS += -Weffc++
    LIBS += -lncursesw
    target.path += /usr/bin
    INSTALLS += target
} else {
    LIBS           += $$PWD/pdcurses/libcurses.lib
    PRE_TARGETDEPS += $$PWD/pdcurses/libcurses.lib
}

linux-icc-64 {
    QMAKE_CXXFLAGS -= \
        -fstrict-enums -Wcast-align -Wredundant-decls -Wdouble-promotion \
        -Wlogical-op -Wold-style-cast -O3
    QMAKE_CXXFLAGS += -wd2015,2012,2017
}

INCLUDEPATH += include

HEADERS +=  $$PWD/include/*.h \
     $$PWD/include/blocks/*.h \
    $$PWD/include/screens/*.h

CONFIG( debug, debug|release ) {
    SOURCES +=  $$PWD/src/*.cpp \
         $$PWD/src/blocks/*.cpp \
        $$PWD/src/screens/*.cpp
} else {
    SOURCES += src/everything/everything.cpp
}

#CONFIG += console_only
CONFIG(console_only) {
    DEFINES += CONSOLE
}

TRANSLATIONS = \
    freg_ru_RU.ts \

RESOURCES = resources.qrc

DISTFILES += \
    recipes/*.json \
    help_*/* \
    rooms/*.room \
    freg_ru.qm \
    debian/c* \
    debian/rules \
    debian/usr/share/doc/freg/copyright \
    debian/source/format \
    debian/source/include-binaries \
    fregMap.vim \
    *.md

MOC_DIR     = moc
RCC_DIR     = res
OBJECTS_DIR = obj
