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

#include "blocks/Block.h"
#include "World.h"
#include "blocks/Inventory.h"
#include "BlockFactory.h"

#include <QDataStream>
#include <QStack>

int  Inventory::Start() const { return 0; }
bool Inventory::Access() const { return true; }

bool Inventory::Drop(const int src, int dest, int num, Inventory* const inv_to)
{
    dest = std::max(inv_to->Start(), dest);
    bool ok_flag = false;
    while ( num-- ) {
        if ( src < GetSize()
                && dest < inv_to->GetSize()
                && not inventory[src].isEmpty()
                && inv_to->Get(inventory[src].top(), dest) )
        {
            ok_flag = true;
            Pull(src);
        }
    }
    if ( ok_flag ) {
        TrString lighterString = World::tr("Your inventory is lighter now.");
        ReceiveSignal(lighterString);
    }
    return ok_flag;
}

bool Inventory::GetAll(Inventory* const from) {
    bool flag = false;
    for (int i=0; i<from->GetSize(); ++i) {
        if ( from->Drop(i, 0, from->Count(i), this) ) {
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

void Inventory::SaveAttributes(QDataStream& out) const {
    for (int i=0; i<GetSize(); ++i) {
        const int number = Count(i);
        out << quint8(number);
        if ( number > 0 ) {
            /// @todo save each block separately
            const Block* const to_save = inventory[i].top();
            for (int j=0; j<number; ++j) {
                if ( BlockFactory::IsNormal(to_save) ) {
                    to_save->SaveNormalToFile(out);
                } else {
                    to_save->SaveToFile(out);
                }
            }
        }
    }
}

bool Inventory::Get(Block* const block, const int start) {
    if ( block == nullptr ) return true;
    if ( block->Wearable() == WEARABLE_VESSEL ) {
        for (int i=0; i<GetSize(); ++i) {
            if ( Count(i)==1 && ShowBlock(i) ) {
                Inventory* const inner = ShowBlock(i)->HasInventory();
                if ( inner && inner->Get(block) ) {
                    return true;
                }
            }
        }
    } else {
        for (int i=start; i<GetSize(); ++i) {
            if ( GetExact(block, i) ) {
                TrString haveString = QObject::tr("You have %1 at slot '%2'.");
                ReceiveSignal(haveString
                    .arg(block->FullName())
                    .arg(char(i + 'a')));
                return true;
            }
        }
    }
    TrString noRoomString = QObject::tr("No room.");
    ReceiveSignal(noRoomString);
    return false;
}

bool Inventory::GetExact(Block* const block, const int num) {
    if ( block == nullptr) return true;
    if ( inventory[num].isEmpty() ||
            (*block == *inventory[num].top() && Count(num)<MAX_STACK_SIZE) )
    {
        inventory[num].push(block);
        return true;
    } else {
        return false;
    }
}

void Inventory::MoveInside(const int from, const int num_to, const int num) {
    for (int i=0; i<num; ++i) {
        if ( GetExact(ShowBlock(from), num_to) ) {
            Pull(from);
        }
    }
}

bool Inventory::InscribeInv(const int num, const QString& str) {
    const int number = Count(num);
    if ( number == 0 ) {
        return false;
    }
    const int sub = inventory[num].top()->Sub();
    if ( BlockFactory::IsNormal(inventory[num].top()) ) {
        for (int i=0; i<number; ++i) {
            inventory[num].replace(i, BlockFactory::Normal(sub));
        }
    }
    for (int i=0; i<number; ++i) {
        if ( not inventory[num].at(i)->Inscribe(str) ) {
            TrString cannotInscribeString = QObject::tr("Cannot inscribe %1.");
            ReceiveSignal(cannotInscribeString.arg(InvFullName(num)));
            return false;
        }
    }
    TrString inscribedString = QObject::tr("Inscribed %1.");
    ReceiveSignal(inscribedString.arg(InvFullName(num)));
    return true;
}

QString Inventory::InvFullName(const int num) const {
    TrString emptyString = World::tr("-empty-");
    return inventory[num].isEmpty() ?
        emptyString :
        Str("%1%2").
            arg( inventory[num].top()->FullName() ).
            arg( Count(num) <= 1 ?
                QString() :
                Str(" (x%1)").arg(Count(num)) );
}

int Inventory::GetInvWeight(const int i) const {
    return GetSlotWeight(inventory[i]);
}

int Inventory::GetSlotWeight(const QStack<Block*>& slot) {
    return slot.isEmpty() ?
        0 : slot.top()->Weight() * slot.size();
}

subs Inventory::GetInvSub(const int i) const {
    return inventory[i].isEmpty() ?
        AIR : inventory[i].top()->Sub();
}

kinds Inventory::GetInvKind(const int i) const {
    return inventory[i].isEmpty() ?
        BLOCK : inventory[i].top()->Kind();
}

int Inventory::Weight() const {
    return std::accumulate(inventory, inventory + GetSize(), 0,
        [](const int sum, const auto& slot) {
            return sum + GetSlotWeight(slot);
    });
}

int Inventory::Count(const int i) const { return inventory[i].size(); }

Block* Inventory::ShowBlock(const int slot) const {
    return ( slot >= GetSize() || inventory[slot].isEmpty() ) ?
        nullptr : inventory[slot].top();
}

bool Inventory::IsEmpty() const {
    return std::all_of(inventory + Start(), inventory + GetSize(),
        [](const auto& slot) { return slot.isEmpty(); });
}

bool Inventory::IsEmpty(const int i) const { return inventory[i].isEmpty(); }

InvIterator Inventory::begin() const { return InvIterator(this,           -1); }
InvIterator Inventory::end()   const { return InvIterator(this, GetSize() -1); }

void Inventory::Push(const_int(x, y, z), const int push_direction) {
    const World* const world = World::GetCWorld();
    int x_targ, y_targ, z_targ;
    world->Focus(x, y, z, &x_targ, &y_targ, &z_targ,
        World::Anti(Block::MakeDirFromDamage(push_direction)));
    Inventory* const inv =
        world->GetBlock(x_targ, y_targ, z_targ)->HasInventory();
    if ( inv ) inv->GetAll(this);
}

void Inventory::Shake() {
    for (int i=Start(); i<GetSize()-1; ++i) {
        if ( Count(i) != MAX_STACK_SIZE ) {
            for (int j=i; j<GetSize(); ++j) {
                MoveInside(j, i, Count(j));
            }
        }
    }
}

Inventory::Inventory(const int sz)
    : inventorySize(sz)
    , inventory(new QStack<Block*>[sz])
{}

Inventory::Inventory(QDataStream& str, const int sz)
    : Inventory(sz)
{
    for (int i=0; i<GetSize(); ++i) {
        quint8 num;
        str >> num;
        while ( num-- ) {
            quint8 kind, sub;
            inventory[i].push(BlockFactory::KindSubFromFile(str, &kind, &sub) ?
                BlockFactory::Normal(static_cast<subs>(sub)) :
                BlockFactory::BlockFromFile(str, static_cast<kinds>(kind),
                                                 static_cast<subs >(sub)));
        }
    }
}

Inventory::Inventory(const Inventory& origin)
    : Inventory(origin.inventorySize)
{}

Inventory::~Inventory() {
    std::for_each(inventory, inventory + GetSize(), [](const auto& inv) {
        qDeleteAll(inv);
    });
    delete [] inventory;
}

// Define pure virtual functions to simplify debugging
void Inventory::ReceiveSignal(const QString&) { Q_UNREACHABLE(); }

QString Inventory::FullName() const {
    Q_UNREACHABLE();
    return QString();
}

bool InvIterator::operator!=(const InvIterator& other) const {
    return ( other.index    != index
          || other.iterated != iterated );
}

void InvIterator::operator++()
{
    const int size = iterated->GetSize();
    do { ++index; } while (index < size && iterated->IsEmpty(index));
}

Block* InvIterator::operator*() const { return iterated->ShowBlock(index); }

InvIterator::InvIterator(const Inventory* const _iterated, const int i)
    : iterated(_iterated)
    , index(i)
{
    operator++();
}
