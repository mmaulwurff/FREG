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

#include "Block.h"
#include "Inventory.h"
#include "CraftManager.h"
#include "BlockManager.h"
#include "world.h"

bool   Inventory::Access() const { return true; }
int Inventory::Start() const { return 0; }
int Inventory::Size() const { return size; }
Inventory * Inventory::HasInventory() { return this; }

bool Inventory::Drop(const int src, int dest, int num,
        Inventory * const inv_to)
{
    dest = qMax(inv_to->Start(), dest);
    bool ok_flag = false;
    for ( ; num; --num) {
        if ( src < Size()
                && dest < inv_to->Size()
                && !inventory[src].isEmpty()
                && inv_to->Get(inventory[src].top(), dest) )
        {
            ok_flag = true;
            Pull(src);
        }
    }
    return ok_flag;
}

bool Inventory::GetAll(Inventory * const from) {
    bool flag = false;
    for (int i=0; i<from->Size(); ++i) {
        if ( from->Drop(i, 0, from->Number(i), this) ) {
            flag = true;
        }
    }
    return flag;
}

void Inventory::Pull(const int num) {
    if ( not inventory[num].isEmpty() ) {
        inventory[num].pop();
    }
}

void Inventory::SaveAttributes(QDataStream & out) const {
    for (int i=0; i<Size(); ++i) {
        out << Number(i);
        for (int j=0; j<Number(i); ++j) {
            inventory[i].top()->SaveToFile(out);
        }
    }
}

bool Inventory::Get(Block * const block, const int start) {
    if ( block == nullptr ) return true;
    if ( block->Kind() == LIQUID ) {
        for (int i=qMax(Start(), start); i<Size(); ++i) {
            if ( Number(i)==1 && ShowBlock(i) ) {
                Inventory * const inner = ShowBlock(i)->HasInventory();
                if ( inner && inner->Get(block) ) {
                    return true;
                }
            }
        }
        return false;
    } // else:
    for (int i=qMax(Start(), start); i<Size(); ++i) {
        if ( GetExact(block, i) ) {
            return true;
        }
    }
    return false;
}

bool Inventory::GetExact(Block * const block, const int num) {
    if ( block ) {
        if ( inventory[num].isEmpty() ) {
            inventory[num].push(block);
        } else if ( *block == *inventory[num].top()
                && Number(num) < MAX_STACK_SIZE )
        {
            Inventory * const inner = inventory[num].top()->HasInventory();
            if ( inner==nullptr || inner->IsEmpty() ) {
                inventory[num].push(block);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

void Inventory::MoveInside(const int num_from, const int num_to,
        const int num)
{
    for (int i=0; i<num; ++i) {
        if ( GetExact(ShowBlock(num_from), num_to) ) {
            Pull(num_from);
        }
    }
}

bool Inventory::InscribeInv(const int num, const QString str) {
    const int number = Number(num);
    if ( number == 0 ) {
        ReceiveSignal(QObject::tr("Nothing here."));
        return false;
    }
    const int sub = inventory[num].top()->Sub();
    if ( inventory[num].top() == block_manager.NormalBlock(sub) ) {
        for (int i=0; i<number; ++i) {
            inventory[num].replace(i, block_manager.NormalBlock(sub));
        }
    }
    for (int i=0; i<number; ++i) {
        if ( !inventory[num].at(i)->Inscribe(str) ) {
            ReceiveSignal(QObject::tr("Cannot inscribe this."));
            return false;
        }
    }
    ReceiveSignal(QObject::tr("Inscribed."));
    return true;
}

QString Inventory::InvFullName(const int num) const {
    return inventory[num].isEmpty() ?
        "" : inventory[num].top()->FullName();
}

QString Inventory::NumStr(const int num) const {
    return QString(" (%1x)").arg(Number(num));
}

int Inventory::GetInvWeight(const int i) const {
    return inventory[i].isEmpty() ?
        0 : inventory[i].top()->Weight()*Number(i);
}

int Inventory::GetInvSub(const int i) const {
    return inventory[i].isEmpty() ?
        AIR : inventory[i].top()->Sub();
}

int Inventory::GetInvKind(const int i) const {
    return inventory[i].isEmpty() ?
        BLOCK : int(inventory[i].top()->Kind());
}

QString Inventory::GetInvNote(const int num) const {
    return inventory[num].top()->GetNote();
}

int Inventory::Weight() const {
    int sum = 0;
    for (int i=0; i<Size(); ++i) {
        sum += GetInvWeight(i);
    }
    return sum;
}

Block * Inventory::ShowBlock(const int slot) const {
    return ( slot > Size() || Number(slot)==0 ) ?
        nullptr : inventory[slot].top();
}

Block * Inventory::ShowBlock(const int slot, const int num) const {
    return ( slot > Size() || num+1 > Number(slot) ) ?
        nullptr : inventory[slot].at(num);
}

bool Inventory::IsEmpty() const {
    for (int i=Start(); i<Size(); ++i) {
        if ( not inventory[i].isEmpty() ) {
            return false;
        }
    }
    return true;
}

void Inventory::Push(Block * const who) {
    Inventory * const inv = who->HasInventory();
    if ( inv ) {
        inv->GetAll(this);
    }
}

quint8 Inventory::Number(const int i) const {
    return inventory[i].size();
}

bool Inventory::MiniCraft(const int num) {
    if ( Number(num) == 0 ) {
        ReceiveSignal(QObject::tr("Nothing here."));
        return false;
    } // else:
    const CraftItem * const crafted = world->GetCraftManager()->MiniCraft(
        Number(num), BlockManager::MakeId(GetInvKind(num),GetInvSub(num)));
    if ( crafted != nullptr ) {
        while ( not inventory[num].isEmpty() ) {
            Block * const to_delete = ShowBlock(num);
            Pull(num);
            block_manager.DeleteBlock(to_delete);
        }
        for (int i=0; i<crafted->num; ++i) {
            GetExact(block_manager.NewBlock(
                BlockManager::KindFromId(crafted->id),
                BlockManager:: SubFromId(crafted->id) ), num);
        }
        ReceiveSignal(QObject::tr("Craft successful."));
        delete crafted;
        return true;
    } else {
        ReceiveSignal(QObject::tr("You don't know how to craft this."));
        return false;
    }
}

void Inventory::Shake() {
    for (int i=Start(); i<Size()-1; ++i) {
        if ( Number(i) == MAX_STACK_SIZE ) {
            continue;
        }
        for (int j=i; j<Size(); ++j) {
            MoveInside(j, i, Number(j));
        }
    }
}

Inventory::Inventory(const int sz) :
        size(sz),
        inventory(new QStack<Block *>[sz])
{}

Inventory::Inventory(QDataStream & str, const int sz) :
        Inventory(sz)
{
    for (int i=0; i<Size(); ++i) {
        quint8 num;
        str >> num;
        while ( num-- ) {
            inventory[i].push(block_manager.
                BlockFromFile(str));
        }
    }
}

Inventory::~Inventory() {
    for (int i=0; i<Size(); ++i) {
        while ( !inventory[i].isEmpty() ) {
            block_manager.DeleteBlock(inventory[i].pop());
        }
    }
    delete [] inventory;
}
