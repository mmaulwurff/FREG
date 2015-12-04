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
#include "blocks/Pipe.h"
#include <QDataStream>
#include <QDebug>
#include <type_traits>

#define X_BLOCK_CONSTRUCT(column1, substance, ...) { BLOCK, substance },

BlockFactory* BlockFactory::blockFactory = nullptr;

Block BlockFactory::normals[] = { SUB_TABLE(X_BLOCK_CONSTRUCT) };

BlockFactory::BlockFactory()
    : creates()
    , loads()
    , castsToInventory()
{
    Q_ASSERT(blockFactory == nullptr);
    blockFactory = this;

    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        int sum = 0;
        for (int kind = 0; kind<LAST_KIND; ++kind)
        for (int sub  = 0; sub <LAST_SUB;  ++sub ) {
            sum += IsValid(static_cast<kinds>(kind), static_cast<subs>(sub));
        }
        qDebug() << "valid pairs:" << sum;
    }

    RegisterAll(typeList< KIND_TABLE(X_CLASS) TemplateTerminator >());
}

Block* BlockFactory::NewBlock(const kinds kind, const subs sub) {
    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    }
    return blockFactory->creates[kind](sub);
}

Block* BlockFactory::Normal(const int sub) {
    return &(blockFactory->normals[sub]);
}

const Block* BlockFactory::ConstNormal(const int sub) { return Normal(sub); }

Block* BlockFactory::BlockFromFile(QDataStream& str,
        const kinds kind, const subs sub)
{
    if ( KIND_SUB_PAIR_VALID_CHECK ) {
        qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
    }
    return blockFactory->loads[kind](str, sub);
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

Inventory *BlockFactory::Block2Inventory(Block* const block) {
    return blockFactory->castsToInventory[block->Kind()](block);
}

void BlockFactory::DeleteBlock(Block* const block) {
    if ( block != blockFactory->Normal(block->Sub()) ) {
        if (Active* const active = block->ActiveBlock()) active->Unregister();
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

Q_DECL_RELAXED_CONSTEXPR bool BlockFactory::IsValid(const kinds kind,
                                                    const subs sub)
{
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
    case PIPE:
    case SIGNALLER: return ( group == GROUP_METAL );

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

template <typename BlockType, kinds kind>
Block* BlockFactory::Create(const subs sub) {
    return new BlockType(kind, sub);
}

template <typename BlockType, kinds kind>
Block* BlockFactory::Load(QDataStream& stream, const subs sub) {
    return new BlockType(stream, kind, sub);
}

template <typename BlockType, typename Base, std::enable_if_t<
        std::is_base_of<Base, BlockType>::value >* = nullptr>
Base* castTo(Block* const block) { return static_cast<BlockType*>(block); }

template <typename BlockType, typename Base, std::enable_if_t< not
        std::is_base_of<Base, BlockType>::value >* = nullptr>
Base* castTo(Block*) { return nullptr; }

template <typename BlockType, typename ... RestBlockTypes>
void BlockFactory::RegisterAll(typeList<BlockType, RestBlockTypes...>)
{
    static const kinds kind =
        static_cast<kinds>(KIND_COUNT - sizeof...(RestBlockTypes));
    creates[kind] = Create<BlockType, kind>;
    loads  [kind] = Load  <BlockType, kind>;
    castsToInventory[kind] = castTo<BlockType, Inventory>;

    RegisterAll(typeList<RestBlockTypes...>());
}
