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

enum dirsBits {
    B_UP     = 1,
    B_DOWN   = 2,
    B_AROUND = 4
};

/**
 * @brief The AroundCoordinates class contains coordinates around source.
 * These coordinates are guaranteed to be correct.
 */
class AroundCoordinates {
public:
    AroundCoordinates(int dirBits, const Xyz & source);

    const Xyz * begin() const;
    const Xyz *   end() const;

private:
    Xyz array[6];
    int size;
};

#endif // AROUNDCOORDINATES_H
