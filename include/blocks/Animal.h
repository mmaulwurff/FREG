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

#ifndef ANIMAL_H
#define ANIMAL_H

#include "blocks/Active.h"

class Animal : public Falling {
public:

    BLOCK_CONSTRUCTORS(Animal)
    ~Animal();

    int  DamageKind() const override;
    int  ShouldAct() const override;
    void ActFrequent() override;
    void DoRareAction() override;
    void Damage(int dmg, int dmg_kind) override;
    Animal* IsAnimal() override;
    Block*  DropAfterDamage(bool* delete_block) override;
    QString FullName() const override;
    inner_actions ActInner() override;

    int  Breath()    const { return breath; }
    int  Satiation() const { return satiation; }

    bool Eat(subs);
    class DeferredAction* GetDeferredAction();

    virtual int NutritionalValue(subs) const = 0;

    static const int MAX_BREATH = 60;

    void CauseTeleportation() const;

protected:

    void Updated() const;

    void SaveAttributes(QDataStream& out) const override;
    void EatAround();

    bool moved_in_this_turn;

private:

    quint8  breath;
    quint16 satiation;
    class DeferredAction* deferredAction;
}; // class Animal

// Rabbit
class Rabbit : public Animal {
public:

    using Animal::Animal;

    void ActFrequent() override;

protected:

    void DoRareAction() override;
    int  Attractive(int sub) const override;

private:

    int NutritionalValue(subs) const override;
}; // class Rabbit

// Predator
class Predator : public Animal {
public:

    using Animal::Animal;

    int DamageLevel() const override;
    void ActFrequent() override;

protected:

    int  Attractive(int sub) const override;

private:

    int NutritionalValue(subs) const override;
}; // class Predator

#endif // ANIMAL_H
