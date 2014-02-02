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

// Weapon::
    QString Weapon::FullName() const {
        switch ( Sub() ) {
        case STONE: return QObject::tr("Pebble");
        case IRON:  return QObject::tr("Spike");
        case WOOD:  return QObject::tr("Stick");
        default:
            fprintf(stderr, "Weapon::FullName: unlisted sub: %d\n",
                Sub());
            return "Some weapon";
        }
    }

    quint8 Weapon::Kind() const { return WEAPON; }
    ushort Weapon::Weight() const { return Block::Weight()/4; }
    int    Weapon::Wearable() const { return WEARABLE_ARM; }

    ushort Weapon::DamageLevel() const {
        switch ( Sub() ) {
        case WOOD: return 4;
        case IRON: return 6;
        case STONE: return 5;
        default:
            fprintf(stderr, "Weapon::DamageLevel: sub (?): %d\n.",
                Sub());
            return 1;
        }
    }

    int  Weapon::DamageKind() const {
        return ( IRON==Sub() ) ? THRUST : CRUSH;
    }

    void Weapon::Push(const int, Block * const who) {
        who->Damage(DamageLevel(), DamageKind());
    }

    Weapon::Weapon(const int sub, const quint16 id) :
            Block(sub, id, NONSTANDARD)
    {}
    Weapon::Weapon(QDataStream & str, const int sub, const quint16 id) :
            Block(str, sub, id, NONSTANDARD)
    {}
// Pick::
    quint8 Pick::Kind() const { return PICK; }
    int Pick::DamageKind() const { return MINE; }

    ushort Pick::DamageLevel() const {
        switch ( Sub() ) {
        case IRON: return 10;
        default:
            fprintf(stderr, "Pick::DamageLevel: sub (?): %d\n.",
                Sub());
            return 1;
        }
    }

    QString Pick::FullName() const {
        switch ( Sub() ) {
        case IRON: return QObject::tr("Iron pick");
        default:
            fprintf(stderr, "Pick::FullName: unknown sub: %d\n",
                Sub());
            return "Strange pick";
        }
    }

    Pick::Pick(const int sub, const quint16 id) :
            Weapon(sub, id)
    {}
    Pick::Pick(QDataStream & str, const int sub, const quint16 id) :
            Weapon(str, sub, id)
    {}
