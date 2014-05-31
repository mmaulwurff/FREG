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

#include "blocks/Illuminator.h"
#include "BlockManager.h"

Illuminator::Illuminator(const int sub, const quint16 id) :
        Active(sub, id),
        fuel_level(MAX_FUEL)
{}

Illuminator::Illuminator(QDataStream & str, const int sub, const quint16 id) :
        Active(str, sub, id)
{
    str >> fuel_level;
}

void Illuminator::Damage(int /*dmg*/, int /*dmg_kind*/) { Break(); }
quint8 Illuminator::Kind() const { return ILLUMINATOR; }

QString Illuminator::FullName() const {
    switch ( Sub() ) {
    case WOOD:  return tr("Torch, fuel: %1"        ).arg(fuel_level);
    case STONE: return tr("Flint, charges: %1"     ).arg(fuel_level);
    case IRON:  return tr("Lantern, fuel: %1"      ).arg(fuel_level);
    case GLASS: return tr("Flashlight, battery: %1").arg(fuel_level);
    default: fprintf(stderr, "Illuminator::FullName: sub ?: %d\n", Sub());
        return tr("Strange illuminator");
    }
}

Block * Illuminator::DropAfterDamage() { return this; }

usage_types Illuminator::Use(Block *) {
    if ( Sub() == GLASS ) { // flashlight
        return USAGE_TYPE_NO;
    } else {
        fuel_level = ( fuel_level >= 10 ) ?
            fuel_level - 10 : 0;
        return USAGE_TYPE_SET_FIRE;
    }
}

int Illuminator::LightRadius() const {
    if ( fuel_level == 0 ) return 0;
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return 4;
    case IRON:  return 5;
    case GLASS: return 7;
    }
}

bool Illuminator::ShouldFall() const { return false; }
int  Illuminator::ShouldAct() const { return FREQUENT_RARE; }
void Illuminator::DoRareAction() { ActInner(); }

INNER_ACTIONS Illuminator::ActInner() {
    if ( fuel_level == 0 ) {
        if ( Sub() == WOOD ) {
            Break();
        }
    } else {
        if ( Sub() != STONE ) {
            --fuel_level;
        }
    }
    return INNER_ACTION_NONE;
}

void Illuminator::SaveAttributes(QDataStream & out) const {
    Active::SaveAttributes(out);
    out << fuel_level;
}
