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
#include "WaysTree.h"
#include <QThread>
#include <QMutex>
#include <QHash>

class Block;
class Shred;

class World final : public QThread {
    /** \class World world.h
     *  \brief World provides global physics and shred connection.
     *
     * Designed to be single. */
    Q_OBJECT
    Q_DISABLE_COPY(World)

public:

     World(QString world_name, bool* error);
    ~World();

///\name Static information section
///@{
    static World* GetWorld();
    static const World* GetConstWorld();
    static QString WorldName() { return GetConstWorld()->worldName; }
    static QString WorldPath() { return home_path + WorldName(); }

    static dirs TurnRight(dirs dir);
    static dirs TurnLeft (dirs dir);
    static dirs Anti(dirs dir);
    static int GetBound();
///@}

///\name Information section
///@{
    /// Get coordinates near block.
    /** @param[in] x, y, z Coordinates of block.
     *  @param[out] x_targ, y_targ, z_targ Received focus coordinates.
     *  @param[in] dir Direction.
     *  @returns false on error, true if bound check is passed. */
    bool Focus(int x, int y, int z,
            int* x_targ, int* y_targ, int* z_targ, dirs dir) const;
    int NumShreds() const { return numShreds; }
    bool ShredInCentralZone(qint64 longi, qint64  lati) const;
    qint64 Longitude() const;
    qint64 Latitude() const;

    const class WorldMap* GetMap() const;
    class ShredStorage* GetShredStorage();
///@}

///\name Block work section
///@{
    Block* GetBlock(int x, int y, int z) const;
    Shred* GetShred(int i, int j) const;
    Shred* GetShredByPos(int x, int y) const;
    Shred* GetNearShred(Shred*, dirs dir) const;
///@}

///\name Lighting section
///@{
    int Enlightened(int x, int y, int z) const;
    /// Provides lighting of block side, not all block.
    int Enlightened(int x, int y, int z, dirs dir) const;

    /// Makes block emit shining.
    void Shine(const Xyz&, int level);
///@}

    /// Radius 5 ensures that fully enlightened block (in a cloud of most
    /// powerful illuminators) will not overflow lighting storage (8 bit).
    static const int MAX_LIGHT_RADIUS = 5;

///\name Visibility section
///@{
    bool DirectlyVisible(const Xyz& from, const Xyz& to) const;
    /// At least one side of block is visible.
    bool Visible(const Xyz& from, const Xyz& to) const;
///@}

///\name Movement section
///@{
    bool Move(int x, int y, int z, dirs dir); ///< Check and move
    void Jump(int x, int y, int z, dirs dir);
///@}

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

///\name Time section
///@{
    int TimeOfDay() const; ///< Get seconds from start of current day.
    int MiniTime()  const; ///< Get number of time steps since second start.
    quint64 Time()  const; ///< Get time in seconds since world creation.
    QString TimeOfDayStr() const;
    times_of_day PartOfDay() const;
///@}

///\name Interactions section
///@{
    /// Returns damaged block result durability.
    int Damage(int x, int y, int z, int level, int dmg_kind);
    /// Does not check target block durability.
    void DestroyAndReplace(int x, int y, int z);
    /// Returns true on successful build, otherwise false.
    bool Build(Block* thing, int x, int y, int z, Block* who = nullptr);
    /// Returns true on success. Gets a string and inscribes block.
    bool Inscribe(int x, int y, int z);
///@}

///\name Inventory functions section
///@{
    /// Returns true on success.
    bool Drop(Block* from, int x_to, int y_to, int z_to,
            int src, int dest, int num);
    bool Get(Block* to, int x_from, int y_from, int z_from,
            int src, int dest, int num);
///@}

///\name Block information section
///@{
    bool InBounds(int x, int y) const;
    bool InBounds(int x, int y, int z) const;

    int SetNote(QString note);
    int ChangeNote(QString note, int note_id);
    QString GetNote(int note_id) const;
///@}

///\name World section
///@{
    void ReloadAllShreds(QString new_world, qint64 lati, qint64 longi,
            int new_x, int new_y, int new_z);

    void SaveToDisk() const;

    QMutex * GetLock() { return &mutex; }

    void SetReloadShreds(int direction);
    void PhysEvents();
    void ActivateFullReload();
///@}

signals:
    void Notify(QString) const;
    void GetString(QString &) const;
    void UpdatedAll();
    void UpdatedAround(int x, int y, int z);
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
    static const int MIN_WORLD_SIZE;

///\name private Lighting section
///@{
    /// Make all tree shine.
    void Shine(const Xyz&, int level, const WaysTree* tree);

    /// Adds light level to particular coordinate.
    void AddLight(const Xyz&, int level);

    /// Takes back all light in area around coordinates xyz.
    void UnShine(int x, int y, int z, Block* skip_block, Block* add_block);

    /// Updates and restores lighting after UnShine.
    void ReEnlighten();

    /// Updates lighting in all world.
    void ReEnlightenAll();

    /// Called from ReloadShreds(int), enlightens only needed shreds.
    void ReEnlightenMove(dirs);

    /** @brief Checks if lighting should be updated after operation on blocks.
     *  @param block1, block2 blocks to compare.
     *  @param x, y, z coordinates of relighting center.
     *  @param skipBlock1 to skip or not to skip block1 when relighting.
     */
    void ReEnlightenCheck(Block* block1, Block* block2, int x, int y, int z,
                          Block* skip_block, Block* add_block);
///@}

    enum can_move_results {
        CAN_MOVE_OK,
        CAN_MOVE_CANNOT,
        CAN_MOVE_DESTROYED
    };

///\name Private movement section
///@{
    /// This CAN move blocks, but not xyz block.
    can_move_results CanMove(int x,    int y,    int z,
                             int x_to, int y_to, int z_to, dirs dir);
    void NoCheckMove(int x,    int y,    int z,
                     int x_to, int y_to, int z_to, dirs dir);
///@}

    static int CorrectNumShreds(int num);
    static int CorrectNumActiveShreds(int num, int max_num);
    void LoadAllShreds();
    void ReloadShreds();
    void run() override;
    int ShredPos(int x, int y) const;
    Shred** FindShred(int x, int y) const;

    static int Sign(int value);
    /// Returns value1 plus -1 if value1 is bigger than value2, otherwise 1.
    static int DiffSign(int value1, int value2);

    void SaveNotes() const;
    void LoadNotes();
    void SaveState() const;
    void LoadState();

    /// Returns true if block_from can be put into block_to.
    bool Exchange(Block* from, Block* to, int src, int dest, int num);

    QString worldName;
    class WorldMap* map;
    quint64 time;
    int timeStep;
    Shred** shreds;
    /**  N
     *   |
     * W-+->E
     *   |  latitude ( x for shreds )
     * S v longitude ( y for shreds )
     * center of active zone: */
    qint64 longitude, latitude;
    class QSettings* const gameSettings;
    const int numShreds; ///< size of loaded zone
    const int numActiveShreds; ///< size of active zone
    QMutex mutex;
    qint64 newLati, newLongi;
    int newX, newY, newZ;
    QString newWorld;
    /// UP for no reset, DOWN for full reset, N-S-E-W for side shift.
    volatile dirs toResetDir;
    class QTimer* timer;

    class ShredStorage* shredStorage;
    QList<QString> notes;

    /// storage for found shining objects between UnShine and ReEnlighten.
    QHash<class Active*, int> tempShiningList;

    const WaysTree lightWaysTree;

    static World* world;
};

#endif // WORLD_H
