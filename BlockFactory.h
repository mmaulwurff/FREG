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

#ifndef BLOCKFACTORY_H
#define BLOCKFACTORY_H

#include "header.h"

class Block;
class QDataStream;

/** \class BlockFactory BlockFactory.h
     * \brief This class is used for creating and deleting blocks,
     * also for loading them from file.
     *
     * Memory management, if any, should be implemented in this class.
     * At the current moment no special memory management is used.
     *
     * Normal blocks: blocks that are not special, e.g. usual stone, air, soil
     * are actually one block (for each substance).
     * One can receive a pointer to such block with
     * Block * Normal(int sub).
     * Normal blocks are not needed to be deleted.
     * Use Block * NewBlock(int kind, int sub) to receive a pointer to
     * block that will be changed (damaged, inscribed, etc). */

class BlockFactory final {
public:
     BlockFactory();
    ~BlockFactory();

    /// Use this to receive a pointer to normal block.
    static Block * Normal(const int sub);

    /// Use this to receive a pointer to new not-normal block.
    static Block * NewBlock(int kind, int sub);

    /// Use this to load block from file.
    static Block * BlockFromFile(QDataStream & str, int kind, int sub);

    /// Returns true if block is normal.
    static bool KindSubFromFile(QDataStream &, quint8 * kind, quint8 * sub);

    /// Does not actually delete normal blocks.
    static void DeleteBlock(Block *);

    /// For memory economy.
    /** Checks and replaces block with corresponding normal block.
     *  Can delete block, use carefully. */
    static Block * ReplaceWithNormal(Block * block);

    constexpr static int MakeId(const int kind, const int sub) {
        return (kind << 6) | sub;
    }

    static int KindFromId(const int id) { return (id >>   8); }
    static int SubFromId (const int id) { return (id & 0xFF); }

    static bool IsValid(int kind, int sub);

private:
    Q_DISABLE_COPY(BlockFactory)

    Block * normals[SUB_COUNT];

    // Block registration system:

    /// Array of pointers to Create functions.
    Block * (* creates[KIND_COUNT])(int kind, int sub);
    /// Array of pointers to Load functions.
    Block * (*   loads[KIND_COUNT])(QDataStream &, int kind, int sub);

    template <typename BlockType>
    static Block * Create(int kind, int sub);
    template <typename BlockType>
    static Block * Load(QDataStream & str, int kind, int sub);

    /// Type list struct for variadic template without formal parameters.
    template <class ...> struct typeList {};
    struct TemplateTerminator {};

    ///< Base for variadic template.
    void RegisterAll(typeList<TemplateTerminator>) {}

    /// Core of block registration system.
    template <typename BlockType, typename ... RestBlockTypes>
    void RegisterAll(typeList<BlockType, RestBlockTypes...>);
    int kindIndex = 0;

    static BlockFactory * blockFactory;
};

#endif // BLOCKFACTORY_H
