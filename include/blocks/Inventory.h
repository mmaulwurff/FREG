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

/** @file
 *  Provides declaration for class Inventory for freg. */

#ifndef INVENTORY_H
#define INVENTORY_H

#include "header.h"
#include <QtGlobal>

class Block;
template<typename Block> class QStack;

/** Provides block ability to contain other blocks inside. */
class Inventory {
    Inventory& operator=(const Inventory&) = delete;

public:

    virtual QString FullName() const = 0;

    /** @name Information section */ ///@{
    virtual bool Access() const;
    virtual int Weight() const;

    int Count(int i) const;

    /// Returns true if all inventory slots are empty.
    bool IsEmpty() const;
    /// Returns true if particular inventory slot is empty.
    bool IsEmpty(int slot_index) const;

    int GetSize() const { return inventorySize; }
    ///@} // < Information section

    /** @name Modifications section */ ///@{


    ///@}

    /// Returns true on success.
    virtual bool Drop(int src, int dest, int num, Inventory* to);
    /// Returns true on success.
    virtual bool GetAll(Inventory* from);
    /// Returns true on success.
    virtual bool Get(Block* block, int start = 0);
    virtual void ReceiveSignal(const QString&) = 0;
    virtual int Start() const;
    /// Returns true if block found its place.
    virtual bool GetExact(Block* block, int num);
    virtual QString InvFullName(int num) const;
    /// Removes block from inventory. Does not delete block.
    virtual void Pull(int slot);

    void MoveInside(int num_from, int num_to, int num);
    /// Returns true on success (something has been crafted).
    bool MiniCraft(int num);
    /// Returns true on success.
    bool InscribeInv(int num, const QString& str);
    /// Returns AIR if slot number i is empty.
    subs GetInvSub(int i) const;
    /// Returns BLOCK if slot number i is empty.
    kinds GetInvKind(int i) const;
    int GetInvWeight(int i) const;
    Block* ShowBlock(int slot) const;

    void Push(int x, int y, int z, int push_direction);
    /// Stacks items in inventory if possible.
    void Shake();

    static const int MAX_STACK_SIZE = 9;

protected:
    static const int INV_SIZE = 26;

    /// It is not recommended to make inventory size more than 26.
    /** Because it will not be convenient to deal with inventory
     *  in console version. */
    explicit Inventory(int sz = INV_SIZE);
    explicit Inventory(QDataStream& str, int size = INV_SIZE);
    Inventory(const Inventory&);
    virtual ~Inventory();

    virtual void SaveAttributes(QDataStream& out) const;

private:
    static int GetSlotWeight(const QStack<Block*>& slot);

    const quint8 inventorySize;
    QStack<Block*>* const inventory;
};

#endif // INVENTORY_H
