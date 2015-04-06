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
#include "blocks/Text.h"
#include <QDataStream>
#include <QDebug>

#define X_NEW_BLOCK_SUB(column1, substance, ...) new Block(BLOCK, substance),

BlockFactory* BlockFactory::blockFactory = nullptr;

BlockFactory::BlockFactory() :
    normals{ SUB_TABLE(X_NEW_BLOCK_SUB) },
    creates(),
    loads()
{
    Q_ASSERT(blockFactory == nullptr);
    blockFactory = this;

    static_assert((SUB_COUNT  <= 64 ), "too many substances, should be < 64.");
    static_assert((KIND_COUNT <= 128), "too many kinds, should be < 128.");
    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        int sum = 0;
        for (int kind = 0; kind<LAST_KIND; ++kind)
        for (int sub  = 0; sub <LAST_SUB;  ++sub) {
            sum += IsValid(static_cast<kinds>(kind), static_cast<subs>(sub));
        }
        qDebug() << "valid pairs:" << sum;
    }

    RegisterAll(typeList< KIND_TABLE(X_CLASS) TemplateTerminator >());
}

BlockFactory::~BlockFactory() { qDeleteAll(ALL(normals)); }

Block* BlockFactory::NewBlock(const kinds kind, const subs sub) {
    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    }
    return blockFactory->creates[kind](kind, sub);
}

Block* BlockFactory::Normal(const int sub) {
    return blockFactory->normals[sub];
}

Block* BlockFactory::BlockFromFile(QDataStream& str,
        const kinds kind, const subs sub)
{
    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    }
    return blockFactory->loads[kind](str, kind, sub);
}

bool BlockFactory::KindSubFromFile(QDataStream& str, quint8* kind, quint8* sub)
{
    str >> *sub;
    if ( *sub & 0b10000000 ) { // normal bit
        *sub &= 0b01111111;
        return true;
    } else {
        *sub &= 0b01111111;
        str >> *kind;
        return false;
    }
}

void BlockFactory::DeleteBlock(Block* const block) {
    if ( block != blockFactory->Normal(block->Sub()) ) {
        Active* const active = block->ActiveBlock();
        if ( active ) active->Unregister();
        delete block;
    }
}

Block* BlockFactory::ReplaceWithNormal(Block* const block) {
    Block* const normal = blockFactory->Normal(block->Sub());
    if ( block!=normal && *block==*normal ) {
        delete block;
        return normal;
    } else {
        return block;
    }
}

bool BlockFactory::IsValid(const kinds kind, const subs sub) {
    const sub_groups group = Block::GetSubGroup(sub);
    switch ( kind ) {
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
Block* BlockFactory::Create(const kinds kind, const subs sub) {
    return new BlockType(kind, sub);
}

template <typename BlockType>
Block* BlockFactory::Load(QDataStream& str, const kinds kind, const subs sub) {
    return new BlockType(str, kind, sub);
}

template <typename BlockType, typename ... RestBlockTypes>
void BlockFactory::RegisterAll(typeList<BlockType, RestBlockTypes...>) {
    creates[kindIndex  ] = Create<BlockType>;
    loads  [kindIndex++] = Load  <BlockType>;
    RegisterAll(typeList<RestBlockTypes...>()); // recursive call
}
