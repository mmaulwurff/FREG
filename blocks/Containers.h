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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "blocks/Active.h"
#include "blocks/Inventory.h"

class Container : public Active, public Inventory {
    /** \class Container Container.h
     *  \brief Container is multi-purpose container for blocks.
     *
     *  Behaviour depends on substance:
     *  - container from IRON (locker) and WATER (fridge) prevents all activity
     *    inside;
     *  - container from other substances are different chests. */
    Q_OBJECT
public:
    Container(kinds, subs, int size = INV_SIZE);
    Container(QDataStream&, kinds, subs, int size = INV_SIZE);

    void ReceiveSignal(const QString&) override;
    int  ShouldAct() const override;
    int  Weight() const override;
    void Damage(int dmg, int dmg_kind) override;
    Block* DropAfterDamage(bool* delete_block) override;
    QString FullName() const override;
    usage_types Use(Active* user) override;
    push_reaction PushResult(dirs) const override;
    inner_actions ActInner() override;

    static const int WORKBENCH_SIZE = 10;

protected:
    void SaveAttributes(QDataStream& out) const override;
}; // Container

class Box : public Falling, public Inventory {
    /** \class Box Box.h
     *  \brief Box represents falling inventory.
     *
     *  Unlike chests, position of box is not static, it can be moved and falls
     *  when it can.
     *  Also, pile is box of substance DIFFERENT, it will disappear if empty.*/
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Box)

    int  ShouldAct() const override;
    void Damage(int dmg, int dmg_kind) override;
    void ReceiveSignal(const QString&) override;
    void DoRareAction() override;
    QString FullName() const override;
    Block* DropAfterDamage(bool* delete_block) override;
    usage_types Use(Active* user) override;
    inner_actions ActInner() override;

protected:
    void SaveAttributes(QDataStream& out) const override;
}; // Box

class Workbench : public Container {
    /** \class Workbench Container.h
     *  \brief Workbench allows craft from multiple sources. There can be up to
     *  2 products. Also can be used as container of smaller size. */
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Workbench)

    bool Drop(int src, int dest, int num, Inventory* inv) override;
    bool Get(Block* block, int start) override;
    bool GetAll(Inventory* from) override;
    int Start() const override;
    QString FullName() const override;
    QString InvFullName(int slot_number) const override;

private:
    void Craft();
}; // Workbench

/// @brief Converter blocks convert fuel into energy.
class Converter : public Container {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Converter)

    int  ShouldAct() const override;
    int  DamageKind() const override;
    int  LightRadius() const override;
    void DoRareAction() override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    QString InvFullName(int slot_number) const override;

protected:
    void SaveAttributes(QDataStream&) const override;

private:
    static const int CONVERTER_LIGHT_RADIUS = 2;

    void InitDamageKinds();
    int  ConvertRatio(int sub) const;

    // saved attributes
    bool isOn;
    quint16 fuelLevel;

    // not saved attributes
    int lightRadius;
    damage_kinds damageKindOn;
    damage_kinds damageKindOff;
}; // Converter

#endif // CONTAINER_H
