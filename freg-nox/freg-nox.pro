######################################################################
# Automatically generated by qmake (2.01a) Fri Feb 8 19:47:08 2013
######################################################################

TEMPLATE = app
TARGET = freg-nox
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += qt thread debug
QT -= gui
LIBS += -lcurses

# Input
HEADERS += blocks.h \
           header.h \
           Player.h \
           screen.h \
           Shred.h  \
           VirtScreen.h \
           world.h \
           BlockManager.h \
           CraftManager.h

SOURCES += blocks.cpp \
           Block.cpp \
	   Bush.cpp \
           Lighting.cpp \
           main.cpp \
           Player.cpp \
           screen.cpp \
           Shred.cpp \
           VirtScreen.cpp \
           world.cpp \
           BlockManager.cpp \
           CraftManager.cpp
