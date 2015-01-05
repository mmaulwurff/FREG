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

#define SHRED_TABLE \
X(QT_TRANSLATE_NOOP("Shred", "Plain"      ), SHRED_PLAIN,        '.' )\
X(QT_TRANSLATE_NOOP("Shred", "Test shred" ), SHRED_TESTSHRED,    'T' )\
X(QT_TRANSLATE_NOOP("Shred", "Pyramid"    ), SHRED_PYRAMID,      'P' )\
X(QT_TRANSLATE_NOOP("Shred", "Hill"       ), SHRED_HILL,         '+' )\
X(QT_TRANSLATE_NOOP("Shred", "Desert"     ), SHRED_DESERT,       ':' )\
X(QT_TRANSLATE_NOOP("Shred", "Water"      ), SHRED_WATER,        '~' )\
X(QT_TRANSLATE_NOOP("Shred", "Forest"     ), SHRED_FOREST,       '%' )\
X(QT_TRANSLATE_NOOP("Shred", "Mountain"   ), SHRED_MOUNTAIN,     '^' )\
X(QT_TRANSLATE_NOOP("Shred", "Empty"      ), SHRED_EMPTY,        '_' )\
X(QT_TRANSLATE_NOOP("Shred", "Chaos"      ), SHRED_CHAOS,        '!' )\
X(QT_TRANSLATE_NOOP("Shred", "Castle"     ), SHRED_CASTLE,       'C' )\
X(QT_TRANSLATE_NOOP("Shred", "Waste"      ), SHRED_WASTE,        '=' )\
X(QT_TRANSLATE_NOOP("Shred", "Acid lake"  ), SHRED_ACID_LAKE,    'a' )\
X(QT_TRANSLATE_NOOP("Shred", "Lava lake"  ), SHRED_LAVA_LAKE,    'l' )\
X(QT_TRANSLATE_NOOP("Shred", "Crater"     ), SHRED_CRATER,       'c' )\
X(QT_TRANSLATE_NOOP("Shred", "Dead forest"), SHRED_DEAD_FOREST,  'f' )\
X(QT_TRANSLATE_NOOP("Shred", "Dead hill"  ), SHRED_DEAD_HILL,    '*' )\
X(QT_TRANSLATE_NOOP("Shred", "0mountain"  ), SHRED_NULLMOUNTAIN, '#' )\
X(QT_TRANSLATE_NOOP("Shred", "Underground"), SHRED_UNDERGROUND,  '-' )\

enum shred_type {
    #define X(column1, column2, column3) column2 = column3,
    SHRED_TABLE
    #undef X
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
X(QT_TRANSLATE_NOOP("Block", "Block"       ), BLOCK        )\
X(QT_TRANSLATE_NOOP("Block", "Bell"        ), BELL         )\
X(QT_TRANSLATE_NOOP("Block", "Chest"       ), CONTAINER    )\
X(QT_TRANSLATE_NOOP("Block", "Intellectual"), DWARF        )\
X(QT_TRANSLATE_NOOP("Block", "Pick"        ), PICK         )\
X(QT_TRANSLATE_NOOP("Block", "Liquid"      ), LIQUID       )\
X(QT_TRANSLATE_NOOP("Block", "Plant"       ), GRASS        )\
X(QT_TRANSLATE_NOOP("Block", "Bush"        ), BUSH         )\
X(QT_TRANSLATE_NOOP("Block", "Herbivore"   ), RABBIT       )\
X(QT_TRANSLATE_NOOP("Block", "Falling"     ), FALLING      )\
X(QT_TRANSLATE_NOOP("Block", "Clock"       ), CLOCK        )\
X(QT_TRANSLATE_NOOP("Block", "Plate"       ), PLATE        )\
X(QT_TRANSLATE_NOOP("Block", "Workbench"   ), WORKBENCH    )\
X(QT_TRANSLATE_NOOP("Block", "Stick"       ), WEAPON       )\
X(QT_TRANSLATE_NOOP("Block", "Ladder"      ), LADDER       )\
X(QT_TRANSLATE_NOOP("Block", "Door"        ), DOOR         )\
X(QT_TRANSLATE_NOOP("Block", "Box"         ), BOX          )\
X(QT_TRANSLATE_NOOP("Block", "Sign"        ), KIND_TEXT    )\
X(QT_TRANSLATE_NOOP("Block", "Map"         ), MAP          )\
X(QT_TRANSLATE_NOOP("Block", "Predator"    ), PREDATOR     )\
X(QT_TRANSLATE_NOOP("Block", "Bucket"      ), BUCKET       )\
X(QT_TRANSLATE_NOOP("Block", "Shovel"      ), SHOVEL       )\
X(QT_TRANSLATE_NOOP("Block", "Axe"         ), AXE          )\
X(QT_TRANSLATE_NOOP("Block", "Hammer"      ), HAMMER       )\
X(QT_TRANSLATE_NOOP("Block", "Illuminator" ), ILLUMINATOR  )\
X(QT_TRANSLATE_NOOP("Block", "RainMachine" ), RAIN_MACHINE )\
X(QT_TRANSLATE_NOOP("Block", "Converter"   ), CONVERTER    )\
X(QT_TRANSLATE_NOOP("Block", "BodyArmour"  ), ARMOUR       )\
X(QT_TRANSLATE_NOOP("Block", "Helmet"      ), HELMET       )\
X(QT_TRANSLATE_NOOP("Block", "Boots"       ), BOOTS        )\
X(QT_TRANSLATE_NOOP("Block", "Telegraph"   ), TELEGRAPH    )\
X(QT_TRANSLATE_NOOP("Block", "Medkit"      ), MEDKIT       )\
X(QT_TRANSLATE_NOOP("Block", "Filter"      ), FILTER       )\
X(QT_TRANSLATE_NOOP("Block", "Informer"    ), INFORMER     )\
X(QT_TRANSLATE_NOOP("Block", "Teleport"    ), TELEPORT     )\
X(QT_TRANSLATE_NOOP("Block", "Accumulator" ), ACCUMULATOR  )\
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
 *  \snippet header.h List of subs */
/// [List of subs]
#define SUB_TABLE \
X(QT_TRANSLATE_NOOP("Block", "stone"     ), STONE       )\
X(QT_TRANSLATE_NOOP("Block", "mossStone" ), MOSS_STONE  )\
X(QT_TRANSLATE_NOOP("Block", "0stone"    ), NULLSTONE   )\
X(QT_TRANSLATE_NOOP("Block", "air"       ), SKY         )\
X(QT_TRANSLATE_NOOP("Block", "air"       ), STAR        )\
X(QT_TRANSLATE_NOOP("Block", "diamond"   ), DIAMOND     )\
X(QT_TRANSLATE_NOOP("Block", "soil"      ), SOIL        )\
X(QT_TRANSLATE_NOOP("Block", "meat"      ), H_MEAT      )\
X(QT_TRANSLATE_NOOP("Block", "meat"      ), A_MEAT      )\
X(QT_TRANSLATE_NOOP("Block", "glass"     ), GLASS       )\
X(QT_TRANSLATE_NOOP("Block", "wood"      ), WOOD        )\
X(QT_TRANSLATE_NOOP("Block", "different" ), DIFFERENT   )\
X(QT_TRANSLATE_NOOP("Block", "iron"      ), IRON        )\
X(QT_TRANSLATE_NOOP("Block", "water"     ), WATER       )\
X(QT_TRANSLATE_NOOP("Block", "greenery"  ), GREENERY    )\
X(QT_TRANSLATE_NOOP("Block", "sand"      ), SAND        )\
X(QT_TRANSLATE_NOOP("Block", "nut"       ), SUB_NUT     )\
X(QT_TRANSLATE_NOOP("Block", "rose"      ), ROSE        )\
X(QT_TRANSLATE_NOOP("Block", "clay"      ), CLAY        )\
X(QT_TRANSLATE_NOOP("Block", "air"       ), AIR         )\
X(QT_TRANSLATE_NOOP("Block", "paper"     ), PAPER       )\
X(QT_TRANSLATE_NOOP("Block", "gold"      ), GOLD        )\
X(QT_TRANSLATE_NOOP("Block", "bone"      ), BONE        )\
X(QT_TRANSLATE_NOOP("Block", "steel"     ), STEEL       )\
X(QT_TRANSLATE_NOOP("Block", "adamantine"), ADAMANTINE  )\
X(QT_TRANSLATE_NOOP("Block", "fire"      ), FIRE        )\
X(QT_TRANSLATE_NOOP("Block", "coal"      ), COAL        )\
X(QT_TRANSLATE_NOOP("Block", "explosive" ), EXPLOSIVE   )\
X(QT_TRANSLATE_NOOP("Block", "acid"      ), ACID        )\
X(QT_TRANSLATE_NOOP("Block", "cloud"     ), SUB_CLOUD   )\
X(QT_TRANSLATE_NOOP("Block", "dust"      ), SUB_DUST    )\
X(QT_TRANSLATE_NOOP("Block", "plastic"   ), SUB_PLASTIC )\
/// [List of subs]

#define X(column1, column2) column2,
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
    UNDEF ///< temporary, doesn't appear in world.
};

/// For positive numbers only.
inline int Round(const float x) { return int(x + 0.5f); }

inline unsigned Abs(const int x) {
    const unsigned mask = x >> (sizeof(unsigned)*8 - 1);
    return (x ^ mask) - mask;
}

#endif // HEADER_H
