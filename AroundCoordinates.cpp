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

// AroundCoordinates:: section

AroundCoordinates::AroundCoordinates(const int dirsBits, const XyzInt& xyz) :
        AroundCoordinatesN<6>()
{
    if ( dirsBits & B_UP     ) array[size++] = { xyz.X(), xyz.Y(), xyz.Z()+1 };
    if ( dirsBits & B_DOWN   ) array[size++] = { xyz.X(), xyz.Y(), xyz.Z()-1 };
    if ( dirsBits & B_AROUND ) {
        fill4(xyz);
    }
}

AroundCoordinates::AroundCoordinates(const XyzInt& xyz) :
        AroundCoordinatesN<6>()
{
    array[0] = { xyz.X(), xyz.Y(), xyz.Z()+1 };
    array[1] = { xyz.X(), xyz.Y(), xyz.Z()-1 };
    size = 2;
    fill4(xyz);
}

// AroundCoordinates4:: section

AroundCoordinates4::AroundCoordinates4(const XyzInt& xyz) :
    AroundCoordinatesN<4>()
{
    fill4(xyz);
}

template<int maxSize>
void AroundCoordinatesN<maxSize>::fill4(const XyzInt& xyz) {
    if ( xyz.X() > 0 )    array[size++] = { xyz.X()-1, xyz.Y(), xyz.Z() };
    if ( xyz.Y() > 0 )    array[size++] = { xyz.X(), xyz.Y()-1, xyz.Z() };
    const int bound = World::GetConstWorld()->GetBound();
    if ( xyz.X() < bound) array[size++] = { xyz.X()+1, xyz.Y(), xyz.Z() };
    if ( xyz.Y() < bound) array[size++] = { xyz.X(), xyz.Y()+1, xyz.Z() };
}

LazyAroundCoordinates::LazyAroundCoordinates(const XyzInt& source) :
    XyzInt(source),
    step(-1)
{}

const XyzInt LazyAroundCoordinates::shifts[] = {
    { 0,  0,  1},
    { 0,  0, -2},
    {-1,  0,  1},
    { 2,  0,  0},
    {-1, -1,  0},
    { 0,  2,  0}
};

const XyzInt *LazyAroundCoordinates::getNext() {
    ++step;
    if (step < DIRS_COUNT) {
        x_self += shifts[step].X();
        y_self += shifts[step].Y();
        z_self += shifts[step].Z();
        return World::GetConstWorld()->InBounds(x_self, y_self) ?
            this : getNext();
    } else {
        return nullptr;
    }
}
