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

#ifndef BLOCK_FACTORY_H
#define BLOCK_FACTORY_H

#include "blocks/Block.h"
#include "header.h"
#include "Singleton.h"

class Inventory;
class QDataStream;

/** * This class is used for creating and deleting blocks,
    * also for loading them from file.
    *
    * Memory management, if any, should be implemented in this class.
    * At the current moment no special memory management is used.
    *
    * Normal blocks: blocks that are not special, e.g. usual stone, air, soil
    * are actually one block (for each substance).
    * One can receive a pointer to such block with
    * Block* Normal(int sub).
    * Normal blocks are not needed to be deleted.
    * Use Block* NewBlock(int kind, int sub) to receive a pointer to
    * block that will be changed (damaged, inscribed, etc). */

class BlockFactory final : private Singleton<BlockFactory> {
public:

    BlockFactory();

    /// Use this to receive a pointer to normal block.
    static Block* Normal(int sub);
    static const Block* ConstNormal(int sub);
    static bool IsNormal(const Block* block);

    /// Use this to receive a pointer to new not-normal block.
    static Block* NewBlock(kinds kind, subs sub);

    /// Use this to load block from file.
    static Block* BlockFromFile(QDataStream& str, kinds kind, subs sub);

    /// Returns true if block is normal.
    static bool KindSubFromFile(QDataStream&, quint8* kind, quint8* sub);

    static Block* CopyBlock(const Block& block);

    /// For memory economy.
    /** Checks and replaces block with corresponding normal block. */
    static void ReplaceWithNormal(Block*& block);

    static Inventory* Block2Inventory(Block*);

private:

    static const int KIND_SUB_PAIR_VALID_CHECK = false;
    /** IsValid check if kind-sub pair is valid in game.
     *
     *  If pair is not valid, it doesn't mean that such block cannot exist.
     *  @return kind-sub pair is valid. */
    static bool IsValid(kinds, subs);

    static Block normals[SUB_COUNT];

    // Block registration system
    class FuncArrays {
    public:

        FuncArrays();

        /// Array of pointers to Create functions.
        Block* (*const creates[KIND_COUNT])(subs sub);
        /// Array of pointers to Load functions.
        Block* (*const   loads[KIND_COUNT])(QDataStream&, subs sub);
        /// Array of pointers to Copy functions
        Block* (*const  copies[KIND_COUNT])(const Block& origin);

        Inventory* (*const castsToInventory[KIND_COUNT])(Block*);

    private:

        template <typename BlockType, kinds>
        static Block* Create(subs);

        template <typename BlockType, kinds>
        static Block* Load(QDataStream&, subs);

        template <typename BlockType>
        static Block* Copy(const Block& origin);
    };

    const FuncArrays funcArrays;
};

#endif // BLOCK_FACTORY_H
