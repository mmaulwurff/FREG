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

#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include "blocks/Block.h"

class Accumulator : public Block {
    M_DISABLE_COPY(Accumulator)
public:
    BLOCK_CONSTRUCTORS(Accumulator)

    QString FullName() const override;
    void Damage(int dmg, int dmg_kind) override;

protected:
    void SaveAttributes(QDataStream&) const;

private:
    static damage_kinds EnergyType(int substance);

    static const uint16_t MAX_CHARGE = 0xffff;

    int16_t charge;
};

#endif // ACCUMULATOR_H
