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

#include <QDataStream>
#include "BlockManager.h"

#include "blocks/blocks.h"
#include "blocks/Dwarf.h"
#include "blocks/Bucket.h"
#include "blocks/Weapons.h"
#include "blocks/Illuminator.h"
#include "blocks/Containers.h"
#include "blocks/RainMachine.h"

#define sizeof_array(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

const QByteArray BlockManager::kinds[] = { // do not usp space, use '_'
    "block",
    "bell",
    "container",
    "intellectual",
    "animal",
    "pick",
    "telegraph",
    "liquid",
    "grass",
    "bush",
    "rabbit",
    "falling",
    "clock",
    "plate",
    "workbench",
    "weapon",
    "ladder",
    "door",
    "creator",
    "text",
    "map",
    "predator",
    "bucket",
    "shovel",
    "axe",
    "hammer",
    "illuminator",
    "rain_machine",
    "converter",
};

const QByteArray BlockManager::subs[] = { // do not usp space, use '_'
    "stone",
    "moss stone",
    "nullstone",
    "sky",
    "star",
    "sun_or_moon",
    "soil",
    "meat_of_intellectual",
    "animal_meat",
    "glass",
    "wood",
    "different",
    "iron",
    "water",
    "greenery",
    "sand",
    "hazelnut",
    "rose",
    "clay",
    "air",
    "paper",
    "gold",
    "bone",
    "steel",
    "adamantine",
    "fire",
    "coal",
    "explosive",
    "acid",
    "cloud",
    "dust",
};

BlockManager block_manager;

BlockManager::BlockManager() {
    for (int sub=0; sub<LAST_SUB; ++sub) {
        normals[sub] = new Block(BLOCK, sub);
    }
    static_assert((sizeof_array(BlockManager::kinds) == LAST_KIND),
        "invalid number of strings in BlockManager::kinds[]");
    static_assert((sizeof_array(BlockManager::subs) == LAST_SUB),
        "invalid number of strings in BlockManager::subs[]");
    static_assert((LAST_SUB  <= 128), "too many substances, should be < 127.");
    static_assert((LAST_KIND <= 256), "too many kinds, should be < 255.");
}

BlockManager::~BlockManager() {
    for(int sub=0; sub<LAST_SUB; ++sub) {
        delete normals[sub];
    }
}

Block * BlockManager::Normal(const int sub) const { return normals[sub]; }

Block * BlockManager::NewBlock(const int kind, const int sub) {
    switch ( static_cast<enum kinds>(kind) ) {
    // valid kinds:
    case BLOCK:  return new Block (kind, sub);
    case LIQUID: return new Liquid(kind, sub);
    case GRASS:  return new Grass (kind, sub);
    case FALLING: return new Falling(kind, sub);
    case CONTAINER: return new Container(kind, sub);
    case CONVERTER: return new Converter(kind, sub);
    case PLATE:  return new Plate (kind, sub);
    case LADDER: return new Ladder(kind, sub);
    case WEAPON: return new Weapon(kind, sub);
    case BUSH:   return new Bush  (kind, sub);
    case DWARF:  return new Dwarf (kind, sub);
    case RABBIT: return new Rabbit(kind, sub);
    case PREDATOR: return new Predator(kind, sub);
    case DOOR:   return new Door  (kind, sub);
    case CLOCK:  return new Clock (kind, sub);
    case BELL:   return new Bell  (kind, sub);
    case TEXT:   return new Text  (kind, sub);
    case MAP:    return new Map   (kind, sub);
    case BUCKET: return new Bucket(kind, sub);
    case PICK:   return new Pick  (kind, sub);
    case SHOVEL: return new Shovel(kind, sub);
    case HAMMER: return new Hammer(kind, sub);
    case AXE:    return new Axe   (kind, sub);
    case CREATOR: return new Creator(kind, sub);
    case WORKBENCH: return new Workbench(kind, sub);
    case ILLUMINATOR: return new Illuminator(kind, sub);
    case RAIN_MACHINE: return new RainMachine(kind, sub);
    // invalid kinds:
    case ANIMAL:
    case TELEGRAPH:
    case LAST_KIND:
        fprintf(stderr, "%s: kind ?: %d.\n", Q_FUNC_INFO, kind);
        return new Block(kind, sub);
    }
    Q_UNREACHABLE();
    return nullptr;
} // Block * BlockManager::NewBlock(int kind, int sub)

Block * BlockManager::BlockFromFile(QDataStream & str,
        const int kind, const int sub)
{
    switch ( static_cast<enum kinds>(kind) ) {
    // valid kinds:
    case BLOCK:  return new Block (str, kind, sub);
    case LIQUID: return new Liquid(str, kind, sub);
    case GRASS:  return new Grass (str, kind, sub);
    case CONTAINER: return new Container(str, kind, sub);
    case CONVERTER: return new Converter(str, kind, sub);
    case FALLING: return new Falling(str, kind, sub);
    case PLATE:  return new Plate (str, kind, sub);
    case LADDER: return new Ladder(str, kind, sub);
    case WEAPON: return new Weapon(str, kind, sub);
    case BUSH:   return new Bush  (str, kind, sub);
    case RABBIT: return new Rabbit(str, kind, sub);
    case PREDATOR: return new Predator(str, kind, sub);
    case DWARF:  return new Dwarf (str, kind, sub);
    case BELL:   return new Bell  (str, kind, sub);
    case TEXT:   return new Text  (str, kind, sub);
    case MAP:    return new Map   (str, kind, sub);
    case BUCKET: return new Bucket(str, kind, sub);
    case PICK:   return new Pick  (str, kind, sub);
    case SHOVEL: return new Shovel(str, kind, sub);
    case HAMMER: return new Hammer(str, kind, sub);
    case AXE:    return new Axe   (str, kind, sub);
    case DOOR:   return new Door  (str, kind, sub);
    case CLOCK:  return new Clock (str, kind, sub);
    case CREATOR: return new Creator(str, kind, sub);
    case WORKBENCH: return new Workbench(str, kind, sub);
    case ILLUMINATOR: return new Illuminator(str, kind, sub);
    case RAIN_MACHINE: return new RainMachine(str, kind, sub);
    // invalid kinds:
    case ANIMAL:
    case TELEGRAPH:
    case LAST_KIND:
        fprintf(stderr, "%s: kind ?: %d.\n", Q_FUNC_INFO, kind);
        return new Block(str, kind, sub);
    }
    Q_UNREACHABLE();
    return nullptr;
}

bool BlockManager::KindSubFromFile(QDataStream & str, int * kind, int * sub) {
    quint8 data;
    str >> data;
    *sub = (data & 0x7F);
    if ( data & 0x80 ) { // normal bit
        return true;
    } else {
        str >> data; // read quint8, not int
        *kind = data;
        return false;
    }
}

void BlockManager::DeleteBlock(Block * const block) const {
    if ( block != Normal(block->Sub()) ) {
        delete block;
    }
}

QString BlockManager::KindToString(const int kind) { return kinds[kind]; }
QString BlockManager:: SubToString(const int sub ) { return  subs[sub ]; }

int BlockManager::StringToKind(const QString str) {
    int i = 0;
    for ( ; i<LAST_KIND && kinds[i]!=str; ++i);
    return i;
}

int BlockManager::StringToSub(const QString str) {
    int i = 0;
    for ( ; i<LAST_SUB && subs[i]!=str; ++i);
    return i;
}

Block * BlockManager::ReplaceWithNormal(Block * const block) const {
    Block * const normal = Normal(block->Sub());
    if ( block!=normal && *block==*normal ) {
        delete block;
        return normal;
    } else {
        return block;
    }
}

int BlockManager::KindFromId(const int id) { return (id >> 8); }
int BlockManager:: SubFromId(const int id) { return (id & 0xFF); }
