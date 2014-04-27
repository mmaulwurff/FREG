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

#ifndef ILLUMINATOR_H
#define ILLUMINATOR_H

#include "blocks/Active.h"

class Illuminator : public Active {
public:
    Illuminator(int sub, quint16 id);
    Illuminator(QDataStream & str, int sub, quint16 id);

    void    Damage(ushort dmg, int dmg_kind) override;
    uchar   LightRadius() const override;
    quint8  Kind() const override;
    Block * DropAfterDamage() const override;
    QString FullName() const override;
    usage_types Use(Block *) override;
private:
};

#endif
