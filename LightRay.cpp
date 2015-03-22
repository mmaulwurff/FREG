#include "LightRay.h"
#include <cstdlib>
#include <algorithm>

// DDA line with integers only.

LightRay::LightRay(const Xyz& from, const Xyz& to, const bool force_calculate):
        x(from.X()),
        y(from.Y()),
        z(from.Z()),
        x_step(to.X() - x),
        y_step(to.Y() - y),
        z_step(to.Z() - z),
        maximum(std::max(std::max(abs(x_step), abs(y_step)), abs(z_step))),
        step(0),
        recalculate(force_calculate || maximum>PreCalculatedLightRays::RADIUS),
        precalculated(recalculate ? nullptr :
            preCalculatedLightRays.getRaySteps(-x_step, -y_step, -z_step))
{
    if ( recalculate ) {
        x *= maximum;
        y *= maximum;
        z *= maximum;
    }
}

bool LightRay::nextStep() {
    if ( not recalculate ) {
        return static_cast<size_t>(++step) < precalculated->size();
    } else if ( ++step < maximum ) {
        x += x_step;
        y += y_step;
        z += z_step;
        return true;
    } else {
        return false;
    }
}

Xyz LightRay::getCoordinateFirst() const {
    if ( not recalculate ) {
        const XyzChar& unshifted = (*precalculated)[step - 1].first;
        return Xyz(unshifted.X() - PreCalculatedLightRays::RADIUS + x_step + x,
                   unshifted.Y() - PreCalculatedLightRays::RADIUS + y_step + y,
                   unshifted.Z() - PreCalculatedLightRays::RADIUS + z_step +z);
    } else {
        return Xyz(static_cast<unsigned>(x) / maximum,
                   static_cast<unsigned>(y) / maximum,
                   static_cast<unsigned>(z) / maximum);
    }
}

Xyz LightRay::getCoordinateSecond() const {
    if ( not recalculate ) {
        const XyzChar& unshifted = (*precalculated)[step - 1].second;
        return Xyz(unshifted.X() - PreCalculatedLightRays::RADIUS + x_step + x,
                   unshifted.Y() - PreCalculatedLightRays::RADIUS + y_step + y,
                   unshifted.Z() - PreCalculatedLightRays::RADIUS + z_step +z);
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
        LightRay light_ray(Xyz(i, j, k), center, true);
        while (light_ray.nextStep()) {
            shifts[i][j][k].push_back( {
                XyzChar(XYZ(light_ray.getCoordinateFirst ())),
                XyzChar(XYZ(light_ray.getCoordinateSecond())) } );
        }
        shifts[i][j][k].shrink_to_fit();
    }
}

const LightRay::RaySteps *PreCalculatedLightRays::getRaySteps(
        const int x_shift, const int y_shift, const int z_shift)
const {
    return &shifts[x_shift + RADIUS][y_shift + RADIUS][z_shift + RADIUS];
}

const PreCalculatedLightRays LightRay::preCalculatedLightRays;
