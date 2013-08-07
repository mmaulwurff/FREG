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

QMAKE_CXXFLAGS_DEBUG += -O3
QMAKE_CFLAGS_DEBUG   += -O3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE   -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -s
QMAKE_CFLAGS_RELEASE   += -O3 -s

# Input
HEADERS += \
	BlockManager.h \
	blocks.h \
	CraftManager.h \
	DeferredAction.h \
	header.h \
	Player.h \
	screen.h \
	Shred.h \
	VirtScreen.h \
	world.h \
	worldmap.h \
	ShredStorage.h
SOURCES += \
	BlockManager.cpp \
	blocks.cpp \
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
	ShredStorage.cpp
