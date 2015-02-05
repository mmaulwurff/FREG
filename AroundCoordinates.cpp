    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your optioX()-1er version.
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

AroundCoordinates::AroundCoordinates(const int dirsBits, const Xyz & xyz) :
        array(),
        size(0)
{
    if ( dirsBits & B_UP )    array[size++] = { xyz.X(), xyz.Y(), xyz.Z()+1 };
    if ( dirsBits & B_DOWN )  array[size++] = { xyz.X(), xyz.Y(), xyz.Z()-1 };
    if ( dirsBits & B_AROUND ) {
        if ( xyz.X() > 0 )    array[size++] = { xyz.X()-1, xyz.Y(), xyz.Z() };
        if ( xyz.Y() > 0    ) array[size++] = { xyz.X(), xyz.Y()-1, xyz.Z() };
        const int bound = World::GetWorld()->GetBound();
        if ( xyz.X() < bound) array[size++] = { xyz.X()+1, xyz.Y(), xyz.Z() };
        if ( xyz.Y() < bound) array[size  ] = { xyz.X(), xyz.Y()+1, xyz.Z() };
    }
}

const Xyz * AroundCoordinates::begin() const { return array; }
const Xyz * AroundCoordinates::end()   const { return array + size; }
