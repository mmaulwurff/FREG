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

#ifndef DEFERRED_ACTION_H
#define DEFERRED_ACTION_H

#include "Xyz.h"

/** Deferred Action is used when some action needs to be done at
 *  next physics turn.
 *  Xyz is action target coordinates. */
class DeferredAction final : private Xyz {
public:
    explicit DeferredAction(class Animal* attached);

    void SetGhostMove(int direction);
    void SetMove(int direction);
    void SetJump();
    void SetDamage (int x, int y, int z);
    void SetPour   (int x, int y, int z, int slot);
    void SetSetFire(int x, int y, int z);
    /// Attached block should have inventory.
    void SetBuild  (int x, int y, int z, int builder_slot);
    void SetThrow  (int x, int y, int z,
            int source_slot, int destination_slot, int number);

    int  GetActionType() const;
    void MakeAction();

private:
    enum deferred_actions {
        DEFERRED_NOTHING,
        DEFERRED_GHOST_MOVE,
        DEFERRED_MOVE,
        DEFERRED_JUMP,
        DEFERRED_BUILD,
        DEFERRED_DAMAGE,
        DEFERRED_THROW,
        DEFERRED_POUR,
        DEFERRED_SET_FIRE
    };

    DeferredAction(const DeferredAction&) = delete;
    DeferredAction& operator=(const DeferredAction&) = delete;

    deferred_actions type;
    int srcSlot, destSlot;
    int num;
    class Animal* const attachedBlock;

    void GhostMove() const;
    void Move() const;
    void Jump() const;
    void Build() const;
    void Damage() const;
    void Throw() const;
    void Pour() const;
    void SetFire() const;
};

#endif // DEFERRED_ACTION_H
