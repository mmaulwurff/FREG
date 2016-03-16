    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
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

/** @file
 * This file provides definitions of code common to all freg screens. */

#include "screens/VirtualScreen.h"
#include "Player.h"
#include "World.h"

void VirtualScreen::UpdatesEnd() {}

VirtualScreen::VirtualScreen(Player* const player_)
    : player(player_)
    , settings(home_path + Str("freg.ini"), QSettings::IniFormat)
    , previousCommand(settings.value( Str("last_command")
                                    , Str("moo") ).toString())
{
    World* const world = World::GetWorld();
    connect( world, &World::Notify, this, &VirtualScreen::Notify,
        Qt::DirectConnection);
    connect(player, &Player::ShowFile, this, &VirtualScreen::DisplayFile);
    connect(player, &Player::GetFocus, this, &VirtualScreen::ActionXyz,
        Qt::DirectConnection);

    connect( world, &World::GetString, this, &VirtualScreen::PassString,
        Qt::DirectConnection);
    connect(player, &Player::GetString, this, &VirtualScreen::PassString,
        Qt::DirectConnection);

    connect(player, &Player::Updated, this, &VirtualScreen::UpdatePlayer,
        Qt::DirectConnection);
    connect(world, &World::UpdatedAll, this, &VirtualScreen::UpdateAll,
        Qt::DirectConnection);
    connect(world, &World::Moved, this, &VirtualScreen::Move,
        Qt::DirectConnection);
    connect(world, &World::UpdatedAround, this, &VirtualScreen::UpdateAround,
        Qt::DirectConnection);
    connect(world, &World::UpdatesEnded, this, &VirtualScreen::UpdatesEnd,
        Qt::DirectConnection);
    connect(this, &VirtualScreen::PauseWorld,  world, &World::Pause);
    connect(this, &VirtualScreen::ResumeWorld, world, &World::Start);

    connect(player, &Player::Destroyed, this, &VirtualScreen::DeathScreen,
        Qt::DirectConnection );
}

VirtualScreen::~VirtualScreen() {}

void VirtualScreen::DisplayFile(const QString& /* path */) {}

void VirtualScreen::ActionXyz P3(int* const, x, y, z) const {
    World::GetCWorld()->Focus(player->X(), player->Y(), player->Z(),
        x, y, z, player->GetDir());
}

int VirtualScreen::Color(const int kind, const int sub) {
    // default colors are defined in header.h in SUB_TABLE.
    static const int colors[] { SUB_TABLE(X_COLOR) };
    switch ( kind ) { // foreground_background
    case FALLING: switch ( sub ) {
        case WATER: return   CYAN_WHITE;
        case SAND:  return YELLOW_WHITE;
        } // no break;
    default: return colors[sub];
    case LIQUID: switch ( sub ) {
        case WATER:     return CYAN_BLUE;
        default:        return  RED_YELLOW;
        case SUB_CLOUD:
        case ACID:
        case H_MEAT:
        case A_MEAT:    return colors[sub];
        } break;
    case DWARF: switch ( sub ) {
        case ADAMANTINE: return colors[ADAMANTINE];
        default:         return WHITE_BLUE ;
        } break;
    case RABBIT:    return  RED_WHITE;
    case PREDATOR:  return  RED_BLACK;
    case TELEGRAPH: return BLUE_BLUE;
    case MEDKIT:    return  RED_WHITE;
    case TELEPORT:  return  RED_BLUE;
    }
}

char VirtualScreen::CharName(const int kind, const int sub) {
    // do not use abcdef as they can be used as distance specifiers.
    // default characters are defined in header.h in KIND_TABLE.
    switch ( kind ) {
    case GRASS: return ( FIRE  == sub ) ? '^' : '.';
    case DOOR:  return ( STONE == sub ) ? '#' : '\'';
    case FALLING: switch ( sub ) {
        case SAND:     return '.';
        case WATER:    return '*';
        case STONE:    return ':';
        } // no break;
    case BLOCK: switch ( sub ) {
        case SOIL:     return '.';
        case GREENERY: return '%';
        case A_MEAT:
        case H_MEAT:   return ',';
        case GLASS:    return 'g';
        case ROSE:     return ';';
        case COAL:     return '*';
        case STAR:     return '.';
        case SKY:
        case AIR:      return ' ';
        } break;
    case WEAPON: switch ( sub ) {
        case STONE:    return '.';
        case SKY:      return ' ';
        case SUB_NUT:  return ',';
        } break;
    }
    static const char characters[] = { KIND_TABLE(X_CHAR) };
    return characters[kind];
}

bool VirtualScreen::ProcessCommand(const QString& command) {
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("moo"):
        Notify(Str("^__^"));
        Notify(Str("(oo)\\_______"));
        Notify(Str("(__)\\       )\\/\\"));
        Notify(Str("    ||----w |"));
        Notify(Str("    ||     ||"));
        return true;
    default: return false;
    }
}

// Define pure virtual functions to simplify debugging
void VirtualScreen::Move(int)                   { Q_UNREACHABLE(); }
void VirtualScreen::UpdateAll()                 { Q_UNREACHABLE(); }
void VirtualScreen::DeathScreen()               { Q_UNREACHABLE(); }
void VirtualScreen::UpdatePlayer()              { Q_UNREACHABLE(); }
void VirtualScreen::PassString(QString &) const { Q_UNREACHABLE(); }
void VirtualScreen::UpdateAround(int, int, int) { Q_UNREACHABLE(); }
void VirtualScreen::Notify(const QString&) const { Q_UNREACHABLE(); }
