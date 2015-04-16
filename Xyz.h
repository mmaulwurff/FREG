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

#ifndef XYZ_H
#define XYZ_H

#define XYZ(xyz) xyz.X(), xyz.Y(), xyz.Z()

template<typename T>
class TemplateXyz {
public:
    TemplateXyz(const int x, const int y, const int z) :
            x_self(x), y_self(y), z_self(z)
    {}
    TemplateXyz() : x_self(), y_self(), z_self() {}

    int X() const { return x_self; }
    int Y() const { return y_self; }
    int Z() const { return z_self; }

    void SetXyz(const int x, const int y, const int z) {
        x_self = x;
        y_self = y;
        z_self = z;
    }

    bool operator!=(const TemplateXyz<T>& other) const {
        return x_self != other.x_self ||
               y_self != other.y_self ||
               z_self != other.z_self;
    }

protected:
    T x_self, y_self, z_self;
};

typedef TemplateXyz<short> Xyz;
typedef TemplateXyz<char>  XyzChar;
typedef TemplateXyz<int>   XyzInt;

#endif // XYZ_H
