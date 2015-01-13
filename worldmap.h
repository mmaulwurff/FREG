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

#ifndef WORLDMAP_H
#define WORLDMAP_H

#include <QFile>

class WorldMap final {
public:
    explicit WorldMap(QString worldName);

    char TypeOfShred(qint64 longi, qint64 lati) const;
    static void GenerateMap(
            QString world_name,
            int size,
            char outer,
            int seed);
    qint64 GetSize() const;
    qint64 GetSpawnLongitude() const;
    qint64 GetSpawnLatitude()  const;
    static qint64 GetSpawnCoordinate(int size);

    static const int DEFAULT_MAP_SIZE = 79;

private:
    Q_DISABLE_COPY(WorldMap)

    static constexpr float PI = 3.141592f;

    static float Deg(int x, int y, int size);
    static float R  (int x, int y, int size);
    static void Circle(int min_rad, int max_rad, char ch, int size, char* map);
    static void PieceOfEden(qint64 x, qint64 y, char * map, size_t map_size);
    static void MakeAndSaveSpawn(QString world_name, int size,
            qint64 * longitude, qint64 * latitude);

    int mapSize;
    mutable QFile map;
    qint64 spawnLongitude, spawnLatitude;
};

#endif // WORLDMAP_H
