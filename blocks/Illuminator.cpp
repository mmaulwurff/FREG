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
#include "TrManager.h"
#include "Shred.h"
#include <QDataStream>

Illuminator::Illuminator(const kinds kind, const subs sub) :
        Active(kind, sub),
        fuelLevel(World::SECONDS_IN_DAY),
        isOn(false)
{}

Illuminator::Illuminator(QDataStream & str, const kinds kind, const subs sub) :
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
    default:    return tr("Lantern (%1), fuel: %2" ).
        arg(TrManager::SubName(Sub())).arg(fuelLevel);
    }
}

Block* Illuminator::DropAfterDamage(bool * const delete_block) {
    *delete_block = false;
    Block* const pile = BlockFactory::NewBlock(BOX, DIFFERENT);
    pile->HasInventory()->Get(this);
    return pile;
}

usage_types Illuminator::Use(Active * /* user */) {
    if ( Sub() == WOOD || Sub() == STONE ) {
        fuelLevel -= ( fuelLevel >= 10 ) ? 10 : 0;
        if ( Sub() == STONE ) {
            return USAGE_TYPE_SET_FIRE;
        }
    }
    if ( IsInside() ) {
        isOn = not isOn;
    } else {
        const Xyz xyz = GetXyz();
        int radius = Illuminator::LightRadius();
        if ( radius ) {
            World::GetWorld()->Shine(xyz, -Illuminator::LightRadius());
            GetShred()->RemShining(this);
        }
        isOn = not isOn; // can change light radius
        if ( (radius = Illuminator::LightRadius()) ) {
            World::GetWorld()->Shine(xyz, radius);
            GetShred()->AddShining(this);
        }
    }
    return USAGE_TYPE_INNER;
}

int Illuminator::LightRadius() const {
    if ( fuelLevel == 0 || not isOn ) return 0;
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return World::MAX_LIGHT_RADIUS - 1;
    case IRON:  return World::MAX_LIGHT_RADIUS - 1;
    case GLASS: return World::MAX_LIGHT_RADIUS;
    }
}

int  Illuminator::ShouldAct() const { return FREQUENT_RARE; }
wearable Illuminator::Wearable() const { return WEARABLE_OTHER; }

void Illuminator::DoRareAction() {
    /// \todo decrease torch light radius when fuel is low
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
