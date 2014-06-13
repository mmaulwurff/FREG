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

#include "Dwarf.h"
#include "world.h"
#include "BlockManager.h"
#include <QDataStream>

int  Dwarf::GetActiveHand() const { return activeHand; }
void Dwarf::SetActiveHand(const bool right) {
    activeHand = (right ? IN_RIGHT : IN_LEFT);
}

int Dwarf::Weight() const {
    World * const world = GetWorld();
    static const int bound = world->NumShreds() * SHRED_WIDTH - 1;
    return ( (X() < bound && world->GetBlock(X()+1, Y(), Z())->Catchable()) ||
            ( X() > 0     && world->GetBlock(X()-1, Y(), Z())->Catchable()) ||
            ( Y() < bound && world->GetBlock(X(), Y()+1, Z())->Catchable()) ||
            ( Y() > 0     && world->GetBlock(X(), Y()-1, Z())->Catchable()) ) ?
        0 : Inventory::Weight()+Block::Weight();
}

Block * Dwarf::DropAfterDamage(bool * const delete_block) {
    Block * const pile = block_manager.NewBlock(CONTAINER, DIFFERENT);
    Inventory * const pile_inv = pile->HasInventory();
    pile_inv->Get(block_manager.NormalBlock(H_MEAT));
    pile_inv->Get(Animal::DropAfterDamage(delete_block));
    pile_inv->Get(block_manager.NewBlock(WEAPON, BONE));
    return pile;
}

int Dwarf::Sub() const { return Block::Sub(); }
int Dwarf::ShouldAct() const { return FREQUENT_FIRST | FREQUENT_RARE; }
bool Dwarf::Access() const { return false; }
int Dwarf::Start() const { return ON_LEGS+1; }
int Dwarf::Kind() const { return DWARF; }
QString Dwarf::FullName() const { return "Rational"; }
Inventory * Dwarf::HasInventory() { return Inventory::HasInventory(); }
int Dwarf::LightRadius() const { return lightRadius; }

void Dwarf::UpdateLightRadius() {
    Block * const in_left  = ShowBlock(IN_LEFT);
    Block * const in_right = ShowBlock(IN_RIGHT);
    const int left_rad  = in_left  ? in_left ->LightRadius() : 0;
    const int right_rad = in_right ? in_right->LightRadius() : 0;
    lightRadius = qMax(MIN_DWARF_LIGHT_RADIUS, qMax(left_rad, right_rad));
}

void Dwarf::ReceiveSignal(const QString str) { Active::ReceiveSignal(str); }

int Dwarf::DamageKind() const {
    return ( Number(GetActiveHand()) ) ?
        ShowBlock(GetActiveHand())->DamageKind() : DAMAGE_HANDS;
}

int Dwarf::DamageLevel() const {
    int level = 1;
    if ( Number(IN_RIGHT) ) {
        level += ShowBlock(IN_RIGHT)->DamageLevel();
    }
    if ( Number(IN_LEFT) ) {
        level += ShowBlock(IN_LEFT)->DamageLevel();
    }
    return level;
}

bool Dwarf::Move(const int dir) {
    const bool overstepped = Active::Move(dir);
    if ( overstepped ) {
        for (int i=0; i<ON_LEGS; ++i) {
            Block * const block = ShowBlock(i);
            if ( block && block->Kind()==MAP ) {
                block->Use(this);
            }
        }
    }
    return overstepped;
}

int Dwarf::NutritionalValue(const int sub) const {
    switch ( sub ) {
    case HAZELNUT: return SECONDS_IN_HOUR/2;
    case H_MEAT:   return SECONDS_IN_HOUR*2.5;
    case A_MEAT:   return SECONDS_IN_HOUR*2;
    }
    return 0;
}

void Dwarf::MoveInside(const int num_from, const int num_to, const int num) {
    Block * const block = ShowBlock(num_from);
    if ( block && (num_to > ON_LEGS ||
            IN_RIGHT==num_to || IN_LEFT==num_to ||
            ( ON_HEAD==num_to &&
                WEARABLE_HEAD==block->Wearable() ) ||
            ( ON_BODY==num_to &&
                WEARABLE_BODY==block->Wearable() ) ||
            ( ON_LEGS==num_to &&
                WEARABLE_LEGS==block->Wearable() )) )
    {
        Inventory::MoveInside(num_from, num_to, num);
    }
    UpdateLightRadius();
    GetWorld()->Shine(X(), Y(), Z(), lightRadius, true);
}

void Dwarf::SaveAttributes(QDataStream & out) const {
    Animal::SaveAttributes(out);
    Inventory::SaveAttributes(out);
    out << activeHand;
}

bool Dwarf::Inscribe(const QString) {
    SendSignalAround(tr("Don't touch me!"));
    return false;
}

Dwarf::Dwarf(const int sub, const int id) :
        Animal(sub, id),
        Inventory(),
        activeHand(IN_RIGHT),
        lightRadius(MIN_DWARF_LIGHT_RADIUS)
{
    note = new QString("Urist");
}
Dwarf::Dwarf(QDataStream & str, const int sub, const int id) :
        Animal(str, sub, id),
        Inventory(str)
{
    str >> activeHand;
    UpdateLightRadius();
}
