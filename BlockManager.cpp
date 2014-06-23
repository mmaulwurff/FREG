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
#include "blocks/Container.h"

#define sizeof_array(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

const QString BlockManager::kinds[] = {
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
    "locked door",
    "creator",
    "text",
    "map",
    "predator",
    "bucket",
    "shovel",
    "axe",
    "hammer",
    "illuminator",
};

const QString BlockManager::subs[] = {
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
};

BlockManager block_manager;

BlockManager::BlockManager() {
    for (int sub=0; sub<LAST_SUB; ++sub) {
        normals[sub] = new Block(sub, MakeId(BLOCK, sub));
    }
    static_assert((sizeof_array(BlockManager::kinds) == LAST_KIND),
        "invalid number of strings in BlockManager::kinds[]");
    static_assert((sizeof_array(BlockManager::subs) == LAST_SUB),
        "invalid number of strings in BlockManager::subs[]");
    static_assert((LAST_SUB <= 128), "too many substances, should be < 127.");
}

BlockManager::~BlockManager() {
    for(int sub=0; sub<LAST_SUB; ++sub) {
        delete normals[sub];
    }
}

Block * BlockManager::NormalBlock(const int sub) const { return normals[sub]; }

Block * BlockManager::NewBlock(const int kind, const int sub) {
    const int id = MakeId(kind, sub);
    switch ( static_cast<enum kinds>(kind) ) {
    // valid kinds:
    case BLOCK:  return new Block (sub, id);
    case LIQUID: return new Liquid(sub, id);
    case GRASS:  return new Grass (sub, id);
    case FALLING: return new Falling(sub, id);
    case CONTAINER: return new Container(sub, id);
    case PLATE:  return new Plate (sub, id);
    case LADDER: return new Ladder(sub, id);
    case WEAPON: return new Weapon(sub, id);
    case BUSH:   return new Bush  (sub, id);
    case DWARF:  return new Dwarf (sub, id);
    case RABBIT: return new Rabbit(sub, id);
    case PREDATOR: return new Predator(sub, id);
    case LOCKED_DOOR:
    case DOOR:   return new Door  (sub, id);
    case CLOCK:  return new Clock (sub, id);
    case BELL:   return new Bell  (sub, id);
    case TEXT:   return new Text  (sub, id);
    case MAP:    return new Map   (sub, id);
    case BUCKET: return new Bucket(sub, id);
    case PICK:   return new Pick  (sub, id);
    case SHOVEL: return new Shovel(sub, id);
    case HAMMER: return new Hammer(sub, id);
    case AXE:    return new Axe   (sub, id);
    case CREATOR: return new Creator(sub, id);
    case WORKBENCH: return new Workbench(sub, id);
    case ILLUMINATOR: return new Illuminator(sub, id);
    // invalid kinds:
    case ANIMAL:
    case TELEGRAPH:
    case LAST_KIND:
        fprintf(stderr, "BlockManager::NewBlock: kind ?: %d.\n", kind);
        return new Block(sub, id);
    }
    return nullptr; // should never be returned, everything is in switch.
} // Block * BlockManager::NewBlock(int kind, int sub)

Block * BlockManager::BlockFromFile(QDataStream & str,
        const int kind, const int sub)
{
    const int id = MakeId(kind, sub);
    switch ( static_cast<enum kinds>(kind) ) {
    // valid kinds:
    case BLOCK:  return new Block (str, sub, id);
    case LIQUID: return new Liquid(str, sub, id);
    case GRASS:  return new Grass (str, sub, id);
    case CONTAINER: return new Container(str, sub, id);
    case FALLING: return new Falling(str, sub, id);
    case PLATE:  return new Plate (str, sub, id);
    case LADDER: return new Ladder(str, sub, id);
    case WEAPON: return new Weapon(str, sub, id);
    case BUSH:   return new Bush  (str, sub, id);
    case RABBIT: return new Rabbit(str, sub, id);
    case PREDATOR: return new Predator(str, sub, id);
    case DWARF:  return new Dwarf (str, sub, id);
    case BELL:   return new Bell  (str, sub, id);
    case TEXT:   return new Text  (str, sub, id);
    case MAP:    return new Map   (str, sub, id);
    case BUCKET: return new Bucket(str, sub, id);
    case PICK:   return new Pick  (str, sub, id);
    case SHOVEL: return new Shovel(str, sub, id);
    case HAMMER: return new Hammer(str, sub, id);
    case AXE:    return new Axe   (str, sub, id);
    case LOCKED_DOOR:
    case DOOR:   return new Door  (str, sub, id);
    case CLOCK:  return new Clock (str, sub, id);
    case CREATOR: return new Creator(str, sub, id);
    case WORKBENCH: return new Workbench(str, sub, id);
    case ILLUMINATOR: return new Illuminator(str, sub, id);
    // invalid kinds:
    case ANIMAL:
    case TELEGRAPH:
    case LAST_KIND:
        fprintf(stderr, "BlockManager::BlockFromFile: kind ?: %d.\n", kind);
        return new Block(str, sub, id);
    }
    return nullptr; // should never be returned, everything is in switch.
}

Block * BlockManager::BlockFromFile(QDataStream & str) const {
    int kind, sub;
    return KindSubFromFile(str, &kind, &sub) ?
        NormalBlock(sub) : BlockFromFile(str, kind, sub);
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

void BlockManager::DeleteBlock(const Block * const block) const {
    if ( block != NormalBlock(block->Sub()) ) {
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
    Block * const normal = NormalBlock(block->Sub());
    if ( block!=normal && *block==*normal ) {
        delete block;
        return normal;
    } else {
        return block;
    }
}

int BlockManager::MakeId(const int kind, const int sub) {
    return (kind << 8) | sub;
}

int BlockManager::KindFromId(const int id) { return (id >> 8); }
int BlockManager:: SubFromId(const int id) { return (id & 0xFF); }
