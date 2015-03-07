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

#define M_DISABLE_COPY(Class) \
Class(const Class &) = delete; \
Class &operator=(const Class &) = delete;

#ifdef QT_NO_DEBUG
const bool DEBUG = false;
#else
const bool DEBUG = true;
#endif

#define ALL(container) std::begin(container), std::end(container)
#define const_int(x, y, z) const int x, const int y, const int z

#define QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_TO_ASCII

#define Str(str) QStringLiteral(str)

extern const class QString home_path;

enum sizes {
    SHRED_WIDTH_BITSHIFT = 4,
    SHRED_WIDTH = 16,
    HEIGHT = 256,
    MAX_NOTE_LENGTH = 144,
};

#define X_COLOR X_CHAR
#define X_STRING(string, ...) QStringLiteral(string),
#define X_ENUM(  column1, enum_element, ...) enum_element,
#define X_CHAR(  column1, column2, character, ...) character,
#define X_CLASS( column1, column2, column3, class, ...) class,

#define X_ENUM_INIT(column1, enum_element, init, ...) enum_element = init,

// X(QT_TRANSLATE_NOOP(context, translatable shred name), enum element, symbol)
#define SHRED_TABLE(X) \
X(QT_TRANSLATE_NOOP("Shred", "Plain"      ), SHRED_PLAIN,        '.', )\
X(QT_TRANSLATE_NOOP("Shred", "Test shred" ), SHRED_TESTSHRED,    'T', )\
X(QT_TRANSLATE_NOOP("Shred", "Pyramid"    ), SHRED_PYRAMID,      'P', )\
X(QT_TRANSLATE_NOOP("Shred", "Hill"       ), SHRED_HILL,         '+', )\
X(QT_TRANSLATE_NOOP("Shred", "Desert"     ), SHRED_DESERT,       ':', )\
X(QT_TRANSLATE_NOOP("Shred", "Water"      ), SHRED_WATER,        '~', )\
X(QT_TRANSLATE_NOOP("Shred", "Forest"     ), SHRED_FOREST,       '%', )\
X(QT_TRANSLATE_NOOP("Shred", "Mountain"   ), SHRED_MOUNTAIN,     '^', )\
X(QT_TRANSLATE_NOOP("Shred", "Empty"      ), SHRED_EMPTY,        '_', )\
X(QT_TRANSLATE_NOOP("Shred", "Chaos"      ), SHRED_CHAOS,        '!', )\
X(QT_TRANSLATE_NOOP("Shred", "Castle"     ), SHRED_CASTLE,       'C', )\
X(QT_TRANSLATE_NOOP("Shred", "Waste"      ), SHRED_WASTE,        '=', )\
X(QT_TRANSLATE_NOOP("Shred", "Acid lake"  ), SHRED_ACID_LAKE,    'a', )\
X(QT_TRANSLATE_NOOP("Shred", "Lava lake"  ), SHRED_LAVA_LAKE,    'l', )\
X(QT_TRANSLATE_NOOP("Shred", "Crater"     ), SHRED_CRATER,       'c', )\
X(QT_TRANSLATE_NOOP("Shred", "Dead forest"), SHRED_DEAD_FOREST,  'f', )\
X(QT_TRANSLATE_NOOP("Shred", "Dead hill"  ), SHRED_DEAD_HILL,    '*', )\
X(QT_TRANSLATE_NOOP("Shred", "0mountain"  ), SHRED_NULLMOUNTAIN, '#', )\
X(QT_TRANSLATE_NOOP("Shred", "Flat"       ), SHRED_FLAT,         'F', )\
X(QT_TRANSLATE_NOOP("Shred", "Underground"), SHRED_UNDERGROUND,  '-', )\

enum shred_type {
    SHRED_TABLE(X_ENUM_INIT)
    SHRED_OUT_BORDER = SHRED_WATER
};

enum dirs {
    ANYWHERE = 0,
    UP = 0, ///< 0
    DOWN,   ///< 1
    NORTH,  ///< 2
    EAST,   ///< 3
    SOUTH,  ///< 4
    WEST,   ///< 5
    LAST_DIR = WEST
};

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
 *  \snippet header.h List of kinds */
/// [List of kinds]
/// (context, translatable class name), enum element, character, class)
#define KIND_TABLE(X) \
X(QT_TRANSLATE_NOOP("Block", "Block"       ), BLOCK,       '#',  Block,      )\
X(QT_TRANSLATE_NOOP("Block", "Bell"        ), BELL,        'B',  Bell,       )\
X(QT_TRANSLATE_NOOP("Block", "Chest"       ), CONTAINER,   '&',  Container,  )\
X(QT_TRANSLATE_NOOP("Block", "Rational"    ), DWARF,       '@',  Dwarf,      )\
X(QT_TRANSLATE_NOOP("Block", "Pick"        ), PICK,        '\\', Pick,       )\
X(QT_TRANSLATE_NOOP("Block", "Liquid"      ), LIQUID,      '~',  Liquid,     )\
X(QT_TRANSLATE_NOOP("Block", "Plant"       ), GRASS,       '.',  Grass,      )\
X(QT_TRANSLATE_NOOP("Block", "Bush"        ), BUSH,        ';',  Bush,       )\
X(QT_TRANSLATE_NOOP("Block", "Herbivore"   ), RABBIT,      'r',  Rabbit,     )\
X(QT_TRANSLATE_NOOP("Block", "Snow"        ), FALLING,     '.',  Falling,    )\
X(QT_TRANSLATE_NOOP("Block", "Clock"       ), CLOCK,       'C',  Clock,      )\
X(QT_TRANSLATE_NOOP("Block", "Plate"       ), PLATE,       '_',  Plate,      )\
X(QT_TRANSLATE_NOOP("Block", "Workbench"   ), WORKBENCH,   '*',  Workbench,  )\
X(QT_TRANSLATE_NOOP("Block", "Stick"       ), WEAPON,      '/',  Weapon,     )\
X(QT_TRANSLATE_NOOP("Block", "Ladder"      ), LADDER,      '^',  Ladder,     )\
X(QT_TRANSLATE_NOOP("Block", "Door"        ), DOOR,        '\'', Door,       )\
X(QT_TRANSLATE_NOOP("Block", "Box"         ), BOX,         '&',  Box,        )\
X(QT_TRANSLATE_NOOP("Block", "Sign"        ), KIND_TEXT,   '?',  Text,       )\
X(QT_TRANSLATE_NOOP("Block", "Map"         ), MAP,         '?',  Map,        )\
X(QT_TRANSLATE_NOOP("Block", "Predator"    ), PREDATOR,    '!',  Predator,   )\
X(QT_TRANSLATE_NOOP("Block", "Bucket"      ), BUCKET,      'u',  Bucket,     )\
X(QT_TRANSLATE_NOOP("Block", "Shovel"      ), SHOVEL,      '|',  Shovel,     )\
X(QT_TRANSLATE_NOOP("Block", "Axe"         ), AXE,         '/',  Axe,        )\
X(QT_TRANSLATE_NOOP("Block", "Hammer"      ), HAMMER,      'T',  Hammer,     )\
X(QT_TRANSLATE_NOOP("Block", "Lantern"     ), ILLUMINATOR, 'i',  Illuminator,)\
X(QT_TRANSLATE_NOOP("Block", "RainMachine" ), RAIN_MACHINE,'R',  RainMachine,)\
X(QT_TRANSLATE_NOOP("Block", "Converter"   ), CONVERTER,   'V',  Converter,  )\
X(QT_TRANSLATE_NOOP("Block", "BodyArmour"  ), ARMOUR,      'A',  Armour,     )\
X(QT_TRANSLATE_NOOP("Block", "Helmet"      ), HELMET,      'H',  Helmet,     )\
X(QT_TRANSLATE_NOOP("Block", "Boots"       ), BOOTS,       'L',  Boots,      )\
X(QT_TRANSLATE_NOOP("Block", "Telegraph"   ), TELEGRAPH,   't',  Telegraph,  )\
X(QT_TRANSLATE_NOOP("Block", "Medkit"      ), MEDKIT,      '+',  MedKit,     )\
X(QT_TRANSLATE_NOOP("Block", "Filter"      ), FILTER,      'F',  Filter,     )\
X(QT_TRANSLATE_NOOP("Block", "Informer"    ), INFORMER,    'I',  Informer,   )\
X(QT_TRANSLATE_NOOP("Block", "Teleport"    ), TELEPORT,    '0',  Teleport,   )\
X(QT_TRANSLATE_NOOP("Block", "Accumulator" ), ACCUMULATOR, '=',  Accumulator,)\
/// [List of kinds]

/** \page subs List of available substances
 *  Complete list.
 *  These substances can be used as parameters to `get KIND SUB` command.
 *  Don't make blocks (BLOCK kind) from SKY, they are special for
 *  shred loading and saving.
 *  Don't make non-BLOCK blocks from air, otherwise memory leaks are possible.
 *  Don't change order, this will break save file compatibility.
 *  Add new substances to bottom.
 *  \snippet header.h List of subs */
/// [List of subs]
/// X(QT_TRANSLATE_NOOP(context, translatable name), enum element, color)
#define SUB_TABLE(X) \
X(QT_TRANSLATE_NOOP("Block", "stone"     ), STONE,       BLACK_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "mossStone" ), MOSS_STONE,  GREEN_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "0stone"    ), NULLSTONE, MAGENTA_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "0stone"    ), SKY,       MAGENTA_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "star"      ), STAR,        WHITE_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "diamond"   ), DIAMOND,      CYAN_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "soil"      ), SOIL,        BLACK_YELLOW, )\
X(QT_TRANSLATE_NOOP("Block", "meat"      ), H_MEAT,      WHITE_RED,    )\
X(QT_TRANSLATE_NOOP("Block", "meat"      ), A_MEAT,      BLACK_RED,    )\
X(QT_TRANSLATE_NOOP("Block", "glass"     ), GLASS,        BLUE_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "wood"      ), WOOD,        BLACK_RED,    )\
X(QT_TRANSLATE_NOOP("Block", "different" ), DIFFERENT,   WHITE_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "iron"      ), IRON,        WHITE_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "water"     ), WATER,       WHITE_CYAN,   )\
X(QT_TRANSLATE_NOOP("Block", "greenery"  ), GREENERY,    BLACK_GREEN,  )\
X(QT_TRANSLATE_NOOP("Block", "sand"      ), SAND,        WHITE_YELLOW, )\
X(QT_TRANSLATE_NOOP("Block", "nut"       ), SUB_NUT,     BLACK_YELLOW, )\
X(QT_TRANSLATE_NOOP("Block", "rose"      ), ROSE,          RED_GREEN,  )\
X(QT_TRANSLATE_NOOP("Block", "clay"      ), CLAY,        WHITE_RED,    )\
X(QT_TRANSLATE_NOOP("Block", "air"       ), AIR,         WHITE_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "paper"     ), PAPER,     MAGENTA_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "gold"      ), GOLD,        WHITE_YELLOW, )\
X(QT_TRANSLATE_NOOP("Block", "bone"      ), BONE,      MAGENTA_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "steel"     ), STEEL,       WHITE_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "adamantine"), ADAMANTINE,   CYAN_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "fire"      ), FIRE,          RED_YELLOW, )\
X(QT_TRANSLATE_NOOP("Block", "coal"      ), COAL,        BLACK_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "explosive" ), EXPLOSIVE,   WHITE_RED,    )\
X(QT_TRANSLATE_NOOP("Block", "acid"      ), ACID,        GREEN_GREEN,  )\
X(QT_TRANSLATE_NOOP("Block", "cloud"     ), SUB_CLOUD,   BLACK_WHITE,  )\
X(QT_TRANSLATE_NOOP("Block", "dust"      ), SUB_DUST,    BLACK_BLACK,  )\
X(QT_TRANSLATE_NOOP("Block", "plastic"   ), SUB_PLASTIC, GREEN_BLACK,  )\
/// [List of subs]

enum kinds {
    KIND_TABLE(X_ENUM)
    LAST_KIND, ///< Nothing is LAST_KIND.
    KIND_COUNT = LAST_KIND
};

enum subs {
    SUB_TABLE(X_ENUM)
    LAST_SUB, ///< Nothing is made from LAST_SUB.
    SUB_COUNT = LAST_SUB
};

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
    BLOCK_OPAQUE,
    BLOCK_TRANSPARENT,
    INVISIBLE,
};

#endif // HEADER_H
