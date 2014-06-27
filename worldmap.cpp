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

#include <qmath.h>
#include "worldmap.h"
#include "header.h"

const float PI = 3.141592f;

WorldMap::WorldMap(const QString world_name) :
        map(world_name+"/map.txt")
{
    if ( map.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        mapSize = int(qSqrt(1+4*map.size())-1)/2;
    } else {
        GenerateMap(qPrintable(world_name+"/map.txt"),
            DEFAULT_MAP_SIZE, SHRED_WATER, 0);
        mapSize = ( map.open(QIODevice::ReadOnly | QIODevice::Text) ) ?
            DEFAULT_MAP_SIZE : 1;
    }
}

char WorldMap::TypeOfShred(const long longi, const long lati) const {
    if (
            longi >= mapSize || longi < 0 ||
            lati  >= mapSize || lati  < 0 )
    {
        return OUT_BORDER_SHRED;
    } else if ( not map.seek((mapSize+1)*longi+lati) ) {
        return DEFAULT_SHRED;
    }
    char c;
    map.getChar(&c);
    return c;
}

long WorldMap::MapSize() const { return mapSize; }

float WorldMap::Deg(const int x, const int y, const int size) {
    const float x_cent = x-size/2.f;
    const float y_cent = y-size/2.f;
    float fi;
    if ( x_cent > 0 && y_cent >= 0 ) {
        fi = atanf(y_cent/x_cent);
    } else if ( x_cent>0 && y_cent<0 ) {
        fi = atanf(y_cent/x_cent)+2*PI;
    } else if ( x_cent<0 ) {
        fi = atanf(y_cent/x_cent)+PI;
    } else if ( qFuzzyCompare(x_cent, 0.f) && y_cent>0 ) {
        fi = PI/2;
    } else if ( qFuzzyCompare(x_cent, 0.f) && y_cent<0 ) {
        fi = 3*PI/2;
    } else {
        fi = 0;
    }
    return 360*fi / 2 / PI;
}

float WorldMap::R(const int x, const int y, const int size) {
    return sqrtf((x-size/2.f)*(x-size/2.f)+(y-size/2.f)*(y-size/2.f));
}

void WorldMap::Circle(
        const int min_rad,
        const int max_rad,
        const char ch,
        const int size,
        char * const map)
{
    if ( min_rad >= max_rad ) {
        fprintf(stderr, "%s: %c: min_rad (%d) >= max_rad (%d).\n",
            Q_FUNC_INFO, ch, min_rad, max_rad);
    }
    float maxs[360] = { float(qMax(min_rad, qrand()%Round(max_rad))) };
    for (int x=1; x<360; ++x) {
        maxs[x] = qBound(float(min_rad),
            maxs[x-1]+(qrand()%400-200)/200.f, float(max_rad));
        if ( x > 315 ) { // connect beginning and end of circle
            maxs[x] += (maxs[0]-maxs[x-1])/90;
        }
    }
    for (int y=0; y<size; ++y)
    for (int x=0; x<size; ++x) {
        if ( R(x, y, size) < maxs[Round(Deg(x, y, size))] ) {
            map[x*size+y] = ch;
        }
    }
}

void WorldMap::GenerateMap(
        const char * const filename,
        int size,
        const char outer,
        const int seed)
{
    qsrand(seed);
    size = qMax(10, size);

    char * const map = new char[size*size];
    for (int y=0; y<size; ++y)
    for (int x=0; x<size; ++x) {
        map[x*size+y] = outer;
    }

    const float min_rad = size/3.0;
    const float max_rad = size/2.0;
    Circle(min_rad,   max_rad,     SHRED_PLAIN,    size, map);
    Circle(min_rad/2, max_rad/2,   SHRED_FOREST,   size, map);
    Circle(min_rad/3, max_rad/3+1, SHRED_HILL,     size, map);
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
                    map[int((r*cosf(j*2*PI/360))+size/2)*size+
                        int((r*sinf(j*2*PI/360))+size/2)] = SHRED_WATER;
                }
            }
        }
    }

    FILE * const file = fopen(filename, "wb");
    for (int y=0; y<size; ++y, fputc('\n', file))
    for (int x=0; x<size; ++x) {
        fputc(map[x*size+y], file);
    }
    delete [] map;
    fclose(file);
}
