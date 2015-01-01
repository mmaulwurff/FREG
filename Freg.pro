######################################################################
# Automatically generated by qmake (2.01a) Thu Jun 13 12:57:30 2013
######################################################################

# Build options

QT -= gui
TARGET = freg-nox

CONFIG += thread warn_on console c++11 rtti_off
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# compile with clang:
#CONFIG += clang
#CONFIG += g++-old

VERSION = 0.3
VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -Wextra -std=c++11 -pedantic
QMAKE_CXXFLAGS += -Wfloat-equal -Woverloaded-virtual -Wundef

#QMAKE_CXXFLAGS_DEBUG += -fno-inline
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -DQT_NO_DEBUG
QMAKE_CXXFLAGS += -O3
QMAKE_CXXFLAGS += -fstrict-enums -fno-rtti

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
DEFINES += COMPILER=\"\\\"$${QMAKE_CXX}\"\\\"

unix {
    LIBS += -lncursesw
    target.path += /usr/bin
    INSTALLS += target
    QMAKE_CXXFLAGS += -Weffc++ -Wold-style-cast -Werror
} else {
    LIBS += -Lpdcurses -lcurses
}

HEADERS += *.h  \
    blocks/*.h  \
    screens/*.h \

SOURCES += *.cpp  \
    blocks/*.cpp  \
    screens/*.cpp \

INCLUDEPATH += .

TRANSLATIONS =    \
    freg_ru_RU.ts \

RESOURCES = resources.qrc

DISTFILES += \
    texts/death.txt \
    texts/splash.txt \
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
    *.md \

MOC_DIR = moc
OBJECTS_DIR = obj
