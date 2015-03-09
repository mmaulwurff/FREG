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
#include "blocks/Inventory.h"
#include "World.h"
#include "TrManager.h"
#include "Shred.h"
#include <QDataStream>

const int Illuminator::MAX_FUEL_LEVEL = World::SECONDS_IN_DAY;
const int Illuminator::TORCH_FULL_LEVEL = World::MAX_LIGHT_RADIUS - 2;
const int Illuminator::TORCH_END_LEVEL  = World::MAX_LIGHT_RADIUS - 3;

Illuminator::Illuminator(const kinds kind, const subs sub) :
        Active(kind, sub),
        fuelLevel(MAX_FUEL_LEVEL),
        isOn(false)
{}

Illuminator::Illuminator(QDataStream& str, const kinds kind, const subs sub) :
        Active(str, kind, sub),
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
    default:    return tr("%1 (%2), fuel: %3" ).
        arg(TrManager::KindName(Kind())).
        arg(TrManager::SubName(Sub())).
        arg(fuelLevel);
    }
}

Block* Illuminator::DropAfterDamage(bool* const delete_block) {
    return DropInto(delete_block);
}

usage_types Illuminator::Use(Active* /* user */) {
    if ( Sub() == WOOD || Sub() == STONE ) {
        fuelLevel -= ( fuelLevel >= 10 ) ? 10 : 0;
        if ( Sub() == STONE ) {
            return USAGE_TYPE_SET_FIRE;
        }
    }
    const int old_radius = Illuminator::LightRadius();
    isOn = not isOn;
    UpdateLightRadius(old_radius);
    return USAGE_TYPE_INNER;
}

int Illuminator::LightRadius() const {
    if ( fuelLevel == 0 || not isOn ) return 0;
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return ( fuelLevel > MAX_FUEL_LEVEL/4 ) ?
                    TORCH_FULL_LEVEL : TORCH_END_LEVEL;
    case IRON:  return World::MAX_LIGHT_RADIUS - 1;
    case GLASS: return World::MAX_LIGHT_RADIUS;
    }
}

wearable Illuminator::Wearable() const { return WEARABLE_OTHER; }

inner_actions Illuminator::ActInner() {
    if ( fuelLevel != 0 ) {
        if ( isOn && Sub()!=STONE ) {
            --fuelLevel;
            if ( Sub()==WOOD && fuelLevel==MAX_FUEL_LEVEL/4 ) {
                UpdateLightRadius(TORCH_FULL_LEVEL);
            }
        }
    } else {
        if ( Sub() == WOOD ) {
            Break();
        }
    }
    return INNER_ACTION_ONLY;
}

void Illuminator::SaveAttributes(QDataStream& out) const {
    out << fuelLevel << isOn;
}
