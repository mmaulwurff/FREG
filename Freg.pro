TARGET   = freg
TEMPLATE = app
VERSION  = 0.3
DEFINES += VER=$$VERSION
QMAKE_TARGET_COPYRIGHT   = (C) 2012-2015 Alexander \'m8f\' Kromm
QMAKE_TARGET_DESCRIPTION = Freg, 3D game with TUI

CONFIG += warn_on console rtti_off exceptions_off
CONFIG( release, debug|release ):DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES *= QT_USE_QSTRINGBUILDER

#QMAKE_CXXFLAGS_DEBUG += -fno-inline
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS += \
    -isystem $$(QTDIR)/include/QtCore -isystem $$(QTDIR)/include/QtGui \
    -std=c++14 -O3 \
    -Wfloat-equal -Woverloaded-virtual -Wundef -fstrict-enums -pedantic \
    -Werror \
    -Wdisabled-optimization -Wcast-align -Wcast-qual \
    -Wmissing-include-dirs -Wredundant-decls -Wshadow

!for_coverity {
    QMAKE_CXXFLAGS += -Weffc++
}

clang {
    QMAKE_CXX  = clang++
    QMAKE_LINK = clang++
} else {
    QMAKE_CXXFLAGS_RELEASE += -s
    QMAKE_CXXFLAGS += -Wdouble-promotion -Wlogical-op
}

unix {
    QMAKE_CXXFLAGS += -Wold-style-cast
    LIBS += -lncursesw
    target.path += /usr/bin
    INSTALLS += target
} else {
    LIBS           += $$PWD/pdcurses/libcurses.lib
    PRE_TARGETDEPS += $$PWD/pdcurses/libcurses.lib
}

HEADERS +=  *.h \
     blocks/*.h \
    screens/*.h

CONFIG( debug, debug|release ) {
    SOURCES +=  *.cpp \
         blocks/*.cpp \
        screens/*.cpp
} else {
    SOURCES += everything/everything.cpp
}

#CONFIG += console_only
CONFIG(console_only) {
    DEFINES += CONSOLE
}

TRANSLATIONS = *.ts

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
