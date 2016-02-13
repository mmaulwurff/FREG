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

#include "AroundCoordinates.h"
#include "World.h"
#include "header.h"

// AroundCoordinates:: section

AroundCoordinates::AroundCoordinates(const int dirsBits, const_int(x, y, z))
    : AroundCoordinatesN<6>()
{
    if ( dirsBits & B_UP     ) array[size++] = { x, y, z+1 };
    if ( dirsBits & B_DOWN   ) array[size++] = { x, y, z-1 };
    if ( dirsBits & B_AROUND ) {
        fill4(x, y, z);
    }
}

AroundCoordinates::AroundCoordinates(const_int(x, y, z)) :
        AroundCoordinatesN<6>()
{
    array[0] = { x, y, z+1 };
    array[1] = { x, y, z-1 };
    size = 2;
    fill4(x, y, z);
}

// AroundCoordinates4:: section

AroundCoordinates4::AroundCoordinates4(const_int(x, y, z))
    : AroundCoordinatesN<4>()
{
    fill4(x, y, z);
}

template<int maxSize>
void AroundCoordinatesN<maxSize>::fill4(const_int(x, y, z)) {
    if ( x > 0 )    array[size++] = { x-1, y,   z };
    if ( y > 0 )    array[size++] = { x,   y-1, z };
    const int bound = World::GetCWorld()->GetBound();
    if ( x < bound) array[size++] = { x+1, y,   z };
    if ( y < bound) array[size++] = { x,   y+1, z };
}

LazyAroundCoordinates::LazyAroundCoordinates(const_int(x, y, z))
    : XyzInt(x, y, z)
    , step(0)
{}

const XyzInt* LazyAroundCoordinates::getNext() {
    switch (step++) {
    case UP:                        ++z_self; break;
    case DOWN:                        z_self -= 2; break;
    case NORTH:           --y_self; ++z_self; break;
    case EAST:  ++x_self; ++y_self;           break;
    case SOUTH: --x_self; ++y_self;           break;
    case WEST:  --x_self; --y_self;           break;
    default: return nullptr;
    }
    return (World::GetCWorld()->InBounds(x_self, y_self)
            ? this
            : getNext());
}
