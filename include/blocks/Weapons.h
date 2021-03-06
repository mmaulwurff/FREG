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

#ifndef WEAPONS_H
#define WEAPONS_H

#include "blocks/Active.h"

/** Weapon class represents simple weapons as sticks, pebbles and so on.
 *  Also is used as base class for more special weapons.
 *  Weapon of SKY substance is abyss block, everything that touches abyss
 *  will be destroyed. */
class Weapon : public Falling {
public:

    BLOCK_CONSTRUCTORS(Weapon)

    int  DamageKind() const override;
    int  DamageLevel() const override;
    int  Weight() const override;
    QString FullName() const override;
    QString Description() const override;
    wearable Wearable() const override;
    push_reaction PushResult(dirs) const override;
};

class Pick : public Weapon {
public:
    BLOCK_CONSTRUCTORS(Pick)

    int  DamageKind() const override;
};

class Shovel : public Weapon {
public:
    BLOCK_CONSTRUCTORS(Shovel)

    int  DamageKind() const override;
};

class Hammer : public Weapon {
public:
    BLOCK_CONSTRUCTORS(Hammer)

    int  DamageKind() const override;
};

class Axe : public Weapon {
public:
    BLOCK_CONSTRUCTORS(Axe)

    int  DamageKind() const override;
};

#endif // WEAPONS_H
