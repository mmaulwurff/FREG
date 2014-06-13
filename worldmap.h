    /* freg, Free-Roaming Elementary Game with open and interactive world
    *  Copyright (C) 2012-2014 Alexander 'mmaulwurff' Kromm
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

#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <QFile>

class WorldMap final {
public:
    explicit WorldMap(QString);

    WorldMap & operator=(const WorldMap &) = delete;
    WorldMap(const WorldMap &) = delete;

    long MapSize() const;
    char TypeOfShred(long longi, long lati) const;
    static void GenerateMap(
            const char * filename,
            ushort size,
            char outer,
            int seed);

private:
    static float Deg(float x, float y, ushort size);
    static float R  (float x, float y, ushort size);
    static void Circle(
            float min_rad,
            float max_rad,
            char ch,
            ushort size,
            char * map);

    long mapSize;
    mutable QFile map;
};

#endif // WORLDMAP_H
