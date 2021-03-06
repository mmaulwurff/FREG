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

#include "blocks/Weapons.h"
#include "TrManager.h"
#include <QObject>

CONSTRUCT_AS_PARENT(Weapon, Falling)

QString Weapon::FullName() const {
    if ( Kind() != WEAPON ) return Block::FullName();

    TrString stoneName = QObject::tr("Pebble");
    TrString ironName  = QObject::tr("Spike");
    switch ( Sub() ) {
    case STONE: return stoneName;
    case IRON:  return ironName;
    case BONE:
    case SKY:
    case SUB_NUT: return TrManager::SubNameUpper(Sub());
    default:      return Block::FullName();
    }
}

QString Weapon::Description() const {
    TrString description = QObject::tr("Damage: %1. Damage type: %2.");
    return description
        .arg(DamageLevel())
        .arg(TrManager::GetDamageString(
            static_cast<damage_kinds>(DamageKind())));
}

int  Weapon::Weight() const { return Block::Weight() / 4; }
wearable Weapon::Wearable() const { return WEARABLE_OTHER; }
push_reaction Weapon::PushResult(dirs) const { return DAMAGE; }

int Weapon::DamageLevel() const {
    switch ( Sub() ) {
    default:    return  1;
    case WOOD:  return  4;
    case BONE:  return  5;
    case STONE: return  7;
    case IRON:  return 10;
    case STEEL: return 12;
    case SKY:   return MAX_DURABILITY;
    case ADAMANTINE: return 20;
    }
}

int  Weapon::DamageKind() const {
    switch ( Sub() ) {
    case IRON: return DAMAGE_THRUST;
    case SKY:  return DAMAGE_TIME;
    default:   return DAMAGE_CRUSH;
    }
}

CONSTRUCT_AS_PARENT(Pick  , Weapon)
CONSTRUCT_AS_PARENT(Shovel, Weapon)
CONSTRUCT_AS_PARENT(Hammer, Weapon)
CONSTRUCT_AS_PARENT(Axe   , Weapon)

int Pick  ::DamageKind() const { return DAMAGE_MINE;  }
int Shovel::DamageKind() const { return DAMAGE_DIG;   }
int Hammer::DamageKind() const { return DAMAGE_CRUSH; }
int Axe   ::DamageKind() const { return DAMAGE_CUT;   }
