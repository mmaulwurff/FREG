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

#ifndef VISION_RAY_H
#define VISION_RAY_H

template<typename T> class TemplateXyz;
typedef TemplateXyz<short> Xyz;

class VisionRay {
public:

    VisionRay(const Xyz& from, const Xyz& to);

    bool nextStep();

    Xyz getCoordinate() const;

private:

    int x, y, z;
    int step;
    const int x_start, y_start, z_start;
    const int x_step, y_step, z_step;
    const int maximum;
};

#endif // VISION_RAY_H
