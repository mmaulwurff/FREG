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

#ifndef HEADER_H
#define HEADER_H

#include <QtGlobal>
#include <QLocale>

#ifdef Q_CC_MSVC
#define not !
#endif

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

const int SHRED_WIDTH = 16;
const int HEIGHT = 128;

const int SECONDS_IN_HOUR = 60;
const int SECONDS_IN_DAY = 24*SECONDS_IN_HOUR;
const int END_OF_NIGHT   =  6*SECONDS_IN_HOUR;
const int END_OF_MORNING = 12*SECONDS_IN_HOUR;
const int END_OF_NOON    = 18*SECONDS_IN_HOUR;
const int END_OF_EVENING =  0*SECONDS_IN_HOUR;
const int SECONDS_IN_NIGHT = END_OF_NIGHT;
const int SECONDS_IN_DAYLIGHT = SECONDS_IN_DAY-END_OF_NIGHT;

const int MAX_DURABILITY = 100;
const int MAX_BREATH = 60;

const uchar MAX_LIGHT_RADIUS = 15;

const QString locale = QLocale::system().name();

enum shred_type {
    SHRED_PLAIN     = '.',
    SHRED_TESTSHRED = 't',
    SHRED_PYRAMID   = 'p',
    SHRED_HILL      = '+',
    SHRED_DESERT    = ':',
    SHRED_WATER     = '~',
    SHRED_FOREST    = '%',
    SHRED_MOUNTAIN  = '^',
    SHRED_EMPTY     = '_',
    SHRED_CHAOS     = '!',
    SHRED_CASTLE    = 'c',
    SHRED_NULLMOUNTAIN = '#',
    SHRED_NORMAL_UNDERGROUND = '-',
};

const int DEFAULT_MAP_SIZE = 75U;
const char DEFAULT_SHRED = SHRED_PLAIN;
const char OUT_BORDER_SHRED = SHRED_WATER;

enum dirs {
    UP,     ///< 0
    DOWN,   ///< 1
    NORTH,  ///< 2
    SOUTH,  ///< 3
    EAST,   ///< 4
    WEST,   ///< 5
    NOWHERE ///< 6
};

enum push_reaction {
    MOVABLE,
    ENVIRONMENT,
    NOT_MOVABLE,
    MOVE_UP,
    JUMP
};

enum times_of_day { MORNING, NOON, EVENING, NIGHT };

enum damage_kinds {
    MINE,    ///<  0
    DIG,     ///<  1
    CUT,     ///<  2
    THRUST,  ///<  3
    CRUSH,   ///<  4
    HEAT,    ///<  5
    FREEZE,  ///<  6
    MELT,    ///<  7
    ELECTRO, ///<  8
    HUNGER,  ///<  9
    BREATH,  ///< 10
    BITE,    ///< 11
    TIME,    ///< 12
    NO_HARM, ///< 13
    DAMAGE_FALL,  ///< 14
    DAMAGE_HANDS, ///< 15
}; // enum damage_kinds

/// Kinds of atom
enum kinds {
    // add new kinds to bottom (before LAST_KIND).
    // changind kind codes will break file compatibility.
    BLOCK,       ///<  0
    BELL,        ///<  1
    CONTAINER,   ///<  2
    DWARF,       ///<  3
    ANIMAL,      ///<  4
    PICK,        ///<  5
    TELEGRAPH,   ///<  6
    LIQUID,      ///<  7
    GRASS,       ///<  8
    BUSH,        ///<  9
    RABBIT,      ///< 10
    ACTIVE,      ///< 11
    CLOCK,       ///< 12
    PLATE,       ///< 13
    WORKBENCH,   ///< 14
    WEAPON,      ///< 15
    LADDER,      ///< 16
    DOOR,        ///< 17
    LOCKED_DOOR, ///< 18
    CREATOR,     ///< 19
    TEXT,        ///< 20
    MAP,         ///< 21
    PREDATOR,    ///< 22
    BUCKET,      ///< 23
    SHOVEL,      ///< 24
    AXE,         ///< 25
    HAMMER,      ///< 26
    ILLUMINATOR, ///< 27
    /// Nothing is LAST_KIND.
    LAST_KIND // keep it last in this list.
}; // enum kinds

/// Substance block is made from.
/** Don't make blocks from SKY and STAR, they are special for shred loading
 *  and saving.
 *  Don't make non-BLOCK blocks from air, otherwise leaks are possible. */
enum subs {
    // do not change order, this will break file compatibility.
    // add new substances right before LAST_SUB.
    STONE,      ///<   0
    MOSS_STONE, ///<   1
    NULLSTONE,  ///<   2
    SKY,        ///<   3
    STAR,       ///<   4
    SUN_MOON,   ///<   5
    SOIL,       ///<   6
    H_MEAT,     ///<   7 (hominid meat)
    A_MEAT,     ///<   8 (animal meat)
    GLASS,      ///<   9
    WOOD,       ///<  10
    DIFFERENT,  ///<  11
    IRON,       ///<  12
    WATER,      ///<  13
    GREENERY,   ///<  14
    SAND,       ///<  15
    HAZELNUT,   ///<  16
    ROSE,       ///<  17
    CLAY,       ///<  18
    AIR,        ///<  19
    PAPER,      ///<  20
    GOLD,       ///<  21
    BONE,       ///<  22
    STEEL,      ///<  23
    ADAMANTINE, ///<  24
    FIRE,       ///<  25
    COAL,       ///<  26
    EXPLOSIVE,  ///<  27
    /// Nothing is made from LAST_SUB.
    LAST_SUB // keep it last in this list
}; // enum subs

enum usage_types {
    USAGE_TYPE_NO,
    USAGE_TYPE_OPEN,
    USAGE_TYPE_READ,
    USAGE_TYPE_READ_IN_INVENTORY,
    USAGE_TYPE_POUR,
    USAGE_TYPE_SET_FIRE
};

enum transparency {
    BLOCK_OPAQUE,
    BLOCK_TRANSPARENT,
    INVISIBLE,
    NONSTANDARD = 6,
    UNDEF // temporary, doesn't appear in world.
};

bool IsLikeAir(int sub);

#endif // HEADER_H
