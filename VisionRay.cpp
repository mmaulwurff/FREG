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
        maximum(std::max(std::max(abs(x_step), abs(y_step)), abs(z_step))),
        step(0),
        precalculated((recalculate || maximum>PreCalculatedLightRays::RADIUS) ?
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

PreCalculatedLightRays::PreCalculatedLightRays() {
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

const VisionRay::RaySteps *PreCalculatedLightRays::getRaySteps(
        const int x_shift, const int y_shift, const int z_shift)
const {
    return &shifts[x_shift + RADIUS][y_shift + RADIUS][z_shift + RADIUS];
}

const PreCalculatedLightRays VisionRay::preCalculatedLightRays;
