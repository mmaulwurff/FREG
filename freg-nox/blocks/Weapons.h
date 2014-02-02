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

#ifndef WEAPONS_H
#define WEAPONS_H

#include "blocks/Block.h"

class Weapon : public Block {
public:
    Weapon(int sub, quint16 id);
    Weapon(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    int Wearable() const;
    int DamageKind() const;
    void Push(int dir, Block * who);
    ushort DamageLevel() const;
    ushort Weight() const;
    QString FullName() const;
}

class Pick : public Weapon {
public:
    Pick(int sub, quint16 id);
    Pick(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    int DamageKind() const;
    ushort DamageLevel() const;
    QString FullName() const;
};

#endif // WEAPONS_H
