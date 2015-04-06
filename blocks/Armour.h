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

#ifndef ARMOUR_H
#define ARMOUR_H

#include "blocks/Block.h"

class Armour : public Block {
public:
    using Block::Block;
    void Damage(int dmg, int dmg_kind) override;
    int  DamageLevel() const override;
    wearable Wearable() const override;

protected:
    static const int THRESHOLD = 10;
}; // class Armour

class Helmet : public Armour {
public:
    using Armour::Armour;
    wearable Wearable() const override;
}; // class Helmet

class Boots : public Armour {
public:
    using Armour::Armour;
    wearable Wearable() const override;
}; // class Boots

#endif // ARMOUR_H
