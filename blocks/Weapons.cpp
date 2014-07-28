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
        case WOOD:  return QObject::tr("Stick");
        case BONE:  return QObject::tr("Bone");
        case SKY:   return QObject::tr("Air");
        default:
            fprintf(stderr, "Weapon::FullName: unlisted sub: %d\n", Sub());
            return "Some weapon";
        }
    }

    int  Weapon::Kind() const { return WEAPON; }
    int  Weapon::Weight() const { return Block::Weight()/4; }
    int  Weapon::Wearable() const { return WEARABLE_ARM; }
    push_reaction Weapon::PushResult(dirs) const { return DAMAGE; }

    void Weapon::Damage(int, const int dmg_kind) {
        if ( dmg_kind < DAMAGE_PUSH_UP ) {
            Break();
        }
    }

    usage_types Weapon::Use(Block * const who) {
        Shred * const shred = GetShred();
        if ( shred != nullptr ) { // not currently in inventory
            Inventory * const inv = who->HasInventory();
            if ( inv != nullptr && inv->Get(this) ) {
                Unregister();
                shred->SetBlockNoCheck(block_manager.Normal(AIR),
                    Shred::CoordInShred(X()), Shred::CoordInShred(Y()), Z());
            }
        }
        return USAGE_TYPE_NO;
    }

    int Weapon::DamageLevel() const {
        switch ( Sub() ) {
        case WOOD:  return 4;
        case IRON:  return 6;
        case STONE: return 5;
        case SKY:   return MAX_DURABILITY;
        default:
            fprintf(stderr, "Weapon::DamageLevel: sub (?): %d\n.", Sub());
            return 1;
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
    int Pick::Kind() const { return PICK; }
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
        switch ( Sub() ) {
        case IRON:  return QObject::tr("Iron pick");
        case STEEL: return QObject::tr("Steel pick");
        case ADAMANTINE: return QObject::tr("Adamantine pick");
        default:
            fprintf(stderr, "Pick::FullName: unknown sub: %d\n", Sub());
            return "Strange pick";
        }
    }

// Shovel::
    int Shovel::Kind() const { return SHOVEL; }
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
        switch ( Sub() ) {
        case BONE:  return QObject::tr("Bone shovel");
        case IRON:  return QObject::tr("Iron shovel");
        case STEEL: return QObject::tr("Steel shovel");
        case ADAMANTINE: return QObject::tr("Adamantine shovel");
        default:
            fprintf(stderr, "Shovel::FullName: unknown sub: %d\n", Sub());
            return "Strange shovel";
        }
    }

// Hammer::
    int Hammer::Kind() const { return HAMMER; }
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
        switch ( Sub() ) {
        case WOOD:  return QObject::tr("Club");
        case STONE: return QObject::tr("Stone hammer");
        case IRON:  return QObject::tr("Iron hammer");
        case STEEL: return QObject::tr("Steel hammer");
        case ADAMANTINE: return QObject::tr("Adamantine hammer");
        default:
            fprintf(stderr, "Hammer::FullName: unknown sub: %d\n", Sub());
            return "Strange hammer";
        }
    }

// Axe::
    int Axe::Kind() const { return AXE; }
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
        switch ( Sub() ) {
        case BONE:  return QObject::tr("Bone axe");
        case STONE: return QObject::tr("Stone axe");
        case IRON:  return QObject::tr("Iron axe");
        case STEEL:  return QObject::tr("Steel axe");
        case ADAMANTINE: return QObject::tr("Adamantine axe");
        default:
            fprintf(stderr, "Axe::FullName: unknown sub: %d\n", Sub());
            return "Strange axe";
        }
    }
