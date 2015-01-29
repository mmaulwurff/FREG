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

/**\file VirtScreen.cpp
 * \brief This file provides definitions of code common to all freg screens. */

#include "VirtScreen.h"
#include "Player.h"
#include "World.h"

void VirtScreen::UpdatesEnd() {}
void VirtScreen::DeathScreen() {}

VirtScreen::VirtScreen(Player * const player_) :
        player(player_),
        settings(home_path + QStringLiteral("freg.ini"), QSettings::IniFormat),
        previousCommand( settings.value(QStringLiteral("last_command"),
            QStringLiteral("moo")).toString() )
{
    World * const world = World::GetWorld();
    connect( world, &World::Notify, this, &VirtScreen::Notify,
        Qt::DirectConnection);
    connect(player, &Player::Notify, this, &VirtScreen::Notify,
        Qt::DirectConnection);
    connect(player, &Player::ShowFile, this, &VirtScreen::DisplayFile);
    connect(player, &Player::GetFocus, this, &VirtScreen::ActionXyz,
        Qt::DirectConnection);

    connect( world, &World::GetString, this, &VirtScreen::PassString,
        Qt::DirectConnection);
    connect(player, &Player::GetString, this, &VirtScreen::PassString,
        Qt::DirectConnection);

    connect(player, &Player::Updated, this, &VirtScreen::UpdatePlayer,
        Qt::DirectConnection);
    connect(world, &World::UpdatedAll, this, &VirtScreen::UpdateAll,
        Qt::DirectConnection);
    connect(world, &World::Moved, this, &VirtScreen::Move,
        Qt::DirectConnection);
    connect(world, &World::Updated, this, &VirtScreen::Update,
        Qt::DirectConnection);
    connect(world, &World::UpdatedAround, this, &VirtScreen::UpdateAround,
        Qt::DirectConnection);
    connect(world, &World::UpdatesEnded, this, &VirtScreen::UpdatesEnd,
        Qt::DirectConnection);
    connect(this, &VirtScreen::PauseWorld,  world, &World::Pause);
    connect(this, &VirtScreen::ResumeWorld, world, &World::Start);

    connect(player, &Player::Destroyed, this, &VirtScreen::DeathScreen,
        Qt::DirectConnection );
}

VirtScreen::~VirtScreen() {}

void VirtScreen::DisplayFile(QString /* path */) {}

void VirtScreen::ActionXyz(int * x, int * y, int * z) const {
    World::GetWorld()->Focus(player->X(), player->Y(), player->Z(), x, y, z,
        player->GetDir());
}

int VirtScreen::Color(const int kind, const int sub) {
    switch ( kind ) { // foreground_background
    case FALLING: switch ( sub ) {
        case WATER: return   CYAN_WHITE;
        case SAND:  return YELLOW_WHITE;
        } // no break;
    default: switch ( sub ) {
        default:         return WHITE_BLACK;
        case STONE:      return BLACK_WHITE;
        case GREENERY:   return BLACK_GREEN;
        case WOOD:
        case SUB_NUT:
        case SOIL:       return   BLACK_YELLOW;
        case SAND:       return   WHITE_YELLOW;
        case COAL:       return   BLACK_WHITE;
        case IRON:       return   BLACK_BLACK;
        case A_MEAT:     return   WHITE_RED;
        case H_MEAT:     return   BLACK_RED;
        case WATER:      return   WHITE_CYAN;
        case GLASS:      return    BLUE_WHITE;
        case NULLSTONE:  return MAGENTA_BLACK;
        case MOSS_STONE: return   GREEN_WHITE;
        case ROSE:       return     RED_GREEN;
        case CLAY:       return   WHITE_RED;
        case PAPER:      return MAGENTA_WHITE;
        case GOLD:       return   WHITE_YELLOW;
        case BONE:       return MAGENTA_WHITE;
        case EXPLOSIVE:  return   WHITE_RED;
        case DIAMOND:    return    CYAN_WHITE;
        case ADAMANTINE: return    CYAN_BLACK;
        case SKY:
        case STAR:       return   WHITE_BLACK;
        case SUB_DUST:   return   BLACK_BLACK;
        case FIRE:       return     RED_YELLOW;
        case ACID:       return   GREEN_GREEN;
        } break;
    case LIQUID: switch ( sub ) {
        case WATER:     return CYAN_BLUE;
        case SUB_CLOUD: return BLACK_WHITE;
        case ACID:      return GREEN_GREEN;
        case H_MEAT:
        case A_MEAT:    return BLACK_RED;
        default:        return RED_YELLOW;
        } break;
    case DWARF: switch ( sub ) {
        case ADAMANTINE: return  CYAN_BLACK;
        default:         return WHITE_BLUE ;
        } break;
    case RABBIT:    return  RED_WHITE;
    case PREDATOR:  return  RED_BLACK;
    case TELEGRAPH: return BLUE_BLUE;
    case MEDKIT:    return  RED_WHITE;
    case TELEPORT:  return  RED_BLUE;
    }
} // color_pairs Screen::Color(int kind, int sub)

char VirtScreen::CharName(const int kind, const int sub) {
    // do not use abcdef as they can be used as distance specifiers.
    // default charecters are defined in header.h in KIND_TABLE.
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

bool VirtScreen::ProcessCommand(const QString command) {
    switch ( Player::UniqueIntFromString(qPrintable(command)) ) {
    case Player::UniqueIntFromString("moo"):
        Notify(QStringLiteral("^__^"));
        Notify(QStringLiteral("(oo)\\_______"));
        Notify(QStringLiteral("(__)\\       )\\/\\"));
        Notify(QStringLiteral("    ||----w |"));
        Notify(QStringLiteral("    ||     ||"));
        return true;
    default: return false;
    }
}

// Define pure virtual functions to simplify debugging
void VirtScreen::Move(int)                        { Q_UNREACHABLE(); }
void VirtScreen::UpdateAll()                      { Q_UNREACHABLE(); }
void VirtScreen::UpdatePlayer()                   { Q_UNREACHABLE(); }
void VirtScreen::Update(int, int, int)            { Q_UNREACHABLE(); }
void VirtScreen::Notify(QString) const            { Q_UNREACHABLE(); }
void VirtScreen::PassString(QString &) const      { Q_UNREACHABLE(); }
void VirtScreen::UpdateAround(int, int, int, int) { Q_UNREACHABLE(); }
