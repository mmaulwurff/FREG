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
#include <cstdlib>
#include <algorithm>

// DDA line with integers only.

VisionRay::VisionRay(const Xyz& from, const Xyz& to, const bool recalculate) :
        x(from.X()),
        y(from.Y()),
        z(from.Z()),
        x_step(to.X() - x),
        y_step(to.Y() - y),
        z_step(to.Z() - z),
        maximum(std::max({abs(x_step), abs(y_step), abs(z_step)})),
        step(0),
        precalculated((recalculate || maximum>PreCalculatedVisionRays::RADIUS)?
            nullptr :
            preCalculatedLightRays.getRaySteps(-x_step, -y_step, -z_step))
{
    if ( not precalculated ) {
        x *= maximum;
        y *= maximum;
        z *= maximum;
    }
}

bool VisionRay::nextStep() {
    if ( ++step < maximum ) {
        if ( precalculated ) {
            return true;
        } else {
            x += x_step;
            y += y_step;
            z += z_step;
            return true;
        }
    } else {
        return false;
    }
}

Xyz VisionRay::getCoordinateFirst() const {
    if ( precalculated ) {
        const XyzChar& unshifted = (*precalculated)[step - 1].first;
        return Xyz(unshifted.X() + x_step + x,
                   unshifted.Y() + y_step + y,
                   unshifted.Z() + z_step + z);
    } else {
        return Xyz(static_cast<unsigned>(x) / maximum,
                   static_cast<unsigned>(y) / maximum,
                   static_cast<unsigned>(z) / maximum);
    }
}

Xyz VisionRay::getCoordinateSecond() const {
    if ( precalculated ) {
        const XyzChar& unshifted = (*precalculated)[step - 1].second;
        return Xyz(unshifted.X() + x_step + x,
                   unshifted.Y() + y_step + y,
                   unshifted.Z() + z_step + z);
    } else {
        return Xyz((x - 1) / static_cast<int>(maximum) + 1,
                   (y - 1) / static_cast<int>(maximum) + 1,
                   static_cast<unsigned>(z - 1) / maximum + 1);
    }
}

PreCalculatedVisionRays::PreCalculatedVisionRays() {
    const Xyz center(RADIUS, RADIUS, RADIUS);
    for (int i=0; i<TABLE_SIZE-1; ++i)
    for (int j=0; j<TABLE_SIZE-1; ++j)
    for (int k=0; k<TABLE_SIZE-1; ++k) {
        VisionRay vision_ray(Xyz(i, j, k), center, true);
        while (vision_ray.nextStep()) {
            const Xyz first  = vision_ray.getCoordinateFirst();
            const Xyz second = vision_ray.getCoordinateSecond();
            shifts[i][j][k].push_back( {
                XyzChar(
                    first.X() - RADIUS,
                    first.Y() - RADIUS,
                    first.Z() - RADIUS),
                XyzChar(
                    second.X() - RADIUS,
                    second.Y() - RADIUS,
                    second.Z() - RADIUS)
            } );
        }
        shifts[i][j][k].shrink_to_fit();
    }
}

const VisionRay::RaySteps* PreCalculatedVisionRays::getRaySteps(
        const int x_shift, const int y_shift, const int z_shift)
const {
    return &shifts[x_shift + RADIUS][y_shift + RADIUS][z_shift + RADIUS];
}

const PreCalculatedVisionRays VisionRay::preCalculatedLightRays;
