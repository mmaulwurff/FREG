######################################################################
# Automatically generated by qmake (2.01a) Thu Jun 13 12:57:30 2013
######################################################################

CONFIG += qt thread warn_on console
CONFIG += debug
QT -= gui
LIBS += -lcurses

MOC_DIR = moc
OBJECTS_DIR = obj

VERSION = 0.2
VERSTR = '\\"$${VERSION}\\"'
DEFINES += VER=\"$${VERSTR}\"

TEMPLATE = app
TARGET = freg-nox-'"$${VERSION}"'
DEPENDPATH  += .
INCLUDEPATH += .

QMAKE_CXXFLAGS += -Wextra

#QMAKE_CXXFLAGS_DEBUG += -O -fno-inline
QMAKE_CXXFLAGS_DEBUG += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -s

# Input
HEADERS += \
	BlockManager.h \
	blocks.h \
	Block.h \
	Active.h \
	Inventory.h \
	Animal.h \
	Dwarf.h \
	CraftManager.h \
	DeferredAction.h \
	header.h \
	Player.h \
	screen.h \
	Shred.h \
	VirtScreen.h \
	world.h \
	worldmap.h \
	ShredStorage.h \
	Xyz.h
SOURCES += \
	BlockManager.cpp \
	blocks.cpp \
	Active.cpp \
	Dwarf.cpp \
	CraftManager.cpp \
	DeferredAction.cpp \
	Lighting-inertia.cpp \
	main.cpp \
	Player.cpp \
	screen.cpp \
	Shred-gen-flat.cpp \
	Shred.cpp \
	VirtScreen.cpp \
	world.cpp \
	worldmap.cpp \
	ShredStorage.cpp \
	Xyz.cpp
