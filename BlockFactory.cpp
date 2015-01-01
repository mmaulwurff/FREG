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

#include "BlockFactory.h"
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
#include <QDataStream>

/** \page subs List of available substances
 *  Complete list.
 *  These substances can be used as parameters to `get KIND SUB` command.
 *  \snippet BlockManager.cpp List of subs */
const QByteArray BlockFactory::subs[] = { // do not usp space, use '_'
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

const BlockFactory * blockFactory;

BlockFactory::BlockFactory() {
    for (int sub=0; sub<SUB_COUNT; ++sub) {
        normals[sub] = new Block(BLOCK, sub);
    }
    static_assert((sizeof_array(BlockFactory::kinds) == KIND_COUNT),
        "Invalid number of strings in BlockManager::kinds[].");
    static_assert((sizeof_array(BlockFactory::subs)  == SUB_COUNT),
        "Invalid number of strings in BlockManager::subs[].");
    static_assert((SUB_COUNT  <= 64 ), "too many substances, should be < 64.");
    static_assert((KIND_COUNT <= 128), "too many kinds, should be < 128.");
    /*int sum = 0;
    for (int kind = 0; kind<LAST_KIND; ++kind)
    for (int sub  = 0; sub <LAST_SUB;  ++sub) {
        sum += IsValid(kind, sub);
    }
    qDebug() << "valid pairs:" << sum;*/

    RegisterAll(typeList<Block, Bell, Container, Dwarf, Pick, Liquid, Grass,
        Bush, Rabbit, Falling, Clock, Plate, Workbench, Weapon, Ladder, Door,
        Box, Text, Map, Predator, Bucket, Shovel, Axe, Hammer, Illuminator,
        RainMachine, Converter, Armour, Helmet,  Boots, Telegraph, MedKit,
        Filter, Informer, Teleport, Accumulator>());
}

BlockFactory::~BlockFactory() {
    for(int sub=0; sub<SUB_COUNT; ++sub) {
        delete normals[sub];
    }
}

bool BlockFactory::KindSubFromFile(QDataStream & str,
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

void BlockFactory::DeleteBlock(Block * const block) const {
    if ( block != Normal(block->Sub()) ) {
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
        delete block;
    }
}

QString BlockFactory::KindToString(const int kind) const {return kinds[kind];}
QString BlockFactory:: SubToString(const int sub ) { return  subs[sub ]; }

int BlockFactory::StringToKind(const QString str) const {
    return std::find(kinds, kinds + KIND_COUNT, str) - kinds;
}

int BlockFactory::StringToSub(const QString str) {
    return std::find(subs, subs + SUB_COUNT, str) - subs;
}

Block * BlockFactory::ReplaceWithNormal(Block * const block) const {
    Block * const normal = Normal(block->Sub());
    if ( block!=normal && *block==*normal ) {
        delete block;
        return normal;
    } else {
        return block;
    }
}

bool BlockFactory::IsValid(const int kind, const int sub) {
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

void BlockFactory::RegisterAll(typeList<>) const {
    Q_ASSERT_X(kindIndex == KIND_COUNT, "BlockFactory",
        "Some classes are not registered by RegisterAll in constructor");
}

template <typename BlockType, typename ... RestBlockTypes>
void BlockFactory::RegisterAll(const typeList<BlockType, RestBlockTypes...>) {
    if (BlockType::AlreadyRegistered()) {
        //qFatal("Macro CREATE_LOAD is missing in class kind \"%d\".", kind);
        qDebug("Macro CREATE_LOAD is missing in class kind \"%d\".", kindIndex);
    }
    creates[kindIndex] = BlockType::Create;
    loads  [kindIndex] = BlockType::Load;
    kinds  [kindIndex] = BlockType::Name();
    ++kindIndex;
    RegisterAll(typeList<RestBlockTypes...>()); // recursive call
}
