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

#include "blocks/Bucket.h"
#include "TrManager.h"
#include <QDataStream>

Bucket::Bucket(const kinds kind, const subs sub)
    : Block(kind, sub)
    , Inventory(1)
{}

Bucket::Bucket(QDataStream& str, const kinds kind, const subs sub)
    : Block(str, kind, sub)
    , Inventory(str, 1)
{}

void Bucket::Damage(int, int) { Break(); }
void Bucket::ReceiveSignal(const QString& str) { Block::ReceiveSignal(str); }
usage_types Bucket::Use(Active*) { return USAGE_TYPE_POUR; }

QString Bucket::FullName() const {
    TrString glassNameString = QObject::tr("Bottle");
    TrString emptyString     = QObject::tr("(empty)");
    TrString fullString      = QObject::tr("with %1 (%2/%3 full)");

    QString name;
    switch ( Sub() ) {
    default:    name = TrManager::KindName(BUCKET); break;
    case GLASS: name = glassNameString; break;
    }
    return ( Str("%1 (%2) %3")
        .arg(name)
        .arg(TrManager::SubName(Sub()))
        .arg( IsEmpty(0) ?
            emptyString :
            fullString
                .arg(ShowBlock(0)->FullName().toLower())
                .arg(Count(0))
                .arg(MAX_STACK_SIZE)) );
}

bool Bucket::Get(Block* const block, const int start) {
    if ( block->Wearable() == WEARABLE_VESSEL ) {
        for (int i=start; i<GetSize(); ++i) {
            if ( GetExact(block, i) ) {
                return true;
            }
        }
    }
    return false;
}

void Bucket::SaveAttributes(QDataStream& out) const {
    Inventory::SaveAttributes(out);
}

int Bucket::Weight() const {
    return Block::Weight()/6 + Inventory::Weight();
}
