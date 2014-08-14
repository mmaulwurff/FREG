######################################################################
# Automatically generated by qmake (2.01a) Thu Jun 13 12:57:30 2013
######################################################################

# Build options

# screen can be: cursed_screen, text_screen
CONFIG += cursed_screen
#CONFIG += text_screen

CONFIG += thread warn_on console
CONFIG += debug

# compile with clang:
#CONFIG += clang
#CONFIG += g++-old

VERSION = 0.3
VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -std=c++11 -pedantic -Weffc++
QMAKE_CXXFLAGS += -Wold-style-cast -Wfloat-equal -Woverloaded-virtual -Wundef

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

cursed_screen {
    TARGET = freg-nox
    QT -= gui
    DEFINES += CURSED_SCREEN
    HEADERS += screens/CursedScreen.h
    SOURCES += screens/CursedScreen.cpp
} else:text_screen {
    TARGET = freg-text
    QT -= gui
    DEFINES += TEXT_SCREEN
    HEADERS += screens/TextScreen.h
    SOURCES += screens/TextScreen.cpp
} else {
    error("define screen type in CONFIG!")
}

unix:cursed_screen {
    LIBS += -lncursesw
    target.path += /usr/bin
    INSTALLS += target
} else:win32 {
    # path =(
    LIBS += -LC:/Users/Alexander/src/FREG/pdcurses -lpdcurses
}

HEADERS += \
    BlockManager.h \
    CraftManager.h \
    DeferredAction.h \
    header.h \
    Player.h \
    Shred.h \
    screens/VirtScreen.h \
    World.h \
    worldmap.h \
    ShredStorage.h \
    Xyz.h \
    blocks/blocks.h \
    blocks/Block.h \
    blocks/Active.h \
    blocks/Inventory.h \
    blocks/Animal.h \
    blocks/Dwarf.h \
    blocks/Bucket.h \
    blocks/Illuminator.h \
    blocks/Weapons.h \
    blocks/Containers.h \
    blocks/RainMachine.h \
    blocks/Armour.h \
    screens/IThread.h
SOURCES += \
    BlockManager.cpp \
    CraftManager.cpp \
    DeferredAction.cpp \
    Lighting-inertia.cpp \
    main.cpp \
    Player.cpp \
    Shred-gen-flat.cpp \
    Shred.cpp \
    screens/VirtScreen.cpp \
    World.cpp \
    worldmap.cpp \
    ShredStorage.cpp \
    Xyz.cpp \
    blocks/blocks.cpp \
    blocks/Active.cpp \
    blocks/Dwarf.cpp \
    blocks/Weapons.cpp \
    blocks/Illuminator.cpp \
    blocks/Inventory.cpp \
    blocks/Bucket.cpp \
    blocks/Containers.cpp \
    blocks/Block.cpp \
    blocks/RainMachine.cpp \
    blocks/Armour.cpp \
    blocks/Animal.cpp \
    screens/IThread.cpp

TRANSLATIONS = \
    freg_ru.ts

DISTFILES += \
    texts/death.txt \
    texts/splash.txt \
    recipes/*.json \
    help_*/* \
    fregMap.vim \
    *.md

MOC_DIR = moc
OBJECTS_DIR = obj
QMAKE_CLEAN += -r moc obj
