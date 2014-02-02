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

#include "blocks/Block.h"
#include "Inventory.h"
#include "blocks/Active.h"
#include "blocks/Animal.h"

// weights in measures - mz (mezuro)
    const ushort WEIGHT_NULLSTONE = 1000;
    const ushort WEIGHT_SAND = 100;
    const ushort WEIGHT_WATER = 50;
    const ushort WEIGHT_GLASS = 150;
    const ushort WEIGHT_IRON = 300;
    const ushort WEIGHT_GREENERY = 3;
    const ushort WEIGHT_MINIMAL = 1;
    const ushort WEIGHT_STONE = 200;
    const ushort WEIGHT_AIR = 0;

class QDataStream;

class Plate : public Block {
public:
    QString FullName() const;
    quint8 Kind() const;
    int PushResult(int dir) const;
    ushort Weight() const;

    Plate(int sub, quint16 id);
    Plate(QDataStream & str, int sub, quint16 id);
}; // class Plate

class Ladder : public Block {
public:
    QString FullName() const;
    quint8 Kind() const;
    int PushResult(int dir) const;
    ushort Weight() const;
    bool Catchable() const;
    Block * DropAfterDamage() const;

    Ladder(int sub, quint16 id);
    Ladder(QDataStream & str, int sub, quint16 id);
}; // class Ladder

class Weapon : public Block {
public:
    quint8 Kind() const;
    int Wearable() const;
    int DamageKind() const;
    void Push(int dir, Block * who);
    ushort DamageLevel() const;
    ushort Weight() const;
    QString FullName() const;

    Weapon(int sub, quint16 id);
    Weapon(QDataStream & str, int sub, quint16 id);
}; // class Weapon

class Pick : public Weapon {
public:
    quint8 Kind() const;
    int DamageKind() const;
    ushort DamageLevel() const;
    QString FullName() const;

    Pick(int sub, quint16 id);
    Pick(QDataStream & str, int sub, quint16 id);
}; // class Pick

class Chest : public Block, public Inventory {
public:
    Chest(int sub, quint16 id, ushort size = INV_SIZE);
    Chest(QDataStream & str, int sub, quint16 id, ushort size = INV_SIZE);

    quint8 Kind() const;
    int  Sub() const;
    void Push(int dir, Block * who);
    void ReceiveSignal(const QString);
    QString FullName() const;
    Inventory * HasInventory();
    usage_types Use(Block * who = 0);
    ushort Weight() const;

protected:
    void SaveAttributes(QDataStream & out) const;
}; // class Chest

class Pile : public Active, public Inventory {
    Q_OBJECT
public:
    Pile(int sub, quint16 id);
    Pile(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    int  Sub() const;
    void ReceiveSignal(const QString);
    void DoRareAction();
    int  ShouldAct() const;
    void Push(int, Block * who);
    Block * DropAfterDamage() const;
    QString FullName() const;
    Inventory * HasInventory();
    usage_types Use(Block * who = 0);
    ushort Weight() const;
protected:
    void SaveAttributes(QDataStream & out) const;
}; // class Pile

class Liquid : public Active {
    Q_OBJECT
public:
    void DoRareAction();
    int  ShouldAct() const;
    quint8 Kind() const;
    int PushResult(int dir) const;
    int  Temperature() const;
    uchar LightRadius() const;
    QString FullName() const;
    Block * DropAfterDamage() const;

    Liquid(int sub, quint16 id);
    Liquid(QDataStream & str, int sub, quint16 id);
}; // class Liquid

class Grass : public Active {
    Q_OBJECT
public:
    QString FullName() const;
    void DoRareAction();
    int  ShouldAct() const;
    quint8 Kind() const;
    bool ShouldFall() const;
    void Push(int dir, Block * who);
    Block * DropAfterDamage() const;

    Grass(int sub, quint16 id);
    Grass(QDataStream & str, int sub, quint16 id);
}; // class Grass

class Bush : public Active, public Inventory {
    Q_OBJECT

    static const ushort BUSH_SIZE = 3;
public:
    Bush(int sub, quint16 id);
    Bush(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    int  Sub() const;
    int PushResult(int dir) const;
    bool ShouldFall() const;
    void DoRareAction();
    int  ShouldAct() const;
    void Push(int dir, Block * who);
    void ReceiveSignal(const QString);
    ushort Weight() const;

    QString FullName() const;
    usage_types Use(Block * who = 0);
    Inventory * HasInventory();
    Block * DropAfterDamage() const;
protected:
    void SaveAttributes(QDataStream & out) const;
}; // class Bush

class Rabbit : public Animal {
    Q_OBJECT
public:
    Rabbit(int sub, quint16 id);
    Rabbit(QDataStream & str, int sub, quint16 id);

    void DoFrequentAction();
    void DoRareAction();
    Block * DropAfterDamage() const;
    quint8 Kind() const;
    quint16 NutritionalValue(quint8 sub) const;
    QString FullName() const;

protected:
    short Attractive(int sub) const;
}; // class Rabbit

class Workbench : public Chest {
public:
    Workbench(int sub, quint16 id);
    Workbench(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    bool Drop(ushort src, ushort dest, ushort num, Inventory * inv);
    bool Get(Block * block, ushort start = 0);
    bool GetAll(Inventory * from);
    void ReceiveSignal(const QString);
    ushort Start() const;
    QString FullName() const;
private:
    void Craft();

    static const ushort WORKBENCH_SIZE = 10;
}; // class Workbench

class Door : public Active {
    Q_OBJECT
public:
    Door(int sub, quint16 id);
    Door(QDataStream & str, int sub, quint16 id);

    int ShouldAct() const;
    void Push(int dir, Block * const);
    int  PushResult(int dir /* not used */) const;
    void DoFrequentAction();
    bool ShouldFall() const;
    quint8 Kind() const;
    QString FullName() const;
    usage_types Use(Block * who = 0);
protected:
    void SaveAttributes(QDataStream & out) const;
private:
    bool shifted;
    bool locked;
    bool movable;
}; // class Door

class Clock : public Active {
public:
    Clock(int sub, quint16 id);
    Clock (QDataStream & str, int sub, quint16 id);

    int ShouldAct() const;
    int PushResult(const int) const;
    void DoRareAction();
    bool ShouldFall() const;
    void Push(int dir, Block * who);
    bool Inscribe(const QString);
    quint8 Kind() const;
    ushort Weight() const;
    QString FullName() const;
    usage_types Use(Block * who = 0);

private:
    short alarmTime;
    short timerTime;
}; // class Clock

class Creator : public Active, public Inventory {
    Q_OBJECT
public:
    Creator(int sub, quint16 id);
    Creator(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    int  Sub() const;
    void ReceiveSignal(const QString);
    int  DamageKind() const;
    ushort DamageLevel() const;
    QString FullName() const;
    Inventory * HasInventory();
protected:
    void SaveAttributes(QDataStream & out) const;
}; // class Creator

class Text : public Block {
public:
    Text(int sub, quint16 id);
    Text(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    QString FullName() const;
    usage_types Use(Block * who = 0);
    bool Inscribe(const QString);
}; // class Text

class Map : public Text {
public:
    Map(int sub, quint16 id);
    Map(QDataStream & str, int sub, quint16 id);

    quint8 Kind() const;
    QString FullName() const;
    usage_types Use(Block * who = 0);
protected:
    void SaveAttributes(QDataStream & out) const;
private:
    // coordinates map titled in. also ~center.
    qint64 longiStart, latiStart;
    quint16 savedShift;
    qint8 savedChar;
}; // class Map

class Bell : public Active {
public:
    quint8 Kind() const;
    QString FullName() const;
    usage_types Use(Block * who = 0);
    void ReceiveSignal(const QString);

    Bell(int sub, quint16 id);
    Bell(QDataStream & str, int sub, quint16 id);
}; // class Bell

class Predator : public Animal {
    Q_OBJECT
public:
    Predator(int sub, quint16 id);
    Predator(QDataStream & str, int sub, quint16 id);

    ushort DamageLevel() const;
    quint8 Kind() const;
    QString FullName() const;
    quint16 NutritionalValue(quint8 sub) const;

protected:
    void DoFrequentAction();
    void DoRareAction();
    short Attractive(int sub) const;
}; // class Predator

#endif // BLOCKS_H