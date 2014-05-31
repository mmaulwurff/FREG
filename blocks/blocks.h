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
    Plate(int sub, quint16 id);
    Plate(QDataStream & str, int sub, quint16 id);

    int     PushResult(int dir) const override;
    quint8  Kind() const override;
    int     Weight() const override;
    QString FullName() const override;
};

class Ladder : public Block {
public:
    Ladder(int sub, quint16 id);
    Ladder(QDataStream & str, int sub, quint16 id);

    int     PushResult(int dir) const override;
    bool    Catchable() const override;
    quint8  Kind() const override;
    int     Weight() const override;
    QString FullName() const override;
    Block * DropAfterDamage() override;
};

class Liquid : public Active {
    Q_OBJECT
public:
    Liquid(int sub, quint16 id);
    Liquid(QDataStream & str, int sub, quint16 id);

    int     ShouldAct() const override;
    int     Temperature() const override;
    int     PushResult(int dir) const override;
    int     LightRadius() const override;
    quint8  Kind() const override;
    QString FullName() const override;
    Block * DropAfterDamage() override;

protected:
    void DoRareAction() override;
};

class Grass : public Active {
    Q_OBJECT
public:
    Grass(int sub, quint16 id);
    Grass(QDataStream & str, int sub, quint16 id);

    int   ShouldAct()  const override;
    int   LightRadius() const override;
    bool  ShouldFall() const override;
    void  Push(int dir, Block * who) override;
    Block * DropAfterDamage() override;
    quint8  Kind()     const override;
    QString FullName() const override;

protected:
    void DoRareAction() override;

private:
    static bool IsBase(int ownsub, int ground);
};

class Bush : public Active, public Inventory {
    Q_OBJECT

    static const int BUSH_SIZE = 3;
public:
    Bush(int sub, quint16 id);
    Bush(QDataStream & str, int sub, quint16 id);

    int  Sub() const override;
    int  PushResult(int dir) const override;
    int  ShouldAct() const override;
    bool ShouldFall() const override;
    void Push(int dir, Block * who) override;
    void ReceiveSignal(QString) override;
    quint8 Kind() const override;
    int Weight() const override;

    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
    Inventory * HasInventory() override;
    Block * DropAfterDamage() override;

protected:
    void DoRareAction() override;
    void SaveAttributes(QDataStream & out) const override;
};

class Rabbit : public Animal {
    Q_OBJECT
public:
    Rabbit(int sub, quint16 id);
    Rabbit(QDataStream & str, int sub, quint16 id);

    Block * DropAfterDamage() override;
    quint8 Kind() const override;
    quint16 NutritionalValue(quint8 sub) const override;
    QString FullName() const override;

protected:
    void DoFrequentAction() override;
    void DoRareAction() override;
    int  Attractive(int sub) const override;
};

class Door : public Active {
    Q_OBJECT
public:
    Door(int sub, quint16 id);
    Door(QDataStream & str, int sub, quint16 id);

    int ShouldAct() const override;
    void Push(int dir, Block *) override;
    int  PushResult(int dir /* not used */) const override;
    bool ShouldFall() const override;
    quint8 Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;

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
    Clock(int sub, quint16 id);
    Clock (QDataStream & str, int sub, quint16 id);

    int ShouldAct() const override;
    int PushResult(int) const override;
    int Weight() const override;
    bool ShouldFall() const override;
    bool Inscribe(QString) override;
    void Push(int dir, Block * who) override;
    void Damage(int dmg, int dmg_kind) override;
    quint8 Kind() const override;
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
    Creator(int sub, quint16 id);
    Creator(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const override;
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
    Text(int sub, quint16 id);
    Text(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
    bool Inscribe(QString) override;
};

class Map : public Text {
public:
    Map(int sub, quint16 id);
    Map(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const override;
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
    Bell(int sub, quint16 id);
    Bell(QDataStream & str, int sub, quint16 id);

    void    ReceiveSignal(QString) override;
    void    Damage(int dmg, int dmg_kind) override;
    quint8  Kind() const override;
    QString FullName() const override;
    usage_types Use(Block * who = 0) override;
};

class Predator : public Animal {
    Q_OBJECT
public:
    Predator(int sub, quint16 id);
    Predator(QDataStream & str, int sub, quint16 id);

    int DamageLevel() const override;
    quint8 Kind() const override;
    QString FullName() const override;
    quint16 NutritionalValue(quint8 sub) const override;

protected:
    void DoFrequentAction() override;
    void DoRareAction() override;
    int  Attractive(int sub) const override;
};

#endif // BLOCKS_H
