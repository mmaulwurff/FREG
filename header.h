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

#ifndef HEADER_H
#define HEADER_H

#include <QtGlobal>

#ifdef QT_NO_DEBUG
const bool DEBUG = false;
#else
const bool DEBUG = true;
#endif

extern const QString home_path;

enum sizes {
    SHRED_WIDTH_BITSHIFT = 4,
    SHRED_WIDTH = 16,
    HEIGHT = 256,
    DEFAULT_MAP_SIZE = 79,
    MAX_NOTE_LENGTH = 144,
};

enum times {
    SECONDS_IN_HOUR = 60,
    SECONDS_IN_DAY = 24*SECONDS_IN_HOUR,
    END_OF_NIGHT   =  6*SECONDS_IN_HOUR,
    END_OF_MORNING = 12*SECONDS_IN_HOUR,
    END_OF_NOON    = 18*SECONDS_IN_HOUR,
    END_OF_EVENING =  0*SECONDS_IN_HOUR,
    SECONDS_IN_NIGHT = END_OF_NIGHT,
    SECONDS_IN_DAYLIGHT = SECONDS_IN_DAY-END_OF_NIGHT
};

enum shred_type {
    SHRED_PLAIN     = '.',
    SHRED_TESTSHRED = 'T',
    SHRED_PYRAMID   = 'P',
    SHRED_HILL      = '+',
    SHRED_DESERT    = ':',
    SHRED_WATER     = '~',
    SHRED_FOREST    = '%',
    SHRED_MOUNTAIN  = '^',
    SHRED_EMPTY     = '_',
    SHRED_CHAOS     = '!',
    SHRED_CASTLE    = 'C',
    SHRED_WASTE     = '=',
    SHRED_ACID_LAKE = 'a',
    SHRED_LAVA_LAKE = 'l',
    SHRED_CRATER    = 'c',
    SHRED_DEAD_FOREST  = 'f',
    SHRED_DEAD_HILL    = '*',
    SHRED_NULLMOUNTAIN = '#',
    SHRED_NORMAL_UNDERGROUND = '-',
    SHRED_DEFAULT    = SHRED_PLAIN,
    SHRED_OUT_BORDER = SHRED_WATER,
};

enum dirs {
    ANYWHERE = 0,
    UP = 0, ///< 0
    DOWN,   ///< 1
    NORTH,  ///< 2
    EAST,   ///< 3
    SOUTH,  ///< 4
    WEST    ///< 5
};
const int DIRS_COUNT = WEST + 1;

enum push_reaction {
    MOVABLE,
    ENVIRONMENT,
    NOT_MOVABLE,
    MOVE_UP,
    JUMP,
    DAMAGE,
};

enum times_of_day {
    TIME_NIGHT,
    TIME_MORNING,
    TIME_NOON,
    TIME_EVENING
};

/** \page kinds List of available block kinds
 *  Complete list.
 *  These kinds can be used as parameters to `get KIND SUB` command.
 *  Changind kind order will break file compatibility.
 *  Do not use space in strings, use '_'.
 *  Add new kinds to bottom.
 *  Define X as "X(a, b) a," to get column 1 and "X(a, b) b," to get column 2.
 *  \snippet header.h List of kinds */
/// [List of kinds]
#define KIND_TABLE \
X(BLOCK,        "block")\
X(BELL,         "bell")\
X(CONTAINER,    "container")\
X(DWARF,        "intellectual")\
X(PICK,         "pick")\
X(LIQUID,       "liquid")\
X(GRASS,        "grass")\
X(BUSH,         "bush")\
X(RABBIT,       "rabbit")\
X(FALLING,      "falling")\
X(CLOCK,        "clock")\
X(PLATE,        "plate")\
X(WORKBENCH,    "workbench")\
X(WEAPON,       "weapon")\
X(LADDER,       "ladder")\
X(DOOR,         "door")\
X(BOX,          "box")\
X(KIND_TEXT,    "text")\
X(MAP,          "map")\
X(PREDATOR,     "predator")\
X(BUCKET,       "bucket")\
X(SHOVEL,       "shovel")\
X(AXE,          "axe")\
X(HAMMER,       "hammer")\
X(ILLUMINATOR,  "illuminator")\
X(RAIN_MACHINE, "rain_machine")\
X(CONVERTER,    "converter")\
X(ARMOUR,       "armour")\
X(HELMET,       "helmet")\
X(BOOTS,        "boots")\
X(TELEGRAPH,    "telegraph")\
X(MEDKIT,       "medkit")\
X(FILTER,       "filter")\
X(INFORMER,     "informer")\
X(TELEPORT,     "teleport")\
X(ACCUMULATOR,  "accumulator")\
/// [List of kinds]

/** \page subs List of available substances
 *  Complete list.
 *  These substances can be used as parameters to `get KIND SUB` command.
 *  Don't make blocks (BLOCK kind) from SKY and STAR, they are special for
 *  shred loading and saving.
 *  Don't make non-BLOCK blocks from air, otherwise memory leaks are possible.
 *  Don't change order, this will break save file compatibility.
 *  Add new substances to bottom.
 *  Define X as "X(a, b) a," to get column 1 and "X(a, b) b," to get column 2.
 *  \snippet BlockFactory.cpp List of subs */
/// [List of subs]
#define SUB_TABLE \
X(STONE,       "stone")\
X(MOSS_STONE,  "moss stone")\
X(NULLSTONE,   "nullstone")\
X(SKY,         "sky")\
X(STAR,        "star")\
X(DIAMOND,     "diamond")\
X(SOIL,        "soil")\
X(H_MEAT,      "meat_of_intellectual")\
X(A_MEAT,      "animal_meat")\
X(GLASS,       "glass")\
X(WOOD,        "wood")\
X(DIFFERENT,   "different")\
X(IRON,        "iron")\
X(WATER,       "water")\
X(GREENERY,    "greenery")\
X(SAND,        "sand")\
X(SUB_NUT,     "nut")\
X(ROSE,        "rose")\
X(CLAY,        "clay")\
X(AIR,         "air")\
X(PAPER,       "paper")\
X(GOLD,        "gold")\
X(BONE,        "bone")\
X(STEEL,       "steel")\
X(ADAMANTINE,  "adamantine")\
X(FIRE,        "fire")\
X(COAL,        "coal")\
X(EXPLOSIVE,   "explosive")\
X(ACID,        "acid")\
X(SUB_CLOUD,   "cloud")\
X(SUB_DUST,    "dust")\
X(SUB_PLASTIC, "plastic")\
/// [List of subs]

#define X(column1, column2) column1,
enum kinds {
    KIND_TABLE
    LAST_KIND, ///< Nothing is LAST_KIND.
    KIND_COUNT = LAST_KIND
};

enum subs {
    SUB_TABLE
    LAST_SUB, ///< Nothing is made from LAST_SUB.
    SUB_COUNT = LAST_SUB
};
#undef X

enum usage_types {
    USAGE_TYPE_NO,
    USAGE_TYPE_OPEN,
    USAGE_TYPE_READ,
    USAGE_TYPE_READ_IN_INVENTORY,
    USAGE_TYPE_POUR,
    USAGE_TYPE_SET_FIRE,
    USAGE_TYPE_INNER
};

enum transparency {
    BLOCK_OPAQUE = 0,
    BLOCK_TRANSPARENT,
    INVISIBLE,
    NONSTANDARD = 6,
    UNDEF // temporary, doesn't appear in world.
};

/// For positive numbers only.
inline int Round(const float x) { return int(x + 0.5f); }

inline unsigned Abs(const int x) {
    const unsigned mask = x >> (sizeof(unsigned)*8 - 1);
    return (x ^ mask) - mask;
}

#define sizeof_array(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

#endif // HEADER_H
