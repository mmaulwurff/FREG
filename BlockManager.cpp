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
#include "blocks/Armour.h"
#include "blocks/Filter.h"
#include "blocks/Teleport.h"
#include "blocks/Accumulator.h"

/** \page kinds List of available kinds
 *  Complete list.
 *  These kinds can be used as parameters to `get KIND SUB` command.
 *  \snippet BlockManager.cpp List of kinds */
const QByteArray BlockManager::kinds[] = { // do not use space, use '_'
    /// [List of kinds]
    "block",
    "bell",
    "container",
    "intellectual",
    "pick",
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
    "box",
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
    "armour",
    "helmet",
    "boots",
    "telegraph",
    "medkit",
    "filter",
    "informer",
    "teleport",
    "accumulator"
    /// [List of kinds]
};

/** \page subs List of available substances
 *  Complete list.
 *  These substances can be used as parameters to `get KIND SUB` command.
 *  \snippet BlockManager.cpp List of subs */
const QByteArray BlockManager::subs[] = { // do not usp space, use '_'
    /// [List of subs]
    "stone",
    "moss stone",
    "nullstone",
    "sky",
    "star",
    "diamond",
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
    "nut",
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
    "plastic",
    /// [List of subs]
};

const BlockManager * block_manager;

BlockManager::BlockManager() {
    for (int sub=0; sub<SUB_COUNT; ++sub) {
        normals[sub] = new Block(BLOCK, sub);
    }
    static_assert((sizeof_array(BlockManager::kinds) == KIND_COUNT),
        "Invalid number of strings in BlockManager::kinds[].");
    static_assert((sizeof_array(BlockManager::subs)  == SUB_COUNT),
        "Invalid number of strings in BlockManager::subs[].");
    static_assert((SUB_COUNT  <= 64 ), "too many substances, should be < 64.");
    static_assert((KIND_COUNT <= 128), "too many kinds, should be < 128.");
    /*int sum = 0;
    for (int kind = 0; kind<LAST_KIND; ++kind)
    for (int sub  = 0; sub <LAST_SUB;  ++sub) {
        sum += IsValid(kind, sub);
    }
    qDebug() << "valid pairs:" << sum;*/
}

BlockManager::~BlockManager() {
    for(int sub=0; sub<SUB_COUNT; ++sub) {
        delete normals[sub];
    }
}

Block * BlockManager::NewBlock(const int kind, const int sub) {
    //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind, sub));
    switch ( static_cast<enum kinds>(kind) ) {
    case BLOCK:     return new Block (kind, sub);
    case BELL:      return new Bell  (kind, sub);
    case CONTAINER: return new Container(kind, sub);
    case DWARF:     return new Dwarf (kind, sub);
    case PICK:      return new Pick  (kind, sub);
    case LIQUID:    return new Liquid(kind, sub);
    case GRASS:     return new Grass (kind, sub);
    case BUSH:      return new Bush  (kind, sub);
    case RABBIT:    return new Rabbit(kind, sub);
    case FALLING:   return new Falling(kind, sub);
    case CLOCK:     return new Clock (kind, sub);
    case PLATE:     return new Plate (kind, sub);
    case WORKBENCH: return new Workbench(kind, sub);
    case WEAPON:    return new Weapon(kind, sub);
    case LADDER:    return new Ladder(kind, sub);
    case DOOR:      return new Door  (kind, sub);
    case BOX:       return new Box   (kind, sub);
    case KIND_TEXT: return new Text  (kind, sub);
    case MAP:       return new Map   (kind, sub);
    case PREDATOR:  return new Predator(kind, sub);
    case BUCKET:    return new Bucket(kind, sub);
    case SHOVEL:    return new Shovel(kind, sub);
    case AXE:       return new Axe   (kind, sub);
    case HAMMER:    return new Hammer(kind, sub);
    case ILLUMINATOR:  return new Illuminator(kind, sub);
    case RAIN_MACHINE: return new RainMachine(kind, sub);
    case CONVERTER: return new Converter(kind, sub);
    case ARMOUR:    return new Armour(kind, sub);
    case HELMET:    return new Helmet(kind, sub);
    case BOOTS:     return new Boots (kind, sub);
    case TELEGRAPH: return new Telegraph(kind, sub);
    case MEDKIT:    return new MedKit(kind, sub);
    case FILTER:    return new Filter(kind, sub);
    case INFORMER:  return new Informer(kind, sub);
    case TELEPORT:  return new Teleport(kind, sub);
    case ACCUMULATOR:  return new Accumulator(kind, sub);
    case LAST_KIND: break;
    }
    Q_UNREACHABLE();
    return nullptr;
} // Block * BlockManager::NewBlock(int kind, int sub)

Block * BlockManager::BlockFromFile(QDataStream & str,
        const int kind, const int sub)
{
    //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind, sub));
    switch ( static_cast<enum kinds>(kind) ) {
    case BLOCK:     return new Block (str, kind, sub);
    case BELL:      return new Bell  (str, kind, sub);
    case CONTAINER: return new Container(str, kind, sub);
    case DWARF:     return new Dwarf (str, kind, sub);
    case PICK:      return new Pick  (str, kind, sub);
    case LIQUID:    return new Liquid(str, kind, sub);
    case GRASS:     return new Grass (str, kind, sub);
    case BUSH:      return new Bush  (str, kind, sub);
    case RABBIT:    return new Rabbit(str, kind, sub);
    case FALLING:   return new Falling(str, kind, sub);
    case CLOCK:     return new Clock (str, kind, sub);
    case PLATE:     return new Plate (str, kind, sub);
    case WORKBENCH: return new Workbench(str, kind, sub);
    case WEAPON:    return new Weapon(str, kind, sub);
    case LADDER:    return new Ladder(str, kind, sub);
    case DOOR:      return new Door  (str, kind, sub);
    case BOX:       return new Box   (str, kind, sub);
    case KIND_TEXT: return new Text  (str, kind, sub);
    case MAP:       return new Map   (str, kind, sub);
    case PREDATOR:  return new Predator(str, kind, sub);
    case BUCKET:    return new Bucket(str, kind, sub);
    case SHOVEL:    return new Shovel(str, kind, sub);
    case AXE:       return new Axe   (str, kind, sub);
    case HAMMER:    return new Hammer(str, kind, sub);
    case ILLUMINATOR:  return new Illuminator(str, kind, sub);
    case RAIN_MACHINE: return new RainMachine(str, kind, sub);
    case CONVERTER: return new Converter(str, kind, sub);
    case ARMOUR:    return new Armour(str, kind, sub);
    case HELMET:    return new Helmet(str, kind, sub);
    case BOOTS:     return new Boots (str, kind, sub);
    case TELEGRAPH: return new Telegraph(str, kind, sub);
    case MEDKIT:    return new MedKit(str, kind, sub);
    case FILTER:    return new Filter(str, kind, sub);
    case INFORMER:  return new Informer(str, kind, sub);
    case TELEPORT:  return new Teleport(str, kind, sub);
    case ACCUMULATOR:  return new Accumulator(str, kind, sub);
    case LAST_KIND: break;
    }
    Q_UNREACHABLE();
    return nullptr;
}

bool BlockManager::KindSubFromFile(QDataStream & str,
        quint8 * kind, quint8 * sub)
{
    str >> *sub;
    if ( *sub & 0x80 ) { // normal bit
        *sub &= 0x7F;
        return true;
    } else {
        *sub &= 0x7F;
        str >> *kind;
        return false;
    }
}

void BlockManager::DeleteBlock(Block * const block) const {
    if ( block != Normal(block->Sub()) ) {
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
        delete block;
    }
}

QString BlockManager::KindToString(const int kind) { return kinds[kind]; }
QString BlockManager:: SubToString(const int sub ) { return  subs[sub ]; }

int BlockManager::StringToKind(const QString str) {
    int i = 0;
    for ( ; i<KIND_COUNT && kinds[i]!=str; ++i);
    return i;
}

int BlockManager::StringToSub(const QString str) {
    int i = 0;
    for ( ; i<SUB_COUNT && subs[i]!=str; ++i);
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

bool BlockManager::IsValid(const int kind, const int sub) {
    const sub_groups group = Block::GetSubGroup(sub);
    switch ( static_cast<enum kinds>(kind) ) {
    case BLOCK:     return true;
    case LAST_KIND: break;
    case DWARF:     return ( sub == DIFFERENT || sub == H_MEAT
                            || group == GROUP_METAL );
    case GRASS:     return ( sub == GREENERY || sub == FIRE );
    case BUSH:      return ( sub == GREENERY || sub == WOOD );
    case FALLING:   return ( sub == WATER || sub == STONE || sub == SUB_DUST );
    case LIQUID:    return ( group == GROUP_MEAT || group == GROUP_METAL
                            || sub == STONE || sub == WATER );
    case ACCUMULATOR: return ( sub == GLASS || GROUP_METAL == group);

    case BUCKET:
    case CLOCK:
    case RAIN_MACHINE:
    case ARMOUR:
    case HELMET:
    case BOOTS:
    case TELEGRAPH:
    case MEDKIT:
    case FILTER:
    case INFORMER:
    case BELL:      return ( group == GROUP_METAL );

    case BOX:
    case DOOR:
    case LADDER:
    case PLATE:
    case WEAPON:
    case PICK:
    case SHOVEL:
    case AXE:
    case HAMMER:
    case ILLUMINATOR:
    case WORKBENCH:
    case TELEPORT:
    case CONVERTER: return ( group == GROUP_METAL || group == GROUP_HANDY );
    case CONTAINER: return ( group == GROUP_METAL ||
                             group == GROUP_HANDY ||
                             group == GROUP_MEAT );

    case PREDATOR:
    case RABBIT:    return ( sub == A_MEAT );

    case MAP:
    case KIND_TEXT: return ( sub == PAPER || sub == GLASS );
    }
    Q_UNREACHABLE();
    return false;
}
