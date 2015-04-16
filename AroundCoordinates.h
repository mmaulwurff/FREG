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

#ifndef AROUND_COORDINATES_H
#define AROUND_COORDINATES_H

#include "Xyz.h"

enum directionBits {
    B_UP     = 0b0001,
    B_DOWN   = 0b0010,
    B_AROUND = 0b0100
};

template<int maxSize>
class AroundCoordinatesN {
public:
    const XyzInt* begin() const { return array; }
    const XyzInt*   end() const { return array + size; }

protected:
    AroundCoordinatesN() : array(), size(0) {}

    void fill4(int x_center, int y_center, int z_center);

    XyzInt array[maxSize];
    int size;
};

/**
 * @brief The AroundCoordinates class contains coordinates around source.
 * These coordinates are guaranteed to be correct.
 */
class AroundCoordinates : public AroundCoordinatesN<6> {
public:
    AroundCoordinates(int x_source, int y_source, int z_source);
    AroundCoordinates(int directionBits, int x, int y, int z);
};

class AroundCoordinates4 : public AroundCoordinatesN<4> {
public:
    AroundCoordinates4(int x_source, int y_source, int z_source);
};

class LazyAroundCoordinates : XyzInt {
public:
    LazyAroundCoordinates(int x, int y, int z);

    const XyzInt* getNext();

private:
    int step;
    static const XyzInt shifts[6];
};

#endif // AROUND_COORDINATES_H
