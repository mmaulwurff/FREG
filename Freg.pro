######################################################################
# Automatically generated by qmake (2.01a) Thu Jun 13 12:57:30 2013
######################################################################

CONFIG += thread warn_on console
CONFIG += debug
# screen can be: cursed_screen, text_screen
CONFIG += cursed_screen
#CONFIG += text_screen

# compile with clang:
#CONFIG += clang

VERSION = 0.2
VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -Wextra -Werror -std=c++11 -pedantic
#QMAKE_CXXFLAGS += -Wno-error=strict-overflow # Qt 5.2 has some problem

#QMAKE_CXXFLAGS_DEBUG += -fno-inline
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS += -O3


clang {
    QMAKE_CXX  = clang++
    QMAKE_LINK = clang++
} else {
    QMAKE_CXXFLAGS_RELEASE += -s
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
    world.h \
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
    blocks/Container.h \
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
    world.cpp \
    worldmap.cpp \
    ShredStorage.cpp \
    Xyz.cpp \
    Global.cpp \
    blocks/blocks.cpp \
    blocks/Active.cpp \
    blocks/Dwarf.cpp \
    blocks/Weapons.cpp \
    blocks/Illuminator.cpp \
    blocks/Inventory.cpp \
    blocks/Bucket.cpp \
    blocks/Container.cpp \
    blocks/Block.cpp \
    screens/IThread.cpp

TRANSLATIONS = \
    freg_ru.ts

DISTFILES += texts/*.txt

MOC_DIR = moc
OBJECTS_DIR = obj
