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

#include "header.h"
#include "Utility.h"
#include "WorldMap.h"
#include "IniSettings.h"

#include <cmath>
#include <random>

#include <QFile>
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QByteArray>

#define UNDERGROUND_ONLY false

WorldMap::WorldMap(const QString& world_name, InitialConstructor)
    : worldName(world_name)
    , map()
    , spawnLongitude()
    , spawnLatitude()
    , defaultShred()
    , outerShred()
    , randomEngine()
{}

// Generation constructor.
WorldMap::WorldMap(const QString& world_name,
                   const int size,
                   const char outer,
                   const int seed)
    : WorldMap(world_name, InitialConstructor{})
{
    GenerateMap(size, outer, seed);
}

// Loading constructor.
WorldMap::WorldMap(const QString& world_name)
    : WorldMap(world_name, InitialConstructor{})
{
    IniSettings map_info(world_name + Str("/map.ini"));
    defaultShred = map_info.value(Str("default_shred"), QChar(SHRED_PLAIN)).
        toString().at(0).toLatin1();
    outerShred   = map_info.value(Str("outer_shred"), QChar(SHRED_OUT_BORDER)).
        toString().at(0).toLatin1();

    QFile mapFile(home_path + world_name + Str("/map.txt"));
    if (mapFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        for (;;) {
            QByteArray line = mapFile.readLine();
            if ( line.isEmpty() ) {
                break;
            } else {
                map.push_back(line);
            }
        }
        MakeAndSaveSpawn(&spawnLongitude, &spawnLatitude);
    } else {
        GenerateMap(DEFAULT_MAP_SIZE, SHRED_OUT_BORDER, 0);
        saveToDisk();
    }
}

void WorldMap::MakeAndSaveSpawn P(qint64* const, longitude, latitude) const {
    IniSettings map_info(worldName + Str("map.ini"));
    *longitude = map_info.value(Str("spawn_longitude"),
        GetSpawnCoordinate(GetHeight())).toLongLong();
    *latitude  = map_info.value(Str("spawn_latitude"),
        GetSpawnCoordinate(GetWidth(*longitude))).toLongLong();
}

qint64 WorldMap::GetSpawnCoordinate(int size) const {
    size = std::max(size/4, 1);
    return (randomEngine()%size) + size;
}

qint64 WorldMap::GetHeight() const { return map.size(); }
qint64 WorldMap::GetWidth(const qint64 longitude) const {
    return map.at(longitude).size();
}

qint64 WorldMap::GetSpawnLongitude() const { return spawnLongitude; }
qint64 WorldMap::GetSpawnLatitude()  const { return spawnLatitude;  }

char WorldMap::TypeOfShred P(const qint64, longi, lati) const {
    return UNDERGROUND_ONLY ?
        SHRED_UNDERGROUND :
        static_cast<shred_type>(IsInBounds(longi, 0LL, GetHeight()) &&
                                IsInBounds(lati,  0LL, GetWidth(longi)) ?
            map[longi].at(lati) :
            outerShred);
}

float WorldMap::Deg P3(const int, x, y, size) {
    const float PI = 3.141592f;
    return (std::atan2(y-size / 2.f, x-size / 2.f) + PI) * 360 / 2 / PI;
}

float WorldMap::R P3(const int, x, y, size) {
    return std::hypot(x - size/2.f, y - size/2.f);
}

void WorldMap::Circle(const int min_rad, const int max_rad, const char ch) {
    Q_ASSERT(min_rad < max_rad);
    float max[360] = { float(randomEngine()%(max_rad - min_rad) + min_rad) };
    for (int x=1; x<360; ++x) {
        max[x] = ( x > 345 ) ? // connect beginning and end of circle
            max[x-1] + (max[0] - max[345]) / 15 :
            qBound(float(min_rad),
                max[x-1]+(randomEngine()%400-200)/200.f, float(max_rad));
    }
    const int size = map.size();
    for (size_t y=0; y<map.size();    ++y)
    for (int    x=0; x<map[y].size(); ++x) {
        if ( R(x, y, size) < max[int(round(Deg(x, y, size)))] ) {
            map[y][uint(x)] = ch;
        }
    }
}

void WorldMap::GenerateMap(int size, const char outer, const int seed) {
    size = std::max(10, size);

    if ( seed ) {
        randomEngine.seed(seed);
    } else {
        std::random_device randomDevice;
        randomEngine.seed(randomDevice());
    }

    map = std::vector<QByteArray>(size, QByteArray(size, outer));

    const float min_rad = size / 3.0f;
    const float max_rad = size / 2.0f;
    Circle(min_rad,   max_rad,     SHRED_WASTE);
    Circle(min_rad/2, max_rad/2,   SHRED_DEAD_FOREST);
    Circle(min_rad/3, max_rad/3+1, SHRED_DEAD_HILL);
    Circle(min_rad/4, max_rad/4+1, SHRED_MOUNTAIN);

    int lakes_number = (randomEngine() % size) + 5;
    while ( lakes_number-- ) {
        char type = SHRED_WATER;
        switch ( randomEngine() % 4 ) {
        case 0: type = SHRED_ACID_LAKE; break;
        case 1: type = SHRED_LAVA_LAKE; break;
        case 2: type = SHRED_CRATER;    break;
        }
        const float lake_size  = randomEngine() % (size/10) + 1;
        const int lake_start_x = randomEngine() % int(size-lake_size);
        const int lake_start_y = randomEngine() % int(size-lake_size);
        const int border = (lake_size-1)*(lake_size-1)/2/2;
        for (int x=lake_start_x; x<lake_start_x+lake_size; ++x)
        for (int y=lake_start_y; y<lake_start_y+lake_size; ++y) {
            if (      (x-lake_start_x-lake_size/2)*(x-lake_start_x-lake_size/2)
                    + (y-lake_start_y-lake_size/2)*(y-lake_start_y-lake_size/2)
                    < border )
            {
                map[x][y] = type;
            }
        }
    }

    MakeAndSaveSpawn(&spawnLongitude, &spawnLatitude);
    PieceOfEden(spawnLatitude-1, spawnLongitude-1);
}

void WorldMap::saveToDisk() const {
    QFile file(home_path + worldName + Str("/map.txt"));
    if ( file.open(QIODevice::WriteOnly) ) {
        for (qint64 y=0; y<GetHeight(); ++y, file.putChar('\n'))
        for (qint64 x=0; x<GetWidth(y); ++x) {
            file.putChar(map[y].at(x));
        }
    }
}

void WorldMap::PieceOfEden(const qint64 x, const qint64 y) {
    const int EDEN_SIZE = 8;
    const int size = map.size();
    if ( (x+EDEN_SIZE-1)*size + y+EDEN_SIZE-1 > size*size) return;
    const char eden[EDEN_SIZE][EDEN_SIZE + 1] = {
        "^~~~~~~^",
        "~~%%%%~~",
        "~%%%%%%~",
        "~%%++%%~",
        "~%%+C%%~",
        "~%%%%%%~",
        "~~%%%%~~",
        "^~~~~~~^"
    };
    for (int i=0; i<EDEN_SIZE; ++i)
    for (int j=0; j<EDEN_SIZE; ++j) {
        map[i + y][uint(j + x)] = eden[i][j];
    }
}
