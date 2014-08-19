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

#include "blocks/Weapons.h"
#include "blocks/Inventory.h"
#include "BlockManager.h"
#include "Shred.h"

// Weapon::
    QString Weapon::FullName() const {
        switch ( Sub() ) {
        case STONE: return QObject::tr("Pebble");
        case IRON:  return QObject::tr("Spike");
        case BONE:  return QObject::tr("Bone");
        case SKY:   return QObject::tr("Air");
        default:    return QObject::tr("Stick (%1)").arg(SubName(Sub()));
        }
    }

    int  Weapon::Weight() const { return Block::Weight()/4; }
    wearable Weapon::Wearable() const { return WEARABLE_OTHER; }
    push_reaction Weapon::PushResult(dirs) const { return DAMAGE; }

    int Weapon::DamageLevel() const {
        switch ( Sub() ) {
        case WOOD:  return 4;
        case IRON:  return 6;
        case STONE: return 5;
        default:    return 1;
        }
    }

    int  Weapon::DamageKind() const {
        switch ( Sub() ) {
        case IRON: return DAMAGE_THRUST;
        case SKY:  return DAMAGE_TIME;
        default:   return DAMAGE_CRUSH;
        }
    }

// Pick::
    int Pick::DamageKind() const { return DAMAGE_MINE; }

    int Pick::DamageLevel() const {
        switch ( Sub() ) {
        case IRON: return 10;
        default:
            fprintf(stderr, "Pick::DamageLevel: sub (?): %d\n.", Sub());
            return 1;
        }
    }

    QString Pick::FullName() const {
        return QObject::tr("Pick (%1)").arg(SubName(Sub()));
    }

// Shovel::
    int Shovel::DamageKind() const { return DAMAGE_DIG; }

    int Shovel::DamageLevel() const {
        switch ( Sub() ) {
        case IRON: return 3;
        default:
            fprintf(stderr, "Shovel::DamageLevel: sub (?): %d\n.", Sub());
            return 1;
        }
    }

    QString Shovel::FullName() const {
        return QObject::tr("Shovel (%1)").arg(SubName(Sub()));
    }

// Hammer::
    int Hammer::DamageKind() const { return DAMAGE_CRUSH; }

    int Hammer::DamageLevel() const {
        switch ( Sub() ) {
        case IRON: return 10;
        default:
            fprintf(stderr, "Hammer::DamageLevel: sub (?): %d\n.", Sub());
            return 1;
        }
    }

    QString Hammer::FullName() const {
        return QObject::tr("Hammer (%1)").arg(SubName(Sub()));
    }

// Axe::
    int Axe::DamageKind() const { return DAMAGE_CUT; }

    int Axe::DamageLevel() const {
        switch ( Sub() ) {
        case IRON: return 10;
        default:
            fprintf(stderr, "Axe::DamageLevel: sub (?): %d\n.", Sub());
            return 1;
        }
    }

    QString Axe::FullName() const {
        return QObject::tr("Axe (%1)").arg(SubName(Sub()));
    }
