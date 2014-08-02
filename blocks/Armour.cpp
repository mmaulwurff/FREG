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

#include "blocks/Armour.h"

const int TRESHOLD = 10;

// Armour section
    void Armour::Damage(const int dmg, const int dmg_kind) {
        if ( dmg_kind >= DAMAGE_PUSH_UP ) {
            Break();
        }
        if ( dmg > TRESHOLD ) {
            Block::Damage(dmg/((Kind() == ARMOUR) ? 4 : 2), dmg_kind);
        }
    }

    int Armour::Wearable() const { return WEARABLE_BODY; }
    int Armour::DamageLevel() const { return 0; }

    QString Armour::FullName() const {
        switch ( Sub() ) {
        case IRON:  return QObject::tr("Iron body armour");
        case STEEL: return QObject::tr("Steel body armour");
        default: fprintf(stderr, "%s: sub (?) %d.\n", Q_FUNC_INFO, Sub());
             return "Unknown armour";
        }
    }

// Helmet section
    int Helmet::Wearable() const { return WEARABLE_HEAD; }

    QString Helmet::FullName() const {
        switch ( Sub() ) {
        case IRON:  return QObject::tr("Iron helmet");
        case STEEL: return QObject::tr("Steel helmet");
        default: fprintf(stderr, "%s: sub (?) %d.\n", Q_FUNC_INFO, Sub());
             return "Unknown helmet";
        }
    }

// Boots section
    int Boots::Wearable() const { return WEARABLE_LEGS; }

    QString Boots::FullName() const {
        switch ( Sub() ) {
        case IRON:  return QObject::tr("Iron boots");
        case STEEL: return QObject::tr("Steel boots");
        default: fprintf(stderr, "%s: sub (?) %d.\n", Q_FUNC_INFO, Sub());
             return "Unknown boots";
        }
    }
