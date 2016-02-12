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

#include "VisionRay.h"
#include "Xyz.h"
#include <cstdlib>

// DDA line with integers only.

VisionRay::VisionRay(const Xyz& from, const Xyz& to)
    : x(0)
    , y(0)
    , z(0)
    , step(0)
    , x_start(from.X())
    , y_start(from.Y())
    , z_start(from.Z())
    , x_step(to.X() - x_start)
    , y_step(to.Y() - y_start)
    , z_step(to.Z() - z_start)
    , maximum(std::abs(x_step) + std::abs(y_step) + std::abs(z_step))
{}

bool VisionRay::nextStep() {
    if ( ++step < maximum ) {
        x += x_step;
        y += y_step;
        z += z_step;
        return true;
    } else {
        return false;
    }
}

Xyz VisionRay::getCoordinate() const {
    return Xyz( x / maximum + x_start
              , y / maximum + y_start
              , z / maximum + z_start );
}
