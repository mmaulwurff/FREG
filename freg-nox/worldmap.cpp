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

#include <QFile>
#include <qmath.h>
#include "worldmap.h"
#include "header.h"

const float PI = 3.141592f;

WorldMap::WorldMap(const QString world_name) :
        map(new QFile(world_name+"/map.txt"))
{
    if ( map->open(QIODevice::ReadOnly | QIODevice::Text) ) {
        mapSize = int(qSqrt(1+4*map->size())-1)/2;
    } else {
        mapSize = ( map->open(QIODevice::ReadOnly | QIODevice::Text) ) ?
            DEFAULT_MAP_SIZE : 1;
    }
}

char WorldMap::TypeOfShred(const long longi, const long lati) {
    if (
            longi >= mapSize || longi < 0 ||
            lati  >= mapSize || lati  < 0 )
    {
        return OUT_BORDER_SHRED;
    } else if ( not map->seek((mapSize+1)*longi+lati) ) {
        return DEFAULT_SHRED;
    }
    char c;
    map->getChar(&c);
    return c;
}

long WorldMap::MapSize() const { return mapSize; }

WorldMap::~WorldMap() { delete map; }

float WorldMap::Deg(const float x, const float y, const ushort size) {
    const float x_cent = x-size/2;
    const float y_cent = y-size/2;
    float fi;
    if ( x_cent > 0 && y_cent >= 0 ) {
        fi = atan(y_cent/x_cent);
    } else if ( x_cent>0 && y_cent<0 ) {
        fi = atan(y_cent/x_cent)+2*PI;
    } else if ( x_cent<0 ) {
        fi = atan(y_cent/x_cent)+PI;
    } else if ( x_cent==0 && y_cent>0 ) {
        fi = PI/2;
    } else if ( x_cent==0 && y_cent<0 ) {
        fi = 3*PI/2;
    } else {
        fi = 0;
    }
    return 360*fi / 2 / PI;
}

float WorldMap::R(const float x, const float y, const ushort size) {
    return sqrt((x-size/2)*(x-size/2)+(y-size/2)*(y-size/2));
}

void WorldMap::Circle(
        const float min_rad,
        const float max_rad,
        const char ch,
        const ushort size,
        char * const map)
{
    if ( min_rad >= max_rad ) {
        fprintf(stderr,
            "WorldMap::Circle: %c: min_rad (%f) >= max_rad (%f).\n",
            ch, min_rad, max_rad);
    }
    float maxs[360] = { (float)qMax(min_rad, float(qrand()%qRound(max_rad))) };
    for (ushort x=1; x<360; ++x) {
        float rad_change = (qrand()%400-200.0)/200.0;
        maxs[x] = maxs[x-1] + rad_change;
        if ( maxs[x] > max_rad ) {
            maxs[x] = max_rad;
            rad_change -= 0.01;
        } else if ( maxs[x] < min_rad ) {
            maxs[x] = min_rad;
            rad_change += 0.01;
        }
        if ( x > 315 ) {
            maxs[x] += (maxs[0]-maxs[x-1])/90;
        }
    }
    for (ushort y=0; y<size; ++y)
    for (ushort x=0; x<size; ++x) {
        if ( R(x, y, size) < maxs[qRound(Deg(x, y, size))] ) {
            map[x*size+y] = ch;
        }
    }
}

void WorldMap::GenerateMap(
        const char * const filename,
        ushort size,
        const char outer,
        const int seed)
{
    qsrand(seed);
    size = qMax(ushort(10U), size);

    char * const map = new char[size*size];
    for (ushort y=0; y<size; ++y)
    for (ushort x=0; x<size; ++x) {
        map[x*size+y] = outer;
    }

    const float min_rad = (float)size/3;
    const float max_rad = (float)size/2;
    Circle(min_rad, max_rad, '.', size, map);
    Circle(min_rad/2, max_rad/2,   SHRED_FOREST, size, map);
    Circle(min_rad/3, max_rad/3+1, SHRED_HILL, size, map);
    Circle(min_rad/4, max_rad/4+1, SHRED_MOUNTAIN, size, map);

    // rivers
    // on large maps rivers behave badly, bug is known.
    // TODO: another river algorythm
    const float river_width = 4;
    const float river_width_deg = river_width*80/size;
    for (float deg=river_width_deg; deg<360-river_width_deg; ++deg) {
        if ( qrand()%(60*80/size) == 0 ) {
            for (float j=deg-river_width_deg; j<=deg+river_width_deg; ++j) {
                for (float r=max_rad/3; r<max_rad; r+=0.5f) {
                    map[int((r*cos(j*2*PI/360))+size/2)*size+
                        int((r*sin(j*2*PI/360))+size/2)] = SHRED_WATER;
                }
            }
        }
    }

    FILE * const file = fopen(filename, "wb");
    for (ushort y=0; y<size; ++y, fputc('\n', file))
    for (ushort x=0; x<size; ++x) {
        fputc(map[x*size+y], file);
    }
    delete [] map;
    fclose(file);
}
