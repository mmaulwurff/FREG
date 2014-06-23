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

#ifndef ANIMAL_H
#define ANIMAL_H

#include "blocks/Active.h"

class Animal : public Falling {
    Q_OBJECT
public:
    Animal(int sub, int id);
    Animal(QDataStream & str, int sub, int id);

    int  DamageKind() const override;
    int  ShouldAct() const override;
    void DoRareAction() override;
    bool Eat(int sub);
    int  Breath() const;
    int  Satiation() const;
    QString FullName() const override = 0;
    Animal * IsAnimal() override;
    Block  * DropAfterDamage(bool * delete_block) override;
    INNER_ACTIONS ActInner() override;

    virtual int NutritionalValue(int sub) const = 0;

protected:
    void SaveAttributes(QDataStream & out) const override;
    void EatGrass();

    bool moved_in_this_turn;

private:
    quint8  breath;
    quint16 satiation;
};

#endif // ANIMAL_H
