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

#ifndef SHRED_H
#define SHRED_H

#include "Weather.h"
#include "header.h"
#include <forward_list>
#include <QLinkedList>

class World;
class Block;
class Active;
class Falling;

class Shred final : public Weather {
public:
     Shred(int shred_x, int shred_y, qint64 longi, qint64 lati);
    ~Shred();

    /// Returns y (line) shred coordinate on world map.
    qint64 Longitude() const { return longitude; }
    /// Returns x (column) shred coordinate on world map.
    qint64 Latitude()  const { return latitude;  }
    int ShredX() const { return shredX; }
    int ShredY() const { return shredY; }
    void PhysEventsFrequent();
    void PhysEventsRare();

    void Register(Active *);
    void Unregister(Active *);
    void AddShining(Active *);
    void RemShining(Active *);
    void AddToDelete(Active *);
    void ReloadTo(dirs);

    std::forward_list<Active *>::const_iterator ShiningBegin() const {
        return shiningList.cbegin();
    }

    std::forward_list<Active *>::const_iterator ShiningEnd() const {
        return shiningList.cend();
    }

    void SaveShred(bool isQuitGame);

    Block * GetBlock(const int x, const int y, const int z) const {
        return blocks[x][y][z];
    }

    /// Removes last block at xyz, then SetBlock, then makes block normal.
    void SetBlock(Block * block, int x, int y, int z);
    /// Puts block to coordinates xyz and activates it.
    void SetBlockNoCheck(Block *, int x, int y, int z);
    void AddFalling(Block *);

    /// Puts block to coordinates, not activates it.
    void PutBlock(Block * const block, const int x, const int y, const int z) {
        blocks[x][y][z] = block;
    }

    // Lighting section

    int Lightmap(const int x, const int y, const int z) const {
        return lightMap[x][y][z];
    }

    int FireLight( int x, int y, int z) const;
    int SunLight(  int x, int y, int z) const;
    int LightLevel(int x, int y, int z) const;

    bool SetSunLight( int x, int y, int z, int level);
    bool SetFireLight(int x, int y, int z, int level);
    void SetLightmap( int x, int y, int z, int level);

    void SetAllLightMapNull();
    void ShineAll();

    // Information section
    void SetNewBlock(int kind, int sub, int x, int y, int z, int dir = UP);
    shred_type GetTypeOfShred() const { return type; }

    static QString FileName(QString world_name, qint64 longi, qint64 lati);
    Shred * GetShredMemory() const;
    /// Make global coordinate from local (in loaded zone).
    qint64 GlobalX(int x) const;
    qint64 GlobalY(int y) const;
    /// Get local coordinate.
    static int CoordInShred(const int x) { return x & 0xF; }

    /// Get shred coordinate in loaded zone (from 0 to numShreds).
    static int CoordOfShred(const int x) { return x >> SHRED_WIDTH_BITSHIFT; }

    /// Lowest nullstone and sky are not in bounds.
    static bool InBounds(int x, int y, int z);
    static bool InBounds(int x, int y);
    static bool InBounds(int z);

    void Rain(int kind, int sub);
    void Dew (int kind, int sub);

private:
    Q_DISABLE_COPY(Shred)

    static const quint8 DATASTREAM_VERSION;
    static const quint8 CURRENT_SHRED_FORMAT_VERSION = 15;

    static const int RAIN_IS_DEW = 1;

    void RemoveAllSunLight();
    void RemoveAllFireLight();
    void RemoveAllLight();

    bool LoadShred();
    void RegisterInit(Active *);

    /// Builds normal underground. Returns ground level.
    int NormalUnderground(int depth = 0, subs sub = SOIL);
    void CoverWith(int kind, int sub);
    /// Puts num things(kind-sub) in random places on shred surface.
    /** If on_water is false, this will not drop things on water,
     *  otherwise on water too. */
    void RandomDrop(int num, int kind, int sub, bool on_water = false);
    void DropBlock(Block * bloc, bool on_water);
    int CountShredTypeAround(int type) const;

    void PlantGrass();
    void TestShred();
    void NullMountain();
    void Plain();
    void Forest(bool dead);
    void Water(subs sub = WATER);
    void Pyramid();
    void Mountain();
    void Hill(bool dead);
    void Desert();
    void Castle();
    void WasteShred();
    /// For testing purposes.
    void ChaosShred();

    /// Loads room from corresponding .room or -index.room file.
    /// Should be placed before any other block generation at the same place.
    bool LoadRoom(int level, int index = 0);

    /// Block combinations section (trees, buildings, etc):
    bool Tree(int x, int y, int z, int height);

    /// Special land generation
    void ShredLandAmplitudeAndLevel(qint64 longi, qint64 lati,
            unsigned * l, float * a) const;
    void ShredNominalAmplitudeAndLevel(char shred_type,
            unsigned * l, float * a) const;
    void AddWater();
    void NormalCube(int x_start, int y_start, int z_start,
                    int x_size,  int y_size,  int z_size, subs);

    void RainBlock(int * kind, int * sub) const;

    Block * blocks[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    uchar lightMap[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    const qint64 longitude, latitude;
    int shredX, shredY;
    shred_type type;

    QLinkedList<Active *> activeListFrequent;
    std::forward_list<Active *> activeListAll;
    std::forward_list<Active *> shiningList;
    std::forward_list<Falling *> fallList;
};

#endif // SHRED_H
