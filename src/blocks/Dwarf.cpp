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

#include "blocks/Dwarf.h"
#include "blocks/Weapons.h"

#include "World.h"
#include "Shred.h"
#include "AroundCoordinates.h"

int Dwarf::Weight() const {
    if ( Sub() == DIFFERENT ) return 0;
    if (GetShred() == nullptr) return Inventory::Weight() + Block::Weight();
    const AroundCoordinates4 around(X(), Y(), Z());
    return std::any_of(ALL(around), [](const XyzInt& xyz) {
            return World::GetCWorld()->GetBlock(XYZ(xyz))->Catchable();
        }) ?
        0 : Inventory::Weight() + Block::Weight();
}

Block* Dwarf::DropAfterDamage(bool* const delete_block) {
    Block* const cadaver = Animal::DropAfterDamage(delete_block);
    cadaver->HasInventory()->Get(new Weapon(BONE));
    return cadaver;
}

int  Dwarf::Start() const { return SPECIAL_SLOTS_COUNT; }
bool Dwarf::Access() const { return false; }
int  Dwarf::ShouldAct() const { return FREQUENT_FIRST | FREQUENT_RARE; }
int  Dwarf::LightRadius() const { return lightRadius; }
void Dwarf::ReceiveSignal(const QString& str) { Active::ReceiveSignal(str); }
bool Dwarf::IsCreator() const { return DIFFERENT == Sub(); }

bool Dwarf::Drop(const int src, const int dest, const int num,
    Inventory *const to)
{
    Updated();
    return Inventory::Drop(src, dest, num, to);
}

inner_actions Dwarf::ActInner() {
    const int old_radius = lightRadius;
    lightRadius = UpdateLightRadiusInner();
    UpdateLightRadius(old_radius);
    return Animal::ActInner();
}

QString Dwarf::FullName() const {
    TrString creatorString = QObject::tr("Creator");
    return (IsCreator() ?
        creatorString : Animal::FullName());
}

/// @todo Make all (switching lanterns on/off) light changes synchronous
/// to avoid [un]shining in wrong place.
int Dwarf::UpdateLightRadiusInner() const {
    const Block* const in_left  = ShowBlock(IN_LEFT );
    const Block* const in_right = ShowBlock(IN_RIGHT);
    return std::max(
        (in_left  ? in_left ->LightRadius() : 0),
                (in_right ? in_right->LightRadius() : 0) );
}

int Dwarf::DamageKind() const {
    return IsCreator() ?
        DAMAGE_TIME :
        (IsEmpty(IN_RIGHT) ? DAMAGE_CRUSH : ShowBlock(IN_RIGHT)->DamageKind())|
        (IsEmpty(IN_LEFT)  ? DAMAGE_CRUSH : ShowBlock(IN_LEFT )->DamageKind());
}

int Dwarf::DamageLevel() const {
    if ( IsCreator() ) return MAX_DURABILITY;
    int level = 1;
    if ( not IsEmpty(IN_RIGHT) ) {
        level += ShowBlock(IN_RIGHT)->DamageLevel();
    }
    if ( not IsEmpty(IN_LEFT) ) {
        level += ShowBlock(IN_LEFT)->DamageLevel();
    }
    return level;
}

void Dwarf::Damage(const int dmg, const int dmg_kind) {
    if ( dmg_kind >= DAMAGE_PUSH_UP ) {
        Animal::Damage(dmg, DAMAGE_PUSH_UP);
        return;
    }
    int damage_to_self = dmg;
    const int places[] = { ON_HEAD, ON_BODY, ON_LEGS };
    for (const int i : places) {
        if ( IsEmpty(i) ) continue;
        Block* const armour = ShowBlock(i);
        const int durability_before_damage = armour->GetDurability();
        const int damage_divider = (i == ON_BODY) ? 2 : 4;
        armour->Damage(dmg/damage_divider, dmg_kind);
        if ( armour->GetDurability() < durability_before_damage ) {
            damage_to_self -= dmg/damage_divider;
            if ( armour->GetDurability() <= 0 ) {
                delete armour;
                Pull(i);
            }
        }
    }
    Animal::Damage(damage_to_self, dmg_kind);
}

void Dwarf::Move(const dirs dir) {
    Shred* const last_shred = GetShred();
    Falling::Move(dir);
    if ( last_shred != GetShred() ) {
        for (Block* const block : *this) {
            block->UseOnShredMove(this);
        }
    }
}

int Dwarf::NutritionalValue(const subs sub) const {
    switch ( sub ) {
    case GREENERY: return World::SECONDS_IN_HOUR/20;
    case SUB_NUT:  return World::SECONDS_IN_HOUR/2;
    case H_MEAT:   return World::SECONDS_IN_HOUR*2.5f;
    case A_MEAT:   return World::SECONDS_IN_HOUR*2;
    default:       return 0;
    }
}

bool Dwarf::GetExact(Block* const block, const int to) {
    return ( block == nullptr ) ||
        ( (to >= SPECIAL_SLOTS_COUNT || ( IsEmpty(to) && (
                     IN_RIGHT==to
                ||   IN_LEFT ==to
                || ( ON_HEAD ==to && WEARABLE_HEAD==block->Wearable() )
                || ( ON_BODY ==to && WEARABLE_BODY==block->Wearable() )
                || ( ON_LEGS ==to && WEARABLE_LEGS==block->Wearable() ))))
            && Inventory::GetExact(block, to) );
}

void Dwarf::SaveAttributes(QDataStream& out) const {
    Animal::SaveAttributes(out);
    Inventory::SaveAttributes(out);
}

bool Dwarf::Inscribe(const QString&) {
    TrString notTouchString = QObject::tr("Don't touch me!");
    SendSignalAround(notTouchString);
    return false;
}

QString Dwarf::InvFullName(const int slot_number) const {
    static const QString invFullNames[] = {
        QObject::tr("-head-"),
        QObject::tr("-body-"),
        QObject::tr("-legs-"),
        QObject::tr("-right hand-"),
        QObject::tr("-left  hand-")
    };
    return ( IsEmpty(slot_number) && slot_number < SPECIAL_SLOTS_COUNT ) ?
        invFullNames[slot_number] :
        Inventory::InvFullName(slot_number);
}

void Dwarf::Pull(const int slot) {
    Inventory::Pull(slot);
    const int old_radius = lightRadius;
    lightRadius = UpdateLightRadiusInner();
    UpdateLightRadius(old_radius);
}

bool Dwarf::Get(Block* const block, const int start) {
    const bool result = Inventory::Get(block, start);
    const int old_radius = lightRadius;
    lightRadius = UpdateLightRadiusInner();
    UpdateLightRadius(old_radius);
    return result;
}

Dwarf::Dwarf(const kinds kind, const subs sub)
    : Animal(kind, sub)
    , Inventory()
    , lightRadius(0)
{}

Dwarf::Dwarf(QDataStream& str, const kinds kind, const subs sub)
    : Animal(str, kind, sub)
    , Inventory(str)
    , lightRadius(UpdateLightRadiusInner())
{}
