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

#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include <vector>
#include <random>

#include <QtGlobal>
#include <QString>

class QString;
class QByteArray;

class WorldMap final {
public:

    /** @brief WorldMap generates new world map.
     *  @param worldName generated world name.
     *  @param size generated world size.
     *  @param outer generated world outer (surrounding) type of shred.
     *  @param seed for random generation. */
    WorldMap(const QString& worldName, int size, char outer, int seed);

    /** @brief WorldMap loads world map if it exists, otherwise generates it.
     *  @param worldName loaded/generated world name. */
    explicit WorldMap(const QString &worldName);

    char TypeOfShred(qint64 longitude, qint64 latitude) const;

    qint64 GetWidth(qint64 longitude) const;
    qint64 GetHeight() const;

    qint64 GetSpawnLongitude() const;
    qint64 GetSpawnLatitude()  const;

    qint64 GetSpawnCoordinate(int size) const;

    void saveToDisk() const;

    static const int DEFAULT_MAP_SIZE = 79;

private:

    Q_DISABLE_COPY(WorldMap)

    struct InitialConstructor {};

    WorldMap(const QString &worldName, InitialConstructor);

    static float Deg(int x, int y, int size);
    static float R  (int x, int y, int size);

    void Circle(int min_rad, int max_rad, char ch);

    void PieceOfEden(qint64 x, qint64 y);

    void MakeAndSaveSpawn(qint64* longitude, qint64* latitude) const;

    void GenerateMap(int size, char outer, int seed);

    QString worldName;
    std::vector<QByteArray> map;
    qint64 spawnLongitude, spawnLatitude;
    char defaultShred, outerShred;
    mutable std::minstd_rand randomEngine;
};

#endif // WORLD_MAP_H
