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

#ifndef WORLD_H
#define WORLD_H

#include "header.h"
#include <QThread>
#include <QMutex>

class Block;
class Shred;

class World final : public QThread {
    /** \class World world.h
     * \brief World provides global physics and shred connection.
     *
     * Designed to be single. */
    Q_OBJECT
    Q_DISABLE_COPY(World)
public:
     World(QString world_name, bool * error);
    ~World();

     static World * GetWorld();

public: // Block work section
    Block * GetBlock(int x, int y, int z) const;
    Shred * GetShred(int i, int j) const;
    Shred * GetShredByPos(int x, int y) const;
    Shred * GetNearShred(Shred *, dirs dir) const;

public: // Lighting section
    int Enlightened(int x, int y, int z) const;
    int Enlightened(int x, int y, int z, dirs dir) const;
    int SunLight   (int x, int y, int z) const;
    int FireLight  (int x, int y, int z) const;
    int LightMap   (int x, int y, int z) const;

    void SunShineVertical(int x, int y);

    /// Makes block emit shining.
    /** Receives only non-sun light as level, from 1 to F. */
    void Shine(const class Xyz&, int level);

    bool GetEvernight() const { return evernight; }

    static const int FIRE_LIGHT_FACTOR = 4;
private:
    bool SetFireLightMap(int level, int x, int y, int z);
    void AddFireLight   (int x, int y, int z, int level);
    void RemoveFireLight(int x, int y, int z);

    /// Called when block is moved.
    void ReEnlighten(int x, int y, int z);
    void ReEnlightenAll();
    void ReEnlightenTime();
    /// Called from ReloadShreds(int), enlightens only needed shreds.
    void ReEnlightenMove(dirs);
    void UpShine(int x, int y, int z_bottom);
    void UpShineInit(int x, int y, int z_bottom);
    void CrossUpShine(int x, int y, int z_bottom);

    static const int MOON_LIGHT_FACTOR = 1;
    static const int  SUN_LIGHT_FACTOR = 8;
    static const int MAX_LIGHT_RADIUS = 15;

public: // Information section
    static QString WorldName() { return GetWorld()->worldName; }
    static QString WorldPath() { return home_path + WorldName(); }
    /// False on error, true if focus is received to _targ successfully.
    bool Focus(int x, int y, int z,
            int * x_targ, int * y_targ, int * z_targ, dirs dir) const;
    int NumShreds() const { return numShreds; }
    bool ShredInCentralZone(qint64 longi, qint64  lati) const;
    static dirs TurnRight(dirs dir);
    static dirs TurnLeft (dirs dir);
    static dirs Anti(dirs dir);
    qint64 Longitude() const;
    qint64 Latitude() const;

    const class WorldMap * GetMap() const;

    class QByteArray * GetShredData(qint64 longi, qint64 lati) const;
    void SetShredData(class QByteArray *, qint64 longi, qint64 lati);
private:
    int ShredPos(int x, int y) const;

public: // Visibility section
    bool DirectlyVisible(int x_from, int y_from, int z_from,
                         int x_to,   int y_to,   int z_to) const;
    /// At least one side of block is visible.
    bool Visible(int x_from, int y_from, int z_from,
                 int x_to,   int y_to,   int z_to) const;

public: // Movement section
    /// Check and move
    bool Move(int x, int y, int z, dirs dir);
    void Jump(int x, int y, int z, dirs dir);
private:
    enum can_move_results {
        CAN_MOVE_OK,
        CAN_MOVE_CANNOT,
        CAN_MOVE_DESTROYED
    };

    /// This CAN move blocks, but not xyz block.
    can_move_results CanMove(int x,    int y,    int z,
                             int x_to, int y_to, int z_to, dirs dir);
    void NoCheckMove(int x,    int y,    int z,
                     int x_to, int y_to, int z_to, dirs dir);

public: // Time section
    enum times {
        SECONDS_IN_HOUR = 60,
        SECONDS_IN_DAY  = 24 * SECONDS_IN_HOUR,
        END_OF_NIGHT    =  6 * SECONDS_IN_HOUR,
        END_OF_MORNING  = 12 * SECONDS_IN_HOUR,
        END_OF_NOON     = 18 * SECONDS_IN_HOUR,
        END_OF_EVENING  =  0 * SECONDS_IN_HOUR,
        SECONDS_IN_NIGHT    = END_OF_NIGHT,
        SECONDS_IN_DAYLIGHT = SECONDS_IN_DAY-END_OF_NIGHT
    };

    times_of_day PartOfDay() const;
    /// This returns seconds from start of current day.
    int TimeOfDay() const;
    /// Returns time in seconds since world creation.
    quint64 Time() const;
    QString TimeOfDayStr() const;
    /// Returns number of physics steps since second start.
    int MiniTime() const;

public: // Interactions section
    /// Returns damaged block result durability.
    int Damage(int x, int y, int z, int level, int dmg_kind);
    /// Does not check target block durability.
    void DestroyAndReplace(int x, int y, int z);
    /// Returns true on successfull build, otherwise false.
    bool Build(Block * thing, int x, int y, int z,
            int dir = UP,
            Block * who = nullptr,
            bool anyway = false);
    /// Returns true on success. Gets a string and inscribes block.
    bool Inscribe(int x, int y, int z);

private: // Inventory functions section
    /// Returns true if block_from can be put into block_to.
    bool Exchange(Block * block_from, Block * block_to,
            int src, int dest, int num);
public:
    /// Returns true on success.
    bool Drop(Block * from, int x_to, int y_to, int z_to,
            int src, int dest, int num);
    bool Get(Block * to, int x_from, int y_from, int z_from,
            int src, int dest, int num);

public: // Block information section
    bool InBounds(int x, int y) const;
    bool InBounds(int x, int y, int z) const;
    static int GetBound();

    int SetNote(QString note);
    int ChangeNote(QString note, int note_id);
    QString GetNote(int note_id) const;
private:
    void SaveNotes() const;
    void LoadNotes();
    void SaveState() const;
    void LoadState();

public: // World section
    void ReloadAllShreds(QString new_world, qint64 lati, qint64 longi,
            int new_x, int new_y, int new_z);

    void SaveToDisk() const;

    QMutex * GetLock() { return &mutex; }

    void SetReloadShreds(int direction);
    void PhysEvents();
    void ActivateFullReload();

signals:
    void Notify(QString) const;
    void GetString(QString &) const;
    void Updated(int, int, int);
    void UpdatedAll();
    void UpdatedAround(int x, int y, int z, int range);
    void StartMove(int);
    /// Emitted when world active zone moved to int direction.
    void Moved(int);
    /// This is emitted when a pack of updates is complete.
    void UpdatesEnded();
    void NeedPlayer(int, int, int);
    void StartReloadAll();
    void FinishReloadAll();
    void ExitReceived();

    void Pause();
    void Start();

private:
    static const int TIME_STEPS_IN_SEC = 10;
    static const int MIN_WORLD_SIZE    =  5;

    static int CorrectNumShreds(int num);
    static int CorrectNumActiveShreds(int num, int max_num);
    void LoadAllShreds();
    void ReloadShreds();
    void run() override;
    Shred ** FindShred(int x, int y) const;
    static unsigned Abs(int x);

    QString worldName;
    class WorldMap * map;
    quint64 time;
    int timeStep;
    Shred ** shreds;
    /**   N
     *    |  E
     * W--+--> latitude ( x for shreds )
     *    |
     *  S v longitude ( y for shreds )
     * center of active zone: */
    qint64 longitude, latitude;
    class QSettings * const gameSettings;
    const int numShreds; ///< size of loaded zone
    const int numActiveShreds; ///< size of active zone
    QMutex mutex;
    bool evernight;
    qint64 newLati, newLongi;
    int newX, newY, newZ;
    QString newWorld;
    /// UP for no reset, DOWN for full reset, NSEW for side shift.
    volatile dirs toResetDir;
    int sunMoonFactor;
    class QTimer * timer;

    class ShredStorage * shredStorage;
    bool initial_lighting;
    QList<QString> notes;

    static World * world;
};

#endif // WORLD_H
