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

#ifndef SHRED_H
#define SHRED_H

#include <QLinkedList>
#include "header.h"

class World;
class Block;
class Active;

enum weathers {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_DEW,
    WEATHER_CLOUDS
};

class Shred final {
public:
    Shred(int shred_x, int shred_y,
            long longi, long lati,
            Shred * memory);
    ~Shred();

    Shred & operator=(const Shred &) = delete;

    /// Returns y (line) shred coordinate on world map.
    long Longitude() const;
    /// Returns x (column) shred coordinate on world map.
    long Latitude()  const;
    int ShredX() const;
    int ShredY() const;
    void PhysEventsFrequent();
    void PhysEventsRare();
    void DeleteDestroyedActives();

    void Register(Active *);
    void UnregisterLater(Active *);
    void AddShining(Active *);
    void RemShining(Active *);
    void AddToDelete(Active *);
    void AddFalling(int x, int y, int z);

    QLinkedList<Active * const>::const_iterator ShiningBegin() const;
    QLinkedList<Active * const>::const_iterator ShiningEnd() const;

    World * GetWorld() const;

    void ReloadToNorth();
    void ReloadToEast();
    void ReloadToSouth();
    void ReloadToWest();

    Block * GetBlock(int x, int y, int z) const;
    /// Removes last block at xyz, then SetBlock, then makes block normal.
    void SetBlock(Block * block, int x, int y, int z);
    /// Puts block to coordinates xyz and activates it.
    void SetBlockNoCheck(Block *, int x, int y, int z);
    /// Puts block to coordinates, not activates it.
    void PutBlock(Block * block, int x, int y, int z);
    static Block * Normal(int sub);

    // Lighting section
    int Lightmap(  int x, int y, int z) const;
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
    char TypeOfShred(long longi, long lati) const;
    char GetTypeOfShred() const;

    static QString FileName(QString world_name, long longi, long lati);
    Shred * GetShredMemory() const;
    /// Make global coordinate from local (in loaded zone).
    long GlobalX(int x) const;
    long GlobalY(int y) const;
    static int CoordInShred(const int x);
    static int CoordOfShred(const int x);

    // Weather
    void SetWeathers();
    weathers GetWeather(times_of_day time) const;
    void Rain();
    void Dew();

private:
    void RemoveAllSunLight();
    void RemoveAllFireLight();
    void RemoveAllLight();

    void AddFalling(Active *);
    void Unregister(Active *);
    void UnregisterExternalActives();

    bool LoadShred();

    QString FileName() const;

    void NormalUnderground(int depth = 0, int sub = SOIL);
    void CoverWith(int kind, int sub);
    /// Puts num things(kind-sub) in random places on shred surface.
    /** If on_water is false, this will not drop things on water,
     *  otherwise on water too. */
    void RandomDrop(int num, int kind, int sub, bool on_water = false);
    int CountShredTypeAround(int type) const;

    void PlantGrass();
    void TestShred();
    void NullMountain();
    void Plain();
    void Forest();
    void Water();
    void Pyramid();
    void Mountain();
    void Hill();
    void Desert();
    void Castle();
    /// For testing purposes.
    void ChaosShred();

    /// Block combinations section (trees, buildings, etc):
    bool Tree(int x, int y, int z, int height);

    /// Special land generation
    void ShredLandAmplitudeAndLevel(long longi, long lati,
            ushort * l, float * a) const;
    void ShredNominalAmplitudeAndLevel(char shred_type,
            ushort * l, float * a) const;
    void AddWater();
    int FlatUndeground(int depth = 0);
    void NormalCube(int x_start, int y_start, int z_start,
            int x_size, int y_size, int z_size, int sub);

    /// Lowest nullstone and sky are not in bounds.
    static bool InBounds(int x, int y, int z);

    Block * blocks[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    uchar lightMap[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    const long longitude, latitude;
    int shredX, shredY;
    char type;

    /// Contains all active blocks.
    QLinkedList<Active * const> activeListAll;
    QLinkedList<Active * const> activeListFrequent;
    QLinkedList<Active * const> fallList;
    QLinkedList<Active * const> shiningList;
    QLinkedList<Active * const> deleteList;
    QLinkedList<Active * const> unregisterList;

    /// memory, allocated for this shred.
    Shred * const memory;

    weathers weather[NIGHT+1]; // NIGHT is the last time of day.
};

#endif // SHRED_H
