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

#ifndef DEFERRED_ACTION_H
#define DEFERRED_ACTION_H

#include "Xyz.h"

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

class Block;
class Active;
class World;

class DeferredAction final : private Xyz {
    /** \class DeferredAction DeferredAction.h
     *  \brief Deferred Action is used when some action needs to be done at
     *  next physics turn.
     *
     *  Xyz is action target coordinates. */
public:
    explicit DeferredAction(Active * attached);
    DeferredAction & operator=(const DeferredAction &) = delete;

    void SetGhostMove(ushort dir = NOWHERE);
    void SetMove(ushort dir = NOWHERE);
    void SetJump();
    void SetDamage (short x, short y, short z);
    void SetPour   (short x, short y, short z, ushort slot);
    void SetSetFire(short x, short y, short z);
    void SetBuild  (short x, short y, short z,
            Block * material, ushort builder_slot);
    void SetThrow  (short x, short y, short z,
            ushort src_slot, ushort dest_slot, ushort unum);

    World * GetWorld() const;
    int  GetActionType() const;
    void MakeAction();

private:
    deferred_actions type;
    Active * const attachedBlock;
    Block * material;
    ushort srcSlot, destSlot;
    ushort num;

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
