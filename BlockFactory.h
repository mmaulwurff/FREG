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
#include <QByteArray>

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
    Block * Normal(const int sub) const { return normals[sub]; }

    /// Use this to receive a pointer to new not-normal block.
    Block * NewBlock(const int kind, const int sub) const {
        //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
        return creates[kind](kind, sub);
    }

    /// Use this to load block from file.
    Block * BlockFromFile(QDataStream & str, const int kind, const int sub)
    const {
        //qDebug("kind: %d, sub: %d, valid: %d", kind, sub, IsValid(kind,sub));
        return loads[kind](str, kind, sub);
    }

    /// Returns true if block is normal.
    static bool KindSubFromFile(QDataStream &, quint8 * kind, quint8 * sub);
    /// Does not actually delete normal blocks.
    void DeleteBlock(Block *) const;
    /// For memory economy.
    /** Checks and replaces block with corresponding normal block.
     *  Can delete block, use carefully. */
    Block * ReplaceWithNormal(Block * block) const;

    /// If kind is unknown, returns "unknown_kind".
    static QString KindToString(int kind);
    /// If substance is unknown, returns "unknown_sub".
    static QString SubToString(int sub);
    /// If string is not convertible to kind, returns LAST_KIND.
    static int StringToKind(QString);
    /// If string is not convertible to substance, returns LAST_SUB.
    static int StringToSub(QString);

    constexpr static int MakeId(const int kind, const int sub) {
        return (kind << 6) | sub;
    }

    static int KindFromId(const int id) { return (id >>   8); }
    static int SubFromId (const int id) { return (id & 0xFF); }

    static bool IsValid(int kind, int sub);

private:
    Q_DISABLE_COPY(BlockFactory)

    Block * normals[SUB_COUNT];
    static const QByteArray kinds[];
    static const QByteArray subs [];

    // Block registration system:

    /// Array of pointers to Create functions.
    Block * (* creates[KIND_COUNT])(int kind, int sub);
    /// Array of pointers to Load functions.
    Block * (*   loads[KIND_COUNT])(QDataStream &, int kind, int sub);

    /// Type list struct for variadic template without formal parameters.
    template <class ...> struct typeList {};

    /// For check for registration of all block types.
    int kindIndex = 0;

    /// Core of block registration system.
    template <typename BlockType, typename ... RestBlockTypes>
    void RegisterAll(typeList<BlockType, RestBlockTypes...>);
    void RegisterAll(typeList<>) const; ///< Base for variadic template.
};

extern const BlockFactory * blockFactory;

#endif // BLOCKFACTORY_H
