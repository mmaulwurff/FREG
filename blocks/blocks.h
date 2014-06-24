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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "Inventory.h"
#include "blocks/Animal.h"

class Plate : public Block {
public:
    using Block::Block;

    int Kind() const override;
    int Weight() const override;
    QString FullName() const override;
    push_reaction PushResult(dirs) const override;
};

class Ladder : public Block {
public:
    using Block::Block;

    bool Catchable() const override;
    int  Kind() const override;
    int  Weight() const override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    push_reaction PushResult(dirs) const override;
};

class Liquid : public Falling {
    Q_OBJECT
public:
    using Falling::Falling;

    int ShouldAct() const override;
    int LightRadius() const override;
    int Kind() const override;
    int DamageKind() const override;
    QString FullName() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    push_reaction PushResult(dirs) const override;

protected:
    void DoRareAction() override;
};

class Grass : public Active {
    Q_OBJECT
public:
    using Active::Active;

    int  ShouldAct()  const override;
    int  LightRadius() const override;
    int  Kind()     const override;
    void Push(dirs, Block * who) override;
    Block * DropAfterDamage(bool * delete_block) override;
    QString FullName() const override;
    push_reaction PushResult(dirs) const override;

protected:
    void DoRareAction() override;

private:
    static bool IsBase(int ownsub, int ground);
};

class Bush : public Active, public Inventory {
    Q_OBJECT

    static const int BUSH_SIZE = 3;
public:
    Bush(int sub, int id);
    Bush(QDataStream & str, int sub, int id);

    int  Sub() const override;
    int  ShouldAct() const override;
    void Push(dirs, Block * who) override;
    void ReceiveSignal(QString) override;
    int  Kind() const override;
    int Weight() const override;

    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
    Inventory * HasInventory() override;
    Block * DropAfterDamage(bool * delete_block) override;

protected:
    void DoRareAction() override;
    void SaveAttributes(QDataStream & out) const override;
};

class Rabbit : public Animal {
    Q_OBJECT
public:
    using Animal::Animal;

    int Kind() const override;
    Block * DropAfterDamage(bool * delete_block) override;
    int NutritionalValue(int sub) const override;
    QString FullName() const override;

protected:
    void DoFrequentAction() override;
    void DoRareAction() override;
    int  Attractive(int sub) const override;
};

class Door : public Active {
    Q_OBJECT
public:
    Door(int sub, int id);
    Door(QDataStream & str, int sub, int id);

    int ShouldAct() const override;
    void Push(dirs, Block *) override;
    int  Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who) override;
    push_reaction PushResult(dirs) const override;

protected:
    void DoFrequentAction() override;
    void SaveAttributes(QDataStream & out) const override;

private:
    bool shifted;
    bool locked;
    bool movable;
};

class Clock : public Active {
    Q_OBJECT
public:
    Clock(int sub, int id);
    Clock (QDataStream & str, int sub, int id);

    int ShouldAct() const override;
    int Weight() const override;
    bool Inscribe(QString) override;
    void Push(dirs dir, Block * who) override;
    void Damage(int dmg, int dmg_kind) override;
    int  Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
    INNER_ACTIONS ActInner() override;

protected:
    void DoRareAction() override;

private:
    int alarmTime;
    int timerTime;
};

class Creator : public Active, public Inventory {
    Q_OBJECT
public:
    Creator(int sub, int id);
    Creator(QDataStream & str, int sub, int id);

    int  Kind() const override;
    int  ShouldAct() const override;
    int  Sub() const override;
    void ReceiveSignal(QString) override;
    int  DamageKind() const override;
    int  DamageLevel() const override;
    QString FullName() const override;
    Inventory * HasInventory() override;

protected:
    void SaveAttributes(QDataStream & out) const override;
};

class Text : public Block {
public:
    using Block::Block;

    int  Kind() const override;
    bool Inscribe(QString) override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
};

class Map : public Text {
public:
    Map(int sub, int id);
    Map(QDataStream & str, int sub, int id);

    int Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    // coordinates map titled in. also ~center.
    qint64 longiStart, latiStart;
    quint16 savedShift;
    qint8 savedChar;
};

class Bell : public Active {
    Q_OBJECT
public:
    using Active::Active;

    void ReceiveSignal(QString) override;
    void Damage(int dmg, int dmg_kind) override;
    int  Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
};

class Predator : public Animal {
    Q_OBJECT
public:
    using Animal::Animal;

    int DamageLevel() const override;
    int Kind() const override;
    QString FullName() const override;
    int NutritionalValue(int sub) const override;

protected:
    void DoFrequentAction() override;
    void DoRareAction() override;
    int  Attractive(int sub) const override;
};

#endif // BLOCKS_H
