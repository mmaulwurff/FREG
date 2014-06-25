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

#include "Active.h"
#include "Shred.h"
#include "world.h"
#include "DeferredAction.h"
#include "Xyz.h"
#include "blocks/Inventory.h"

// Active section

Active * Active::ActiveBlock() { return this; }
int  Active::ShouldAct() const { return FREQUENT_NEVER; }
void Active::DoFrequentAction() {}
void Active::DoRareAction() {}
INNER_ACTIONS Active::ActInner() { return INNER_ACTION_NONE; }
push_reaction Active::PushResult(dirs) const { return NOT_MOVABLE; }

void Active::ActFrequent() {
    if ( GetDeferredAction() != nullptr ) {
        GetDeferredAction()->MakeAction();
    }
    DoFrequentAction();
}

void Active::ActRare() {
    Inventory * const inv = HasInventory();
    if ( inv != nullptr ) {
        for (int i=inv->Size()-1; i; --i)
        for (int j=0; j<inv->Number(i); ++j) {
            Active * const active = inv->ShowBlock(i, j)->ActiveBlock();
            if ( active!=nullptr && active->ActInner()==INNER_ACTION_MESSAGE )
            {
                ReceiveSignal(tr("Item in slot '%1' changed status.").
                    arg(char('a'+i)));
                ReceiveSignal(inv->ShowBlock(i, j)->GetNote());
            }
        }
    }
    DoRareAction();
}

void Active::SetDeferredAction(DeferredAction * const action) {
    delete deferredAction;
    deferredAction = action;
}

DeferredAction * Active::GetDeferredAction() const { return deferredAction; }

void Active::Unregister() {
    if ( shred != nullptr ) {
        shred->Unregister(this);
        shred = nullptr;
    }
}

void Active::Move(const dirs dir) {
    switch ( dir ) {
    case UP:    ++z_self; break;
    case DOWN:  --z_self; break;
    case NORTH: --y_self; break;
    case SOUTH: ++y_self; break;
    case EAST:  ++x_self; break;
    case WEST:  --x_self; break;
    case NOWHERE: Q_UNREACHABLE(); return;
    }
    emit Moved(dir);
    if ( dir > DOWN ) {
        Shred * const new_shred = GetWorld()->GetShred(X(), Y());
        const bool overstep = ( shred != new_shred );
        if ( overstep ) {
            shred->Unregister(this);
            new_shred->Register(this);
        }
    }
}

void Active::SendSignalAround(const QString signal) const {
    World * const world = GetWorld();
    static const int bound = world->GetBound();
    if ( X() > 0 )     world->GetBlock(X()-1, Y(), Z())->ReceiveSignal(signal);
    if ( X() < bound ) world->GetBlock(X()+1, Y(), Z())->ReceiveSignal(signal);
    if ( Y() > 0 )     world->GetBlock(X(), Y()-1, Z())->ReceiveSignal(signal);
    if ( Y() < bound ) world->GetBlock(X(), Y()+1, Z())->ReceiveSignal(signal);
    world->GetBlock(X(), Y(), Z()-1)->ReceiveSignal(signal);
    world->GetBlock(X(), Y(), Z()+1)->ReceiveSignal(signal);
}

void Active::DamageAround() const {
    World * const world = GetWorld();
    static const int bound = world->GetBound();
    int x_temp = X()-1;
    int y_temp = Y();
    int z_temp = Z();
    if (   x_temp     >= 0 )     TryDestroy(  x_temp, y_temp, z_temp);
    if (  (x_temp+=2) <= bound ) TryDestroy(  x_temp, y_temp, z_temp);
    if ( --y_temp     >= 0 )     TryDestroy(--x_temp, y_temp, z_temp);
    if (  (y_temp+=2) <= bound ) TryDestroy(  x_temp, y_temp, z_temp);
    TryDestroy(x_temp, --y_temp, --z_temp);
    TryDestroy(x_temp,   y_temp,   z_temp+=2);
}

void Active::TryDestroy(const int x, const int y, const int z) const {
    World * const world = GetWorld();
    if ( world->Damage(x, y, z, DamageLevel(), DamageKind()) <= 0 ) {
        world->DestroyAndReplace(x, y, z);
    }
}

void Active::SetShred(Shred * const new_shred) { shred = new_shred; }
Shred * Active::GetShred() const { return shred; }
World * Active::GetWorld() const { return world; }

void Active::Damage(const int dmg, const int dmg_kind) {
    const int last_dur = GetDurability();
    Block::Damage(dmg, dmg_kind);
    if ( last_dur != GetDurability() ) {
        ReceiveSignal(OUCH);
        switch ( dmg_kind ) {
        case HUNGER:      ReceiveSignal(tr("You faint from hunger!")); break;
        case HEAT:        ReceiveSignal(tr("You burn!"));              break;
        case BREATH:      ReceiveSignal(tr("You choke withot air!"));  break;
        case DAMAGE_FALL: ReceiveSignal(tr("You fall!"));              break;
        default:          ReceiveSignal(tr("Received damage!"));
        }
        emit Updated();
    }
}

void Active::ReceiveSignal(const QString str) { emit ReceivedText(str); }

void Active::ReloadToNorth() { y_self += SHRED_WIDTH; }
void Active::ReloadToSouth() { y_self -= SHRED_WIDTH; }
void Active::ReloadToWest()  { x_self += SHRED_WIDTH; }
void Active::ReloadToEast()  { x_self -= SHRED_WIDTH; }

void Active::Farewell() {
    ReceiveSignal(tr("You die."));
    emit Destroyed();
}

Active::Active(const int sub, const int id, const int transp) :
        Block(sub, id, transp),
        Xyz()
{}

Active::Active(QDataStream & str, const int sub, const int id,
        const int transp)
    :
        Block(str, sub, id, transp),
        Xyz()
{}

Active::~Active() { delete deferredAction; }

bool Active::Gravitate(const int range, int bottom, int top,
        const int calmness)
{
    static World * const world = GetWorld();
    static const int bound = world->GetBound();
    // analyse world around
    int for_north = 0, for_west = 0;
    const int y_start = qMax(Y()-range, 0);
    const int y_end   = qMin(Y()+range, bound);
    const int x_end   = qMin(X()+range, bound);
    bottom = qMax(Z()-bottom,  0);
    top    = qMin(Z()+top, HEIGHT-1);
    for (int x=qMax(X()-range, 0); x<=x_end; ++x)
    for (int y=y_start; y<=y_end; ++y) {
        Shred * const shred = world->GetShred(x, y);
        const int x_in = Shred::CoordInShred(x);
        const int y_in = Shred::CoordInShred(y);
        for (int z=bottom; z<=top; ++z) {
            const int attractive =
                Attractive(shred->GetBlock(x_in, y_in, z)->Sub());
            if ( attractive != 0
                    && world->DirectlyVisible(X(), Y(), Z(), x, y, z) )
            {
                if ( y!=Y() ) for_north += attractive/(Y()-y);
                if ( x!=X() ) for_west  += attractive/(X()-x);
            }
        }
    }
    // make direction and move there
    if ( qAbs(for_north)>calmness || qAbs(for_west)>calmness ) {
        SetDir( ( qAbs(for_north)>qAbs(for_west) ) ?
            ( ( for_north>0 ) ? NORTH : SOUTH ) :
            ( ( for_west >0 ) ? WEST  : EAST  ) );
        return true;
    } else {
        return false;
    }
}

int Active::Attractive(int) const { return 0; }

bool Active::IsSubAround(const int sub) const {
    const World * const world = GetWorld();
    static const int bound = world->GetBound();
    return (sub == world->GetBlock(X(), Y(), Z()-1)->Sub() ||
            sub == world->GetBlock(X(), Y(), Z()+1)->Sub() ||
            (X() > 0     && sub == world->GetBlock(X()-1, Y(), Z())->Sub()) ||
            (X() < bound && sub == world->GetBlock(X()+1, Y(), Z())->Sub()) ||
            (Y() > 0     && sub == world->GetBlock(X(), Y()-1, Z())->Sub()) ||
            (Y() < bound && sub == world->GetBlock(X(), Y()+1, Z())->Sub()) );
}

// Falling section

Falling::Falling(const int sub, const int id, const int transp) :
        Active(sub, id, transp),
        fallHeight(0)
{}

Falling::Falling(QDataStream & str, const int sub, const int id,
        const int transp)
    :
        Active(str, sub, id, transp)
{
    str >> fallHeight;
}

QString Falling::FullName() const {
    switch ( Sub() ) {
    case SAND:  return tr("Sand");
    case WATER: return tr("Snow");
    case STONE: return tr("Masonry");
    default:
        fprintf(stderr, "%s: Unlisted sub: %d.\n", Q_FUNC_INFO, Sub());
        return "Unkown active block";
    }
}

int  Falling::Kind() const { return FALLING; }
void Falling::SaveAttributes(QDataStream & out) const { out << fallHeight; }
bool Falling::IsFalling() const { return falling; }
Falling * Falling::ShouldFall() { return this; }
push_reaction Falling::PushResult(dirs) const { return MOVABLE; }

void Falling::FallDamage() {
    static const int SAFE_FALL_HEIGHT = 5;
    if ( fallHeight > SAFE_FALL_HEIGHT ) {
        const int dmg = (fallHeight - SAFE_FALL_HEIGHT)*10;
        GetWorld()->Damage(X(), Y(), Z()-1, dmg, DAMAGE_FALL);
        Damage(dmg, DAMAGE_FALL);
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
