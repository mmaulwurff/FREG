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

QString Active::FullName() const {
    switch ( Sub() ) {
    case SAND:  return tr("Sand");
    case WATER: return tr("Snow");
    case STONE: return tr("Masonry");
    default:
        fprintf(stderr, "Active::FullName: Unlisted sub: %d\n", Sub());
        return "Unkown active block";
    }
}
quint8 Active::Kind() const { return ACTIVE; }
Active * Active::ActiveBlock() { return this; }
bool Active::IsFalling() const { return falling; }
bool Active::ShouldFall() const { return true; }
int  Active::ShouldAct() const { return FREQUENT_NEVER; }
int  Active::PushResult(int) const { return MOVABLE; }
void Active::DoFrequentAction() {}
void Active::DoRareAction() {}
INNER_ACTIONS Active::ActInner() { return INNER_ACTION_NONE; }

void Active::ActFrequent() {
    if ( not IsToDelete() ) {
        if ( GetDeferredAction() != nullptr ) {
            GetDeferredAction()->MakeAction();
        }
        DoFrequentAction();
    }
}

void Active::ActRare() {
    if ( IsToDelete() ) return;
    DoRareAction();
    Inventory * const inv = HasInventory();
    if ( inv != nullptr ) {
        for (int i=0; i<inv->Size(); ++i)
        for (int j=0; j<inv->Number(i); ++j) {
            Active * const active = inv->ShowBlock(i, j)->ActiveBlock();
            if ( active != nullptr
                    && active->ActInner()==INNER_ACTION_MESSAGE )
            {
                ReceiveSignal(QString("Item in slot '%1' changed status.").
                    arg(char('a'+i)));
                ReceiveSignal(inv->ShowBlock(i, j)->GetNote());
            }
        }
    }
}

void Active::SetFalling(const bool set) {
    if ( not (falling=set) ) {
        fall_height = 0;
    }
}

void Active::SetDeferredAction(DeferredAction * const action) {
    delete deferredAction;
    deferredAction = action;
}

DeferredAction * Active::GetDeferredAction() const { return deferredAction; }

void Active::FallDamage() {
    if ( fall_height > SAFE_FALL_HEIGHT ) {
        const int dmg = (fall_height - SAFE_FALL_HEIGHT)*10;
        GetWorld()->Damage(X(), Y(), Z()-1, dmg, DAMAGE_FALL);
        Damage(dmg, DAMAGE_FALL);
    }
    fall_height = 0;
}

bool Active::Move(const int dir) {
    switch ( dir ) {
    case NORTH: --y_self; break;
    case SOUTH: ++y_self; break;
    case EAST:  ++x_self; break;
    case WEST:  --x_self; break;
    case UP:    ++z_self; break;
    }
    bool overstep;
    if ( DOWN == dir ) {
        --z_self;
        ++fall_height;
        overstep = false;
    } else {
        Shred * const new_shred = GetWorld()->GetShred(X(), Y());
        if ( (overstep = ( shred != new_shred )) ) {
            falling = false;
            shred->UnregisterLater(this);
            new_shred->Register(this);
        }
    }
    emit Moved(dir);
    return overstep;
}

void Active::SendSignalAround(const QString signal) const {
    World * const world = GetWorld();
    const Xy coords[] = {
        Xy( X()-1, Y()   ),
        Xy( X()+1, Y()   ),
        Xy( X(),   Y()-1 ),
        Xy( X(),   Y()+1 ) };
    for (const Xy xy : coords) {
        if ( world->InBounds(xy.X(), xy.Y()) ) {
             world->GetBlock(xy.X(), xy.Y(), Z())->ReceiveSignal(signal);
        }
    }
    world->GetBlock(X(), Y(), Z()-1)->ReceiveSignal(signal);
    world->GetBlock(X(), Y(), Z()+1)->ReceiveSignal(signal);
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

void Active::EmitUpdated() { emit Updated(); }

void Active::SaveAttributes(QDataStream & out) const { out << fall_height; }

void Active::SetToDelete() {
    if ( not frozen ) {
        frozen = true;
        GetShred()->AddToDelete(this);
        ReceiveSignal(tr("You die."));
        emit Destroyed();
    }
}

bool Active::IsToDelete() const { return frozen; }

Active::Active(const int sub, const quint16 id, const quint8 transp) :
        Block(sub, id, transp),
        Xyz(),
        fall_height(0),
        falling(false),
        frozen(false),
        deferredAction(nullptr),
        shred()
{}

Active::Active(QDataStream & str, const int sub, const quint16 id,
        const quint8 transp)
    :
        Block(str, sub, id, transp),
        Xyz(),
        falling(false),
        frozen(false),
        deferredAction(nullptr),
        shred()
{
    str >> fall_height;
}
Active::~Active() { delete deferredAction; }

bool Active::Gravitate(const int range, const int down, const int up,
        const int calmness)
{
    World * const world = GetWorld();
    static const int bound = SHRED_WIDTH * world->NumShreds() - 1;
    // analyse world around
    int for_north = 0, for_west = 0;
    const int y_start = qMax(Y()-range, 0);
    const int z_start = qMax(Z()-down,  0);
    const int x_end = qMin(X()+range, bound);
    const int y_end = qMin(Y()+range, bound);
    const int z_end = qMin(Z()+up, HEIGHT-1);
    for (int x=qMax(X()-range, 0); x<=x_end; ++x)
    for (int y=y_start; y<=y_end; ++y) {
        Shred * const shred = world->GetShred(x, y);
        const int x_in = Shred::CoordInShred(x);
        const int y_in = Shred::CoordInShred(y);
        for (int z=z_start; z<=z_end; ++z) {
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

bool Active::IsSubAround(const quint8 sub) const {
    const World * const world = GetWorld();
    return (
            sub == world->GetBlock(X(), Y(), Z()-1)->Sub() ||
            sub == world->GetBlock(X(), Y(), Z()+1)->Sub() ||
            (world->InBounds(X()-1, Y()) &&
            sub == world->GetBlock(X()-1, Y(), Z())->Sub()) ||
            (world->InBounds(X()+1, Y()) &&
            sub==world->GetBlock(X()+1, Y(), Z())->Sub()) ||
            (world->InBounds(X(), Y()-1) &&
            sub==world->GetBlock(X(), Y()-1, Z())->Sub()) ||
            (world->InBounds(X(), Y()+1) &&
            sub==world->GetBlock(X(), Y()+1, Z())->Sub()) );
}
