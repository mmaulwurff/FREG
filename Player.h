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

#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "header.h"
#include "Xyz.h"

class QString;
class World;
class Block;
class Active;
class Inventory;
class Shred;

class Player final : public QObject, public Xyz {
    /** \class Player Player.h
     * \brief This class contains information specific to player
     * and interface for manipulating him.
     *
     * It receives input from screen, processes it, and dispatches
     * actions to world.
     *
     * It loads player from file and saves him to it.
     *
     * Note: player block with its inventory is stored it shred
     * file with all other blocks. player_save contains only
     * coordinates where player is, his home coordinates and world
     * player belongs to.
     *
     * Also it does checks for player walking over the shred border. */
    Q_OBJECT
public:
    /// Constructor creates or loads player.
    /** It puts player block to the world if there is no player block,
     *  and makes necessary connections. */
    Player();
    /// Destructor calls Player::CleanAll().
    ~Player();

    long GlobalX() const;
    long GlobalY() const;

    /// If player is not dwarf, false is always returned.
    bool IsRightActiveHand() const;
    /// If player is not dwarf, 0 is returned.
    int  GetActiveHand() const;
    /// If player is not dwarf, does nothing.
    void SetActiveHand(bool right);

    /// This returns current player direction (see enum dirs in header.h)
    dirs GetDir() const;

    /// This returns player hitpoints, also known as durability.
    int HP() const;
    /// This returns player breath reserve.
    int Breath() const;
    int BreathPercent() const;
    int Satiation() const;
    /// Can be > 100 if player is gorged. When player is not animal, 50.
    int SatiationPercent() const;

    bool IfPlayerExists() const;

    /// This returns true if block at (x, y, z) is visible to player.
    bool Visible(int x, int y, int z) const;

    /// This returns how player is using something now.
    /** See enum usage_types in header.h. */
    int UsingType() const;
    int GetUsingInInventory() const;
    void SetUsingTypeNo();
    /// This returns how player is using himself.
    /** For example, OPEN means he is looking in his backpack.
     *  See enum usage_types in header.h. */
    int UsingSelfType() const;

    /// Returns 0 if there is no inventory, otherwise returns inventory.
    Inventory * PlayerInventory() const;

    long GetLongitude() const;
    long GetLatitude() const;

    bool GetCreativeMode() const;
    void SetCreativeMode(bool turn);

    void SetDir(dirs);
    void Move(dirs);
    void Jump();

    /// Closes backpack, chests, etc.
    void StopUseAll();
    /// Tries to switch usingSelfType from NO to OPEN.
    void Backpack();
    void Inscribe() const;
    void Examine() const;
    /// Returns true if xyz are in world bounds.
    bool Damage() const;
    void Use();
    /// Tries to throw (drop out) block number num from inventory.
    void Throw(int src, int dest = 0, int num = 1);

    /// Tries to use block number num in inventory.
    usage_types Use(int num);
    /// Tries to get block number num from outer inventory.
    void Obtain(int src, int dest = 0, int num = 1);
    void Wield   (int num);
    void Inscribe(int num);
    void Eat     (int num);
    void Craft   (int num);
    void TakeOff (int num);
    void Build(int num);
    /// Can also wield appropriate things.
    void MoveInsideInventory(int num_from, int num_to, int num=1);
    void ProcessCommand(QString command);

signals:
    void Moved(long x, long y, int z) const;
    /// This is emitted when a notification is needed to be displayed.
    /** It should be connected to screen::Notify(QString). */
    void Notify(QString) const;

    /// This is emitted when player walks over shred border.
    /** It should be connected to World::ReloadShreds(int) signal. */
    void OverstepBorder(int);

    /// This is emitted when some player property is updated.
    /** It shoul be connected to screen::UpdatePlayer() signal. */
    void Updated();
    void GetString(QString &);
    void Destroyed();
    void ShowFile(QString path);
    void GetFocus(int * x, int * y, int * z) const;

private slots:
    /// For cleaning player-related data before exiting program.
    /** This is connected to app's aboutToQuit() signal, also it
     *  is called from destructor. There is a check for data deleting
     *  not to be called twice.
     *  Also this saves player to file player_save. */
    void CleanAll();

    /// Checks if player walked over the shred border.
    /** This is connected to player's block signal Moved(int).
     *  It emits OverstepBorder(int) signal when player walks
     *  over the shred border. */
    void CheckOverstep(int dir);

    /// This is called when player block is destroyed.
    void BlockDestroy();

    void SetPlayer(int set_x, int set_y, int set_z);
    /// Dir is not used, for slot signature compatibility only.
    void UpdateXYZ(int dir = NOWHERE);

private:
    usage_types UseNoLock(int num);
    void InnerMove(int num_from, int num_to, int num = 1);
    /// Checks player existence, inventory existence, size limits,
    /// block existence.
    Block * ValidBlock(int num) const;
    Shred * GetShred() const;
    World * GetWorld() const;

    long homeLongi, homeLati;
    int homeX, homeY, homeZ;
    dirs dir = NORTH;
    Active * player;
    int usingType     = USAGE_TYPE_NO;
    int usingSelfType = USAGE_TYPE_NO;
    int usingInInventory;
    bool creativeMode;
    bool cleaned = false;
};

#endif // PLAYER_H
