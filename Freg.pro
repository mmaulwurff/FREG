# freg Qt project file

TARGET   = freg
TEMPLATE = app
VERSION  = 0.3
DEFINES += VER=$$VERSION
QMAKE_TARGET_COPYRIGHT   = (C) 2012-2015 Alexander \'m8f\' Kromm
QMAKE_TARGET_DESCRIPTION = Freg - open world with text graphics.

CONFIG += warn_on console rtti_off exceptions_off
CONFIG( release, debug|release ):DEFINES += QT_NO_DEBUG_OUTPUT

QMAKE_CXXFLAGS += -Wfloat-equal -Woverloaded-virtual -Wundef -fstrict-enums
QMAKE_CXXFLAGS += -pedantic -Weffc++ -Werror

#QMAKE_CXXFLAGS_DEBUG += -fno-inline
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS += -O3

DEFINES *= QT_USE_QSTRINGBUILDER

QMAKE_CXXFLAGS += -isystem $$(QTDIR)/include/QtCore
QMAKE_CXXFLAGS += -isystem $$(QTDIR)/include/QtGui
QMAKE_CXXFLAGS += -std=c++14

clang {
    QMAKE_CXX  = clang++
    QMAKE_LINK = clang++
} else {
    QMAKE_CXXFLAGS_RELEASE += -s
    QMAKE_CXXFLAGS += -Wdouble-promotion
    g++-old {
        QMAKE_CXX  = g++-4.8
        QMAKE_LINK = g++-4.8
    }
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
