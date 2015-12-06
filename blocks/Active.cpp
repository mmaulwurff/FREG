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

#include "blocks/Active.h"
#include "Shred.h"
#include "World.h"
#include "blocks/Inventory.h"
#include "TrManager.h"
#include "AroundCoordinates.h"
#include <QDataStream>

// Active section

int Active::X() const {
    return GetShred()->ShredX() << SHRED_WIDTH_BITSHIFT | Xyz::X();
}

int Active::Y() const {
    return GetShred()->ShredY() << SHRED_WIDTH_BITSHIFT | Xyz::Y();
}

const XyzInt Active::GetXyz() const { return {X(), Y(), Z()}; }

int  Active::ShouldAct() const { return FREQUENT_NEVER; }
bool Active::IsInside()  const { return shred == nullptr; }
int  Active::Attractive(int) const { return 0; }
void Active::ReceiveSignal(const QString& str) { emit ReceivedText(str); }
Active* Active::ActiveBlock() { return this; }
inner_actions Active::ActInner() { return INNER_ACTION_ONLY; }
const Active *Active::ActiveBlockConst() const { return this; }

void Active::UpdateLightRadius(const int old_radius) {
    if ( IsInside() ) return;
    const int radius = LightRadius();
    if ( radius != old_radius ) {
        const XyzInt xyz = GetXyz();
        World* const world = World::GetWorld();
        world->Shine(xyz, -old_radius);
        world->Shine(xyz, radius);
        emit world->UpdatedAround(XYZ(xyz));
    }
    if ( radius && old_radius == 0 ) {
        GetShred()->AddShining(this);
    } else if ( old_radius && radius == 0 ) {
        GetShred()->RemShining(this);
    }
}

void Active::ActFrequent() {
    qDebug("Active::ActFrequent called, check ShouldAct\
 return value or add ActFrequent implementation.\nKind: %d, sub: %d.",
        Kind(), Sub());
    Q_UNREACHABLE();
}

void Active::DoRareAction() {
    qDebug("Active::DoRareAction called, check ShouldAct and ActInner\
 return values or add DoRareAction implementation.\nKind: %d, sub: %d.",
        Kind(), Sub());
    Q_UNREACHABLE();
}

void Active::ActRare() {
    Inventory* const inv = HasInventory();
    if (Q_UNLIKELY(inv != nullptr)) {
        for (int i=inv->Size()-1; i; --i) {
            const int number = inv->Number(i);
            if ( number == 0 ) continue;
            Active* const top_active = inv->ShowBlock(i)->ActiveBlock();
            if ( top_active == nullptr ) continue;
            for (int j=0; j<number; ++j) {
                Active* const active =
                    inv->ShowBlockInSlot(i, j)->ActiveBlock();
                if ( active->ActInner() == INNER_ACTION_MESSAGE ) {
                    ReceiveSignal( tr("%1 in slot '%2': %3").
                        arg(inv->InvFullName(i)).
                        arg(char('a'+i)).
                        arg(inv->ShowBlockInSlot(i, j)->GetNote()) );
                }
            }
        }
    }
    DoRareAction();
}

void Active::Unregister() {
    if ( not IsInside() ) {
        shred->Unregister(this);
        shred = nullptr;
        World::GetWorld()->RemoveTempShining(this);
    }
}

void Active::Move(const dirs dir) {
    switch ( dir ) {
    case UP:    ++z_self; break;
    case DOWN:  --z_self; break;
    case NORTH: --y_self; ReRegister(NORTH); break;
    case EAST:  ++x_self; ReRegister(EAST ); break;
    case SOUTH: ++y_self; ReRegister(SOUTH); break;
    case WEST:  --x_self; ReRegister(WEST ); break;
    }
    emit Moved(dir);
}

void Active::ReRegister(const dirs dir) {
    if ( Shred::InBounds(x_self, y_self) ) return;
    x_self &= 0xF;
    y_self &= 0xF;
    shred->Unregister(this);
    (shred = World::GetCWorld()->GetNearShred(shred, dir))->Register(this);
}

void Active::SendSignalAround(const QString& signal) const {
    if ( IsInside() ) return; // for blocks inside inventories
    const World* const world = World::GetCWorld();
    for (const XyzInt& xyz : AroundCoordinates(X(), Y(), Z())) {
        world->GetBlock(XYZ(xyz))->ReceiveSignal(signal);
    }
}

void Active::DamageAround() const {
    for (const XyzInt& xyz : AroundCoordinates(X(), Y(), Z())) {
        TryDestroy(XYZ(xyz));
    }
}

bool Active::TryDestroy(const int x, const int y, const int z) const {
    World* const world = World::GetWorld();
    if ( world->Damage(x, y, z, DamageLevel(), DamageKind()) <= 0 ) {
        world->DestroyAndReplace(x, y, z);
        return true;
    } else {
        return false;
    }
}

Active::Active(const kinds kind, const subs sub)
    : Block(kind, sub)
    , Xyz()
{}

Active::Active(QDataStream& str, const kinds kind, const subs sub)
    : Block(str, kind, sub)
    , Xyz()
{}

bool Active::Gravitate(const int range, int bottom, int top,
        const int calmness)
{
    const World* const world = World::GetCWorld();
    const int bound = World::GetBound();
    // analyze world around
    int for_north = 0, for_west = 0;
    const int my_x = X();
    const int my_y = Y();
    const int y_start = std::max(my_y-range, 0);
    const int y_end   = std::min(my_y+range, bound);
    const int x_end   = std::min(my_x+range, bound);
    bottom = std::max(Z() - bottom,  0);
    top    = std::min(Z() + top,     HEIGHT-1);
    const Xyz my_place(my_x, my_y, Z());
    for (int x=std::max(my_x-range, 0); x<=x_end; ++x)
    for (int y=y_start; y<=y_end; ++y) {
        Shred* const current_shred = world->GetShred(x, y);
        for (int z=bottom; z<=top; ++z) {
            const int attractive = Attractive(current_shred->GetBlock(
                Shred::CoordInShred(x), Shred::CoordInShred(y), z)->Sub());
            if (attractive && world->DirectlyVisible(my_place, Xyz(x, y, z))) {
                if ( y!=my_y ) for_north += attractive / (my_y-y);
                if ( x!=my_x ) for_west  += attractive / (my_x-x);
            }
        }
    }
    // make direction and move there
    if ( abs(for_north)>calmness || abs(for_west)>calmness ) {
        SetDir( ( abs(for_north)>abs(for_west) ) ?
            ( ( for_north>0 ) ? NORTH : SOUTH ) :
            ( ( for_west >0 ) ? WEST  : EAST  ) );
        return true;
    } else {
        return false;
    }
}

bool Active::IsSubAround(const int sub) const {
    LazyAroundCoordinates coordinates(X(), Y(), Z());
    const XyzInt* xyz;
    while ((xyz = coordinates.getNext())) {
        if (World::GetCWorld()->
                GetBlock(xyz->X(), xyz->Y(), xyz->Z())->Sub() == sub)
        {
            return true;
        }
    }
    return false;
}

// Falling section

Falling::Falling(const kinds kind, const subs sub)
    : Active(kind, sub)
    , fallHeight(0)
    , falling(false)
{}

Falling::Falling(QDataStream& str, const kinds kind, const subs sub)
    : Active(str, kind, sub)
    , fallHeight()
    , falling()
{
    str >> fallHeight >> falling;
}

void Falling::SaveAttributes(QDataStream& out) const {
    out << fallHeight << falling;
}

QString Falling::FullName() const {
    switch ( Sub() ) {
    default:    return TrManager::SubNameUpper(Sub());
    case WATER: return TrManager::KindName(FALLING);
    case STONE: return tr("Masonry");
    }
}

Falling* Falling::ShouldFall() { return this; }
push_reaction Falling::PushResult(dirs) const { return MOVABLE; }

void Falling::FallDamage() {
    const int SAFE_FALL_HEIGHT = 5;
    if ( fallHeight > SAFE_FALL_HEIGHT ) {
        const int dmg = (fallHeight - SAFE_FALL_HEIGHT)*10;
        World* const world = World::GetWorld();
        Block* const block_under = world->GetBlock(X(), Y(), Z()-1);
        world->Damage(X(), Y(), Z()-1, dmg, DamageKind());
        if ( block_under->GetDurability() <= 0 ) {
            world->DestroyAndReplace(X(), Y(), Z()-1);
        }
        Damage(dmg, block_under->DamageKind());
        if ( GetDurability() <= 0 ) {
            world->DestroyAndReplace(X(), Y(), Z());
            return;
        }
    }
    falling = false;
    fallHeight = 0;
}

void Falling::Move(const dirs dir) {
    Active::Move(dir);
    if ( DOWN == dir ) {
        ++fallHeight;
    }
}

void Falling::SetFalling(const bool set) {
    if ( not (falling=set) ) {
        fallHeight = 0;
    }
}
