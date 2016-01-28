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

class Block;
class Active;
class Falling;

enum to_dirs {
    TO_NORTH_WEST, TO_NORTH, TO_NORTH_EAST,
    TO_WEST,       TO_HERE,  TO_EAST,
    TO_SOUTH_WEST, TO_SOUTH, TO_SOUTH_EAST,
};

class AroundShredTypes {
public:
    AroundShredTypes(qint64 longitude, qint64 latitude);

    char To(to_dirs toDir) const;
    int Count(char type) const;

private:
    char types[9];
}; // class AroundShredTypes

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

    void Register(Active*);
    void Unregister(Active*);
    void AddShining(Active*);
    void RemShining(Active*);
    void RemFalling(Falling*);
    void ReloadTo(dirs);

    const std::forward_list<Active*>& GetShiningList() const;

    void SaveShred(bool isQuitGame);

    Block* GetBlock(int x, int y, int z) const;
    const Block* FindFirstVisible(int x, int y, int* z, int step) const;

    /// Removes last block at xyz, then SetBlock, then makes block normal.
    void SetBlock(Block* block, int x, int y, int z);
    /// Puts block to coordinates xyz and activates it.
    void PutBlockAndRegister(Block*, int x, int y, int z);

    /** If block block at xyz is normal, replace it with the same but abnormal.
     *  @returns resulting block at xyz. */
    Block* GetModifiableBlock(int x, int y, int z) const;
    void PutModifiedBlock(Block*&, int x, int y, int z);

    void AddFalling(Block*);

    /// Puts block to coordinates, not activates it.
    void PutBlock(Block* block, int x, int y, int z);

    /** @name Lighting section */ ///@{
        /// Get light level in coordinates x, y, z.
        int LightMap(int x, int y, int z) const;

        /// Sets sun light to level in coordinates xyz.
        void AddLight(int x, int y, int z, int level);

        /// Make all shining block shine.
        void ShineAll() const;

        void UpdateSkyLight();
    ///@}

    /** @name Information section */ ///@{
        shred_type GetTypeOfShred() const { return type; }

        static QString FileName(qint64 longi, qint64 lati);
        Shred* GetShredMemory() const;
        /// Make global coordinate from local (in loaded zone).
        qint64 GlobalX(int x) const;
        qint64 GlobalY(int y) const;
        /// Get local coordinate.
        static int CoordInShred(const int x) { return x & 0b1111; }

        /// Get shred coordinate in loaded zone (from 0 to numShreds).
        static int CoordOfShred(const int x) {return x>>SHRED_WIDTH_BITSHIFT;}

        /// Lowest nullstone and sky are not in bounds.
        static bool InBounds(int x, int y, int z);
        static bool InBounds(int x, int y);
        static bool InBounds(int z);
    ///@}

    void Rain(kinds, subs);
    void Dew (kinds, subs);

private:
    Q_DISABLE_COPY(Shred)

    static const quint8 DATASTREAM_VERSION;
    static const quint8 CURRENT_SHRED_FORMAT_VERSION = 17;

    static const int RAIN_IS_DEW = 1;

    bool LoadShred();
    void GenerateShred();
    void RegisterInit(Active*);

    /// Builds normal underground. Returns ground level.
    int NormalUnderground(int depth = 0, subs sub = SOIL);
    void CoverWith(kinds, subs);
    /** Puts num things(kind-sub) in random places on shred surface.
     *  If on_water is false, this will not drop things on water,
     *  otherwise on water too. */
    void RandomDrop(int num, kinds, subs, bool on_water = false);
    void DropBlock(Block*, bool on_water);
    int FindTopNonAir(int x, int y);
    int FindTopTransparent(int x, int y, int z = HEIGHT - 1);

    void SetNewBlock(kinds, subs, int x, int y, int z);

    void PlantGrass();
    void TestShred();
    void NullMountain();
    void Plain();
    void Forest(bool dead);
    void Water(subs = WATER);
    void Pyramid();
    void Mountain();
    void Hill(bool dead);
    void Desert();
    void Castle();
    void WasteShred();
    void Layers();
    /// For testing purposes.
    void ChaosShred();

    void ReloadHeightMap();

    /** Loads room from corresponding .room or -index.room file.
     * Should be placed before any other block generation at the same place. */
    bool LoadRoom(int level, int index = 0);

    /** @name Block combinations section (trees, buildings, etc): */ ///@{
        bool Tree(int x, int y, int z);
        void NormalCube(int x_start, int y_start, int z_start,
                        int x_size,  int y_size,  int z_size, subs);
    ///&}

    void RainBlock(int* kind, int* sub) const;

    static Block* Normal(subs sub);
    static int rand();

    class CoordinateIterator {
    public:
        void operator++();
        int X() const;
        int Y() const;
        bool notEnd() const;
    private:
        int x = 0, y = 0;
    };

    class InitialBlockColumn {
    public:
        InitialBlockColumn();
        const Block* const* GetColumn() const;
    private:
        Block* pattern[HEIGHT];
    };

    Block*  blocks[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    uchar lightMap[SHRED_WIDTH][SHRED_WIDTH][HEIGHT];
    const qint64 longitude, latitude;
    int shredX, shredY;
    shred_type type;

    QLinkedList<Active*> activeListFrequent;
    std::forward_list<Active*> activeListAll;
    std::forward_list<Active*> shiningList;
    std::forward_list<class Falling*> fallList;
};

#endif // SHRED_H
