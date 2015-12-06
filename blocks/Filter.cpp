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

#include "blocks/Filter.h"
#include "World.h"

Filter::Filter(const kinds kind, const subs sub) :
    Container(kind, sub, FILTER_INV_SIZE)
{}

Filter::Filter(QDataStream& str, const kinds kind, const subs sub) :
    Container(str, kind, sub, FILTER_INV_SIZE)
{}

QString Filter::InvFullName(const int slot_number) const {
    TrString exampleString = tr("-example-");
    return ( ( slot_number > 0 ) ?
        Container::InvFullName(slot_number) :
        (IsEmpty(0) ?
            exampleString : Inventory::InvFullName(0)) );
}

bool Filter::Get(Block* const block, int) {
    if ( IsEmpty() ) {
        return Container::Get(block, 0);
    } else {
        World* const world = World::GetWorld();
        Container::Get(block, 1);
        if (    block->Kind() == ShowBlock(0)->Kind() &&
                block->Sub()  == ShowBlock(0)->Sub() )
        {
            int x, y, z;
            world->Focus(X(), Y(), Z(), &x, &y, &z, GetDir());
            return world->Drop(this, x, y, z, 1, 0, 1);
        } else {
            return world->Drop(this, X(), Y(), Z()-1, 1, 0, 1);
        }
    }
}
