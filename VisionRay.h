#ifndef VISION_RAY_H
#define VISION_RAY_H

#include "Xyz.h"
#include <vector>

class PreCalculatedVisionRays;

class VisionRay {
public:
    VisionRay(const Xyz& from, const Xyz& to, bool force_calculate = false);

    bool nextStep();

    Xyz getCoordinateFirst() const;
    Xyz getCoordinateSecond() const;

    typedef std::vector< std::pair<XyzChar, XyzChar> > RaySteps;

private:
    int x, y, z;
    const int x_step, y_step, z_step;
    const int maximum;
    int step;
    const RaySteps* const precalculated;

    static const PreCalculatedVisionRays preCalculatedLightRays;
};

class PreCalculatedVisionRays {
    static const int TABLE_SIZE = 32;

public:
    PreCalculatedVisionRays();

    const VisionRay::RaySteps* getRaySteps(int x_shift, int y_shift,
                                           int z_shift) const;

    static const int RADIUS = TABLE_SIZE / 2 - 1;

private:
    VisionRay::RaySteps shifts[TABLE_SIZE][TABLE_SIZE][TABLE_SIZE];
};

#endif // VISION_RAY_H
