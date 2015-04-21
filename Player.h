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

#ifndef PLAYER_H
#define PLAYER_H

#include "Xyz.h"
#include "header.h"
#include <QObject>

class Player final : public QObject, private Xyz {
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
    Q_DISABLE_COPY(Player)
public:
    /// Constructor creates or loads player.
    /** It puts player block to the world if there is no player block,
     *  and makes necessary connections. */
     Player();
    ~Player();

    qint64 GlobalX() const;
    qint64 GlobalY() const;

    int X() const;
    int Y() const;
    using Xyz::Z;

    /// This returns current player direction (see enum dirs in header.h)
    dirs GetDir() const;

    /// This returns player breath reserve. On error returns -100.
    int BreathPercent() const;

    /// Can be > 100 if player is gorged. On error returns -100.
    int SatiationPercent() const;

    class Block* GetBlock() const;
    const class Block* GetConstBlock() const;

    enum visible {
        VISIBLE,
        IN_SHADOW,
        OBSCURED
    };

    /// Get block at (x, y, z) visibility to player.
    visible Visible(int x, int y, int z) const;

    /// This returns how player is using something now.
    /** See enum usage_types in header.h. */
    int UsingType() const { return usingType; }
    int GetUsingInInventory() const;
    void SetUsingTypeNo() { usingType = USAGE_TYPE_NO; }
    /// This returns how player is using himself.
    /** For example, OPEN means he is looking in his backpack.
     *  See enum usage_types in header.h. */
    int UsingSelfType() const { return usingSelfType; }

    /// Returns nullptr if there is no inventory, otherwise returns inventory.
    class Inventory* PlayerInventory() const;

    qint64 GetLongitude() const;
    qint64 GetLatitude() const;

    bool GetCreativeMode() const { return creativeMode; }
    void SetCreativeMode(bool turn);

    void SetDir(dirs);
    void Move(dirs);
    void Jump();

    void Notify(const QString&) const;

    /// Closes backpack, chests, etc.
    void StopUseAll();
    /// Tries to switch usingSelfType from NO to OPEN.
    void Backpack();
    void Inscribe() const;
    void Examine() const;
    void Examine(int x, int y, int z) const;
    void Examine(const class Block*) const;
    /// Returns true if xyz are in world bounds.
    bool Damage() const;
    void Use();
    /// Tries to throw (drop out) block number num from inventory.
    void Throw(int src, int dest = 0, int num = 1);
    void TurnBlockToFace() const;

    /// Tries to use block number num in inventory.
    usage_types Use(int num);
    /// Tries to get block number num from outer inventory.
    //  Returns true on success.
    bool Obtain(int src, int dest = 0, int num = 1);
    void Inscribe(int num);
    void Craft   (int num);
    void Build   (int num);
    void Wield   (int num);
    /// Can also wield appropriate things.
    void MoveInsideInventory(int num_from, int num_to, int num = 1);
    void ProcessCommand(QString command);

    /// Turns low-case ASCII chars array (length <= 12) into unique quint64.
    /// Can be used as switch statement for switch on strings.
    constexpr static quint64 UniqueIntFromString(const char* const chars ) {
        return chars[0] == '\0' ?
            0 : (UniqueIntFromString(chars + 1) << 5) | (chars[0]-'a');
    }

    void SaveState() const;

signals:
    /// Can be used to track player movements. x and y are global.
    void Moved(qint64 x, qint64 y, int z) const;

    /// This is emitted when player walks over shred border.
    /** It should be connected to World::ReloadShreds(int) signal. */
    void OverstepBorder(int);

    /// This is emitted when some player property is updated.
    /** It should be connected to screen::UpdatePlayer() signal. */
    void Updated();
    void GetString(QString&);
    void Destroyed();
    void ShowFile(QString path);
    void GetFocus(int* x, int* y, int* z) const;

public:
    /// Checks if player walked over the shred border.
    /** This is connected to player's block signal Moved(int).
     *  It emits OverstepBorder(int) signal when player walks
     *  over the shred border. */
    void CheckOverstep(int dir);

    /// This is called when player block is destroyed.
    void BlockDestroy();

    void Disconnect();
    void SetPlayer(int set_x, int set_y, int set_z);
    void UpdateXYZ();

private:
    //static const subs PLAYER_SUB = ADAMANTINE;
    static const subs PLAYER_SUB = H_MEAT;

    /// Checks player/inventory/block existence, size limits.
    class Block* ValidBlock(int num) const;
    class Shred* GetShred() const;
    bool ForbiddenAdminCommands() const;
    void ProcessGetCommand(class QTextStream&);

    void LoadState();

    qint64 longitude, latitude;
    qint64 homeLongi, homeLati;
    int homeX, homeY, homeZ;
    class Animal* player, * const creator;
    int usingType;
    int usingSelfType;
    int usingInInventory;
    bool creativeMode;
    bool cleaned = false;
};

#endif // PLAYER_H
