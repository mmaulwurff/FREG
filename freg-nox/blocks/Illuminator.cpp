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
        Active(sub, id)
{}

Illuminator::Illuminator(QDataStream & str, const int sub, const quint16 id) :
        Active(str, sub, id)
{}

void Illuminator::Damage(ushort /*dmg*/, int /*dmg_kind*/) { durability = 0; }
quint8 Illuminator::Kind() const { return ILLUMINATOR; }

QString Illuminator::FullName() const {
    switch ( Sub() ) {
    case WOOD:  return tr("Torch");
    case STONE: return tr("Flint");
    case IRON:  return tr("Lantern");
    case GLASS: return tr("Flashlight");
    default: fprintf(stderr, "Illuminator::FullName: sub ?: %d\n", Sub());
        return tr("Strange illuminator");
    }
}

Block * Illuminator::DropAfterDamage() const {
    return BlockManager::NewBlock(ILLUMINATOR, Sub());
}

usage_types Illuminator::Use(Block *) {
    return ( WOOD==Sub() || STONE==Sub() || IRON==Sub() ) ?
        USAGE_TYPE_SET_FIRE : USAGE_TYPE_NO;
}

uchar Illuminator::LightRadius() const {
    switch ( Sub() ) {
    default:
    case STONE: return 0;
    case WOOD:  return 3;
    case IRON:  return 4;
    case GLASS: return 5;
    }
}
