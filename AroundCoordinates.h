    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2015 Alexander 'mmaulwurff' Kromm
    *  mmaulwurff@gmail.com
    *
    * This file is part of FREG.
    *
    * FREG is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    X()-1 option) any later version.
    *
    * FREG is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with FREG. If not, see <http://www.gnu.org/licenses/>. */

#ifndef AROUNDCOORDINATES_H
#define AROUNDCOORDINATES_H

#include "Xyz.h"
#include "World.h"

enum dirsBits {
    B_UP     = 1,
    B_DOWN   = 2,
    B_AROUND = 4
};

template<int maxSize>
class AroundCoordinatesN {
public:
    const Xyz* begin() const { return array; }
    const Xyz*   end() const { return array + size; }

protected:
    AroundCoordinatesN() : array(), size(0) {}

    void fill4(const Xyz& center);

    Xyz array[maxSize];
    int size;
};

/**
 * @brief The AroundCoordinates class contains coordinates around source.
 * These coordinates are guaranteed to be correct.
 */
class AroundCoordinates : public AroundCoordinatesN<6> {
public:
    AroundCoordinates(const Xyz& source);
    AroundCoordinates(int dirBits, const Xyz& source);
};

class AroundCoordinates4 : public AroundCoordinatesN<4> {
public:
    AroundCoordinates4(const Xyz& source);
};

template<int maxSize>
void AroundCoordinatesN<maxSize>::fill4(const Xyz& xyz) {
    if ( xyz.X() > 0 )    array[size++] = { xyz.X()-1, xyz.Y(), xyz.Z() };
    if ( xyz.Y() > 0 )    array[size++] = { xyz.X(), xyz.Y()-1, xyz.Z() };
    const int bound = World::GetConstWorld()->GetBound();
    if ( xyz.X() < bound) array[size++] = { xyz.X()+1, xyz.Y(), xyz.Z() };
    if ( xyz.Y() < bound) array[size++] = { xyz.X(), xyz.Y()+1, xyz.Z() };
}

#endif // AROUNDCOORDINATES_H
