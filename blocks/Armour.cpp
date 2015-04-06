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

#include "blocks/Armour.h"

// Armour section
    void Armour::Damage(const int dmg, const int dmg_kind) {
        if ( dmg > THRESHOLD ) {
            Block::Damage(dmg/((Kind() == ARMOUR) ? 4 : 2), dmg_kind);
        }
    }

    int Armour::DamageLevel() const { return 0; }
    wearable Armour::Wearable() const { return WEARABLE_BODY; }

// Helmet section
    wearable Helmet::Wearable() const { return WEARABLE_HEAD; }

// Boots section
    wearable Boots::Wearable() const { return WEARABLE_LEGS; }
