    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

/**\file VirtScreen.cpp
 * \brief This file provides definitions of code common to all freg screens. */

#include "VirtScreen.h"
#include "Player.h"
#include "world.h"

void VirtScreen::ConnectWorld() {
    connect(w, SIGNAL(Updated(int, int, int)), SLOT(Update(int, int, int)),
        Qt::DirectConnection);
    connect(w, SIGNAL(UpdatedAround(int, int, int, ushort)),
        SLOT(UpdateAround(int, int, int, int)),
        Qt::DirectConnection);
}

void VirtScreen::UpdatesEnd() {}
void VirtScreen::DeathScreen() {}

VirtScreen::VirtScreen(World * const world_, Player * const player_) :
        w(world_),
        player(player_)
{
    connect(w, SIGNAL(Notify(const QString)),
        SLOT(Notify(const QString)));
    connect(player, SIGNAL(Notify(const QString)),
        SLOT(Notify(const QString)));
    connect(player, SIGNAL(ShowFile(QString)), SLOT(DisplayFile(QString)));
    connect(player, SIGNAL(GetFocus(int *, int *, int *)),
        SLOT(ActionXyz(int *, int *, int *)), Qt::DirectConnection);

    connect(w, SIGNAL(GetString(QString &)),
        SLOT(PassString(QString &)), Qt::DirectConnection);
    connect(player, SIGNAL(GetString(QString &)),
        SLOT(PassString(QString &)), Qt::DirectConnection);

    connect(player, SIGNAL(Updated()), SLOT(UpdatePlayer()),
        Qt::DirectConnection);
    connect(w, SIGNAL(ReConnect()), SLOT(ConnectWorld()),
        Qt::DirectConnection);
    connect(w, SIGNAL(UpdatedAll()), SLOT(UpdateAll()),
        Qt::DirectConnection);
    connect(w, SIGNAL(Moved(const int)), SLOT(Move(const int)),
        Qt::DirectConnection);
    ConnectWorld();
    connect(w, SIGNAL(UpdatesEnded()), SLOT(UpdatesEnd()),
        Qt::DirectConnection);

    connect(player, SIGNAL(Destroyed()), SLOT(DeathScreen()),
        Qt::DirectConnection );
}

void VirtScreen::CleanAll() {}
VirtScreen::~VirtScreen() { CleanAll(); }

int  VirtScreen::GetChar() const { return 0; }
void VirtScreen::FlushInput() const {}
void VirtScreen::ControlPlayer(int) {}
void VirtScreen::DisplayFile(QString /* path */) {}

void VirtScreen::ActionXyz(int * x, int * y, int * z) const {
    w->Focus(player->X(), player->Y(), player->Z(), x, y, z, player->GetDir());
}

World * VirtScreen::GetWorld() const { return w; }

char VirtScreen::CharName(const int kind, const int sub) const {
    switch ( kind )  {
    case BUSH:   return ';';
    case CREATOR:
    case DWARF:  return '@';
    case LIQUID: return '~';
    case GRASS:  return ( FIRE == sub ) ? 'f' : '.';
    case RABBIT: return 'r';
    case CLOCK:  return 'c';
    case PLATE:  return '_';
    case LADDER: return '^';
    case PICK:   return '\\';
    case SHOVEL: return '|';
    case HAMMER: return 'T';
    case AXE:    return '/';
    case BELL:   return 'b';
    case BUCKET: return 'u';
    case TEXT:   return '?';
    case PREDATOR: return '!';
    case WORKBENCH: return '*';
    case CONTAINER: return '&';
    case TELEGRAPH: return 't';
    case DOOR:        return ( STONE == sub ) ? '#' : '\'';
    case LOCKED_DOOR: return ( STONE == sub ) ? '#' : '`';
    case ILLUMINATOR: return 'i';
    case WEAPON: switch ( sub ) {
        default: fprintf(stderr, "Screen::CharName: weapon sub ?: %d\n", sub);
        // no break;
        case SKY:   return ' ';
        case STONE: return '.';
        case IRON:
        case BONE:
        case WOOD:  return '/';
    } break;
    case ACTIVE: switch ( sub ) {
        case SAND:  return '.';
        case WATER: return '*';
        case STONE: return ':';
        default: fprintf(stderr, "Screen::CharName: active sub ?: %d\n", sub);
    } // no break;
    default: switch ( sub ) {
        default: fprintf(stderr, "Screen::CharName: sub (?): %d\n", sub);
        case NULLSTONE:
        case IRON:
        case CLAY:
        case WOOD:
        case GOLD:
        case MOSS_STONE:
        case SAND:
        case STONE: return '#';
        case GLASS: return 'g';
        case AIR:   return ' ';
        case STAR:  return '.';
        case WATER: return '~';
        case SOIL:  return '.';
        case ROSE:  return ';';
        case A_MEAT:
        case H_MEAT:
        case HAZELNUT: return ',';
        case SKY:
        case SUN_MOON: return ' ';
        case GREENERY: return '%';
        }
    }
} // char VirtScreen::CharName(int kind, int sub)
