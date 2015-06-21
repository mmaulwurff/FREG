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

#include "WorldMap.h"
#include "header.h"
#include "Utility.h"

#include <cmath>

#include <QSettings>
#include <QString>
#include <QFile>
#include <QByteArray>

#define UNDERGROUND_ONLY false

class MapSettings : public QSettings {
public:
    MapSettings(const QString& worldName) :
            QSettings(home_path + worldName + Str("/map.ini"),
                QSettings::IniFormat)
    {}
};

WorldMap::WorldMap(const QString& world_name) :
        map(),
        spawnLongitude(),
        spawnLatitude(),
        defaultShred(),
        outerShred()
{
    MapSettings map_info(world_name);
    const int mapWidth  =
        map_info.value(Str("map_width"), DEFAULT_MAP_SIZE).toInt();
    const int mapHeight =
        map_info.value(Str("map_height"), DEFAULT_MAP_SIZE).toInt();
    defaultShred = map_info.value(Str("default_shred"), QChar(SHRED_PLAIN)).
        toString().at(0).toLatin1();
    outerShred   = map_info.value(Str("outer_shred"), QChar(SHRED_OUT_BORDER)).
        toString().at(0).toLatin1();
    map_info.setValue(Str("default_shred"),
        QString::fromLatin1(&defaultShred, 1));
    map_info.setValue(Str(  "outer_shred"),
        QString::fromLatin1(&outerShred, 1));
    MakeAndSaveSpawn(world_name, mapWidth, mapHeight,
        &spawnLongitude, &spawnLatitude);

    QFile mapFile(home_path + world_name + Str("/map.txt"));

    map.resize(mapHeight);

    if ( not mapFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        GenerateMap(world_name, DEFAULT_MAP_SIZE, SHRED_WATER, 0);
        // second attempt to open, since file could be created by GenerateMap
        mapFile.open(QIODevice::ReadOnly | QIODevice::Text);
    }

    if ( mapFile.isOpen() ) {
        std::for_each(ALL(map), [&mapFile](QByteArray& line) {
            line = mapFile.readLine();
        });
    } else {
        std::for_each(ALL(map), [mapWidth, this](QByteArray& line) {
            line = QByteArray(mapWidth, outerShred);
        });
    }
}

void WorldMap::MakeAndSaveSpawn(const QString& world_name,
        const int width, const int height, qint64* longitude, qint64* latitude)
{
    MapSettings map_info(world_name);
    *longitude = map_info.value(Str("spawn_longitude"),
        GetSpawnCoordinate(height)).toLongLong();
    *latitude  = map_info.value(Str("spawn_latitude"),
        GetSpawnCoordinate(width)).toLongLong();

    map_info.setValue(Str("spawn_longitude"), *longitude);
    map_info.setValue(Str("spawn_latitude" ), *latitude );
}

qint64 WorldMap::GetSpawnCoordinate(int size) {
    size = std::max(size/4, 1);
    return (qrand()%size) + size;
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

void WorldMap::Circle(const int min_rad, const int max_rad,
        const char ch, std::vector<QByteArray>& map)
{
    Q_ASSERT(min_rad < max_rad);
    float max[360] = { float(qrand()%(max_rad - min_rad) + min_rad) };
    for (int x=1; x<360; ++x) {
        max[x] = ( x > 345 ) ? // connect beginning and end of circle
            max[x-1] + (max[0] - max[345]) / 15 :
            qBound(float(min_rad),
                max[x-1]+(qrand()%400-200)/200.f, float(max_rad));
    }
    const int size = map.size();
    for (size_t y=0; y<map.size();    ++y)
    for (int    x=0; x<map[y].size(); ++x) {
        if ( R(x, y, size) < max[int(round(Deg(x, y, size)))] ) {
            map[y][uint(x)] = ch;
        }
    }
}

void WorldMap::GenerateMap(const QString& world_name,
        int size, const char outer, const int seed)
{
    if ( seed ) {
        qsrand(seed);
    }
    size = std::max(10, size);

    std::vector<QByteArray> map(size, QByteArray(size, outer));

    const float min_rad = size / 3.0f;
    const float max_rad = size / 2.0f;
    Circle(min_rad,   max_rad,     SHRED_WASTE,       map);
    Circle(min_rad/2, max_rad/2,   SHRED_DEAD_FOREST, map);
    Circle(min_rad/3, max_rad/3+1, SHRED_DEAD_HILL,   map);
    Circle(min_rad/4, max_rad/4+1, SHRED_MOUNTAIN,    map);

    int lakes_number = (qrand() % size) + 5;
    while ( lakes_number-- ) {
        char type = SHRED_WATER;
        switch ( qrand() % 4 ) {
        case 0: type = SHRED_ACID_LAKE; break;
        case 1: type = SHRED_LAVA_LAKE; break;
        case 2: type = SHRED_CRATER;    break;
        }
        const float lake_size  = qrand() % (size/10) + 1;
        const int lake_start_x = qrand() % int(size-lake_size);
        const int lake_start_y = qrand() % int(size-lake_size);
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

    qint64 spawn_longitude, spawn_latitude;
    MakeAndSaveSpawn(world_name, size,size, &spawn_longitude, &spawn_latitude);
    PieceOfEden(spawn_latitude-1, spawn_longitude-1, map);

    QFile file(home_path + world_name + Str("/map.txt"));
    if ( file.open(QIODevice::WriteOnly) ) {
        for (int y=0; y<size; ++y, file.putChar('\n'))
        for (int x=0; x<size; ++x) {
            file.putChar(map[y].at(x));
        }
    }

    MapSettings(world_name).setValue(Str("map_size"), size);
}

void WorldMap::PieceOfEden(const qint64 x, const qint64 y,
        std::vector<QByteArray>& map)
{
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
