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

const uint DEFAULT_MAP_SIZE = 75;

class WorldMap final {
public:
    explicit WorldMap(QString);
    ~WorldMap();

    long MapSize() const;
    char TypeOfShred(long longi, long lati);

private:
    static int Deg(ushort x, ushort y, ushort size);
    static int R(ushort x, ushort y, ushort size);
    static void Circle(
            ushort min_rad,
            ushort max_rad,
            char ch,
            ushort size,
            char * map);
    static void GenerateMap(const char * filename,
            uint size = DEFAULT_MAP_SIZE,
            char outer = '~',
            int seed = 0);

    long mapSize;
    QFile * map;
};

#endif // WORLDMAP_H
