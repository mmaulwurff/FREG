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

#include "blocks/Illuminator.h"
#include "BlockFactory.h"
#include "blocks/Inventory.h"
#include "World.h"

Illuminator::Illuminator(const int kind, const int sub) :
        Active(kind, sub, NONSTANDARD),
        fuelLevel(World::SECONDS_IN_DAY),
        isOn(false)
{}

Illuminator::Illuminator(QDataStream & str, const int kind, const int sub) :
        Active(str, kind, sub, NONSTANDARD),
        fuelLevel(),
        isOn()
{
    str >> fuelLevel >> isOn;
}

int  Illuminator::DamageKind() const {
    return (Sub()==GLASS) ? DAMAGE_NO : DAMAGE_HEAT;
}

QString Illuminator::FullName() const {
    switch ( Sub() ) {
    case WOOD:  return tr("Torch, fuel: %1"        ).arg(fuelLevel);
    case STONE: return tr("Flint, charges: %1"     ).arg(fuelLevel);
    case GLASS: return tr("Flashlight, battery: %1").arg(fuelLevel);
    default:    return tr("Lantern (%1), fuel: %2" ).
        arg(TrManager::SubName(Sub())).arg(fuelLevel);
    }
}

Block * Illuminator::DropAfterDamage(bool * const delete_block) {
    *delete_block = false;
    Block * const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
    pile->HasInventory()->Get(this);
    return pile;
}

usage_types Illuminator::Use(Active * const user) {
    if ( Sub() == WOOD || Sub() == STONE ) {
        fuelLevel -= ( fuelLevel >= 10 ) ? 10 : 0;
        if ( Sub() == STONE ) {
            return USAGE_TYPE_SET_FIRE;
        }
    }
    isOn = not isOn;
    user->UpdateLightRadius();
    return USAGE_TYPE_INNER;
}

int Illuminator::LightRadius() const {
    if ( fuelLevel == 0 || not isOn ) return 0;
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return 4;
    case IRON:  return 5;
    case GLASS: return 7;
    }
}

int  Illuminator::ShouldAct() const { return FREQUENT_RARE; }
wearable Illuminator::Wearable() const { return WEARABLE_OTHER; }

void Illuminator::DoRareAction() {
    World::GetWorld()->Shine(X(), Y(), Z(), LightRadius());
}

inner_actions Illuminator::ActInner() {
    if ( fuelLevel == 0 ) {
        if ( Sub() == WOOD ) {
            Break();
        }
    } else {
        if ( Sub() != STONE && isOn ) {
            --fuelLevel;
        }
    }
    return INNER_ACTION_NONE;
}

void Illuminator::SaveAttributes(QDataStream & out) const {
    out << fuelLevel << isOn;
}
