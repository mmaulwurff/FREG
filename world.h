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

#ifndef WORLD_H
#define WORLD_H

#include <QThread>
#include "header.h"

class WorldMap;
class Block;
class Shred;
class ShredStorage;
class QByteArray;
class QReadWriteLock;
class CraftManager;

const int SAFE_FALL_HEIGHT = 5;

const int MOON_LIGHT_FACTOR = 1;
const int  SUN_LIGHT_FACTOR = 8;

class World final : public QThread {
    /** \class World world.h
     * \brief World provides global physics and shred connection.
     *
     * Designed to be single. */
    Q_OBJECT
public:
    explicit World(QString);
    ~World();

public: // Block work section
    Block * GetBlock(int x, int y, int z) const;
    Shred * GetShred(int i, int j) const;
    static void DeleteBlock(Block * block);
private:
    /// Puts block to coordinates xyz and activates it.
    void SetBlock(Block * block, int x, int y, int z);
    /// Puts block to coordinates and not activates it.
    void PutBlock(Block * block, int x, int y, int z);
    static Block * Normal(quint8 sub);
    static Block * NewBlock(int kind, int sub);

    void MakeSun();
    void RemSun();

public: // Lighting section
    int Enlightened(int x, int y, int z) const;
    int Enlightened(int x, int y, int z, int dir) const;
    int SunLight   (int x, int y, int z) const;
    int FireLight  (int x, int y, int z) const;
    int LightMap   (int x, int y, int z) const;

    int ClampX(int x) const;
    int ClampY(int y) const;
    int ClampZ(int z) const;

    void SunShineVertical  (int x, int y, int z = HEIGHT-2,
            int level = MAX_LIGHT_RADIUS);
    void SunShineHorizontal(int x, int y, int z);
    /// If init is false, light will not spread from non-invisible blocks.
    void Shine(int x, int y, int z, int level, bool init);
    void RemoveSunLight(int x, int y, int z);

    bool GetEvernight() const;
private:
    bool SetSunLightMap (int level, int x, int y, int z);
    bool SetFireLightMap(int level, int x, int y, int z);
    void AddFireLight   (int x, int y, int z, int level);
    void RemoveFireLight(int x, int y, int z);

    /// Called when block is moved.
    void ReEnlighten(int x, int y, int z);
    /// Called when block is built.
    void ReEnlightenBlockAdd(int x, int y, int k);
    /// Called when block is destroyed.
    void ReEnlightenBlockRemove(int x, int y, int k);
    void ReEnlightenAll();
    void ReEnlightenTime();
    /// Called from ReloadShreds(int), enlightens only needed shreds.
    void ReEnlightenMove(int direction);
    void UpShine(int x, int y, int z_bottom);
    void CrossUpShine(int x, int y, int z_bottom);

public: // Information section
    QString WorldName() const;
    /// True on error, false if focus is received to _targ successfully.
    bool Focus(int x, int y, int z,
            int * x_targ, int * y_targ, int * z_targ, quint8 dir) const;
    int NumShreds() const;
    static quint8 TurnRight(quint8 dir);
    static quint8 TurnLeft (quint8 dir);
    static quint8 Anti(quint8 dir);
    long GetSpawnLongi() const;
    long GetSpawnLati()  const;
    long Longitude() const;
    long Latitude() const;
    static int TimeStepsInSec();

    char TypeOfShred(long longi, long lati);
    long MapSize() const;

    QByteArray * GetShredData(long longi, long lati) const;
    void SetShredData(QByteArray *, long longi, long lati);
private:
    int SunMoonX() const;
    int ShredPos(int x, int y) const;

public: // Visibility section
    bool DirectlyVisible(float x_from, float y_from, float z_from,
                         int   x_to,   int   y_to,     int   z_to) const;
    bool Visible(int x_from, int y_from, int z_from,
                 int x_to,   int y_to,   int z_to) const;
private:
    bool PositiveVisible(float x_from, float y_from, float z_from,
                         int   x_to,   int   y_to,   int   z_to) const;
    bool NegativeVisible(float x_from, float y_from, float z_from,
                         int   x_to,   int   y_to,   int   z_to) const;

public: // Movement section
    /// Check and move
    bool Move(int x, int y, int z, quint8 dir);
    /// This CAN move blocks, but not xyz block.
    bool CanMove(int x,    int y,    int z,
                 int x_to, int y_to, int z_to, quint8 dir);
    void Jump(int x, int y, int z, quint8 dir);
private:
    void NoCheckMove(int x,    int y,    int z,
                     int x_to, int y_to, int z_to, quint8 dir);

public: // Time section
    int PartOfDay() const;
    /// This returns seconds from start of current day.
    int TimeOfDay() const;
    /// Returns time in seconds since world creation.
    ulong Time() const;
    QString TimeOfDayStr() const;
    /// Returns number of physics steps since second start.
    int MiniTime() const;

public: // Interactions section
    /// Returns damaged block result durability.
    int Damage(int x, int y, int z, int level, int dmg_kind);
    /// Does not check target block durability.
    void DestroyAndReplace(int x, int y, int z);
    bool Build(Block * thing, int x, int y, int z,
            quint8 dir = UP,
            Block * who = nullptr,
            bool anyway = false);
    /// Returns true on success. Gets a string and inscribes block.
    bool Inscribe(int x, int y, int z);
    CraftManager * GetCraftManager() const;

private: // Inventory functions section
    void Exchange(Block * block_from, Block * block_to,
            int src, int dest, int num);
public:
    void Drop(Block * from, int x_to, int y_to, int z_to,
            int src, int dest, int num);
    void Get(Block * to, int x_from, int y_from, int z_from,
            int src, int dest, int num);

public: // Block information section
    static bool InVertBounds(int z);
    bool InBounds(int x, int y) const;
    bool InBounds(int x, int y, int z) const;
    int  Temperature(int x, int y, int z) const;
private:
    static bool IsPile(const Block *);

public: // World section
    void ReloadAllShreds(long lati, long longi,
            int new_x, int new_y, int new_z);
private:
    void SetNumActiveShreds(int num);
    /// Also saves all shreds.
    void DeleteAllShreds();
    void LoadAllShreds();
    void ReloadShreds(int direction);
    void run();
    Shred ** FindShred(int x, int y) const;

public:
    QReadWriteLock * GetLock() const;
    void WriteLock();
    void ReadLock();
    bool TryReadLock();
    void Unlock();

public slots:
    void CleanAll();
    void PhysEvents();
    void SetReloadShreds(int direction);

signals:
    void Notify(QString) const;
    void GetString(QString &) const;
    void Updated(int, int, int);
    void UpdatedAll();
    void UpdatedAround(int x, int y, int z, int range);
    /// Emitted when world active zone moved to int direction.
    void Moved(int);
    void ReConnect();
    /// This is emitted when a pack of updates is complete.
    void UpdatesEnded();
    void NeedPlayer(int, int, int);
    void StartReloadAll();
    void FinishReloadAll();
    void ExitReceived();

private:
    static const int TIME_STEPS_IN_SEC = 10;

    ulong time;
    int timeStep;
    Shred ** shreds;
    /**   N
     *    |  E
     * W--+--> latitude ( x for shreds )
     *    |
     *  S v longitude ( y for shreds )
     * center of active zone: */
    long longitude, latitude;
    long spawnLongi, spawnLati;
    const QString worldName;
    int numShreds; ///< size of loaded zone
    int numActiveShreds; ///< size of active zone
    QReadWriteLock * const rwLock;

    int sunMoonX;
    /// stores block behind sun or moon (normal with sub STAR or SKY)
    Block * behindSun;
    bool evernight;

    WorldMap * const map;

    long newLati, newLongi;
    int newX, newY, newZ;
    /// UP for no reset, DOWN for full reset, NSEW for side shift.
    volatile int toResetDir;

    int sunMoonFactor;

    ShredStorage * shredStorage;
    Shred * shredMemoryPool;
    CraftManager * const craftManager;
};

extern World * world;

#endif // WORLD_H
