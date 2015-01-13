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

BlockFactory * BlockFactory::blockFactory = nullptr;

BlockFactory::BlockFactory() {
    Q_ASSERT(blockFactory == nullptr);
    blockFactory = this;

    for (int sub=0; sub<SUB_COUNT; ++sub) {
        normals[sub] = new Block(BLOCK, sub);
    }
    static_assert((SUB_COUNT  <= 64 ), "too many substances, should be < 64.");
    static_assert((KIND_COUNT <= 128), "too many kinds, should be < 128.");
    /*int sum = 0;
    for (int kind = 0; kind<LAST_KIND; ++kind)
    for (int sub  = 0; sub <LAST_SUB;  ++sub) {
        sum += IsValid(kind, sub);
    }
    qDebug() << "valid pairs:" << sum;*/

    RegisterAll(typeList< KIND_TABLE(X_CLASS) TemplateTerminator >());
}

BlockFactory::~BlockFactory() { qDeleteAll(normals, normals + SUB_COUNT); }

Block * BlockFactory::NewBlock(const int kind, const int sub) {
    //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    return blockFactory->creates[kind](kind, sub);
}

Block * BlockFactory::Normal(const int sub) {
    return blockFactory->normals[sub];
}

Block * BlockFactory::BlockFromFile(QDataStream & str,
        const int kind, const int sub)
{
    //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    return blockFactory->loads[kind](str, kind, sub);
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

void BlockFactory::DeleteBlock(Block * const block) {
    if ( block != blockFactory->Normal(block->Sub()) ) {
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
        delete block;
    }
}

Block * BlockFactory::ReplaceWithNormal(Block * const block) {
    Block * const normal = blockFactory->Normal(block->Sub());
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

template <typename BlockType>
Block * BlockFactory::Create(const int kind, const int sub) {
    return new BlockType(kind, sub);
}

template <typename BlockType>
Block * BlockFactory::Load(QDataStream & str, const int kind, const int sub) {
    return new BlockType(str, kind, sub);
}

template <typename BlockType, typename ... RestBlockTypes>
void BlockFactory::RegisterAll(typeList<BlockType, RestBlockTypes...>) {
    creates[kindIndex  ] = Create<BlockType>;
    loads  [kindIndex++] = Load  <BlockType>;
    RegisterAll(typeList<RestBlockTypes...>()); // recursive call
}
