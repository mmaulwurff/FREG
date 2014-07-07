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

#include "world.h"
#include "Bucket.h"

Bucket::Bucket(const int sub, const int id) :
        Block(sub, id),
        Inventory(1)
{}

Bucket::Bucket(QDataStream & str, const int sub, const int id) :
        Block(str, sub, id),
        Inventory(str, 1)
{}

int  Bucket::Kind() const { return BUCKET; }
int  Bucket::Sub()  const { return Block::Sub(); }
void Bucket::Damage(int /*dmg*/, int /*dmg_kind*/) { Break(); }
void Bucket::ReceiveSignal(const QString str) { Block::ReceiveSignal(str); }
Inventory * Bucket::HasInventory() { return this; }
usage_types Bucket::Use(Block *) { return USAGE_TYPE_POUR; }

QString Bucket::FullName() const {
    QString name;
    switch ( Sub() ) {
    default:    name = QObject::tr("Bucket"); break;
    case GLASS: name = QObject::tr("Bottle"); break;
    }
    return ( GetInvSub(0) == AIR ) ?
        QObject::tr("Empty bucket") :
        QObject::tr("%1 with %2 (%3/%4 full)")
            .arg(name)
            .arg(ShowBlock(0)->FullName().toLower())
            .arg(Number(0))
            .arg(MAX_STACK_SIZE);
}

bool Bucket::Get(Block * const block, const int start = 0) {
    if ( block->Kind() != LIQUID ) {
        return false;
    }
    for (int i=start; i<Size(); ++i) {
        if ( GetExact(block, i) ) {
            return true;
        }
    }
    return false;
}

void Bucket::SaveAttributes(QDataStream & out) const {
    Inventory::SaveAttributes(out);
}

int Bucket::Weight() const {
    return Block::Weight()/6+Inventory::Weight();
}
