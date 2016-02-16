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

#ifndef BLOCKS_H
#define BLOCKS_H

#include "blocks/Inventory.h"
#include "blocks/Active.h"

class Plate : public Block {
public:
    using Block::Block;

    int Weight() const override;
    QString FullName() const override;
    push_reaction PushResult(dirs) const override;
};

class Ladder : public Block {
public:
    using Block::Block;

    bool Catchable() const override;
    int  Weight() const override;
    QString FullName() const override;
    Block* DropAfterDamage(bool* delete_block) override;
    push_reaction PushResult(dirs) const override;
};

class Liquid : public Falling {
    Q_OBJECT
public:
    using Falling::Falling;

    int ShouldAct() const override;
    int DamageKind() const override;
    int DamageLevel() const override;
    int LightRadius() const override;
    bool Inscribe(const QString&) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    Block* DropAfterDamage(bool* delete_block) override;
    wearable Wearable() const override;
    push_reaction PushResult(dirs) const override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;
};

class Grass : public Active {
    Q_OBJECT
public:
    using Active::Active;

    int  ShouldAct() const override;
    int  DamageKind() const override;
    int  LightRadius() const override;
    QString FullName() const override;
    QString Description() const override;
    Block* DropAfterDamage(bool* delete_block) override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;

private:
    static bool IsBase(subs own_sub, subs ground);
};

class Bush : public Active, public Inventory {
    Q_OBJECT

    static const int BUSH_SIZE = 3;
public:
    BLOCK_CONSTRUCTORS(Bush)

    int  Weight() const override;
    int  ShouldAct() const override;
    void ReceiveSignal(const QString&) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    QString Description() const override;
    Block* DropAfterDamage(bool* delete_block) override;
    usage_types Use(Active* who) override;
    inner_actions ActInner() override;

protected:
    void DoRareAction() override;
    void SaveAttributes(QDataStream& out) const override;
};

class Door : public Active {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Door)

    int  ShouldAct() const override;
    void ActFrequent() override;
    void Damage(int dmg, int dmg_kind) override;
    void ReceiveSignal(const QString&) override;
    QString FullName() const override;
    usage_types Use(Active* who) override;
    push_reaction PushResult(dirs) const override;

protected:
    void SaveAttributes(QDataStream& out) const override;

private:
    bool shifted;
    bool locked;
    push_reaction movable;
};

class Clock : public Active {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Clock)

    int  Weight() const override;
    int  ShouldAct() const override;
    bool Inscribe(const QString&) override;
    void Damage(int dmg, int dmg_kind) override;
    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Active* who) override;
    inner_actions ActInner() override;

protected:
    void SaveAttributes(QDataStream&) const override;

private:
    int alarmTime;
    int timerTime;
};

/// The Signaller class notifies neighbours about impact.
class Signaller : public Active {
    Q_OBJECT
public:
    using Active::Active;

    QString  FullName()    const override;
    QString  Description() const override;
    wearable Wearable()    const override;
    usage_types Use(Active* who) override;
    void Damage(int dmg, int dmg_kind) override;

private:
    bool AbsorbDamage(damage_kinds) const;
    void Signal(int level) const;
};

class Telegraph : public Active {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Telegraph)

    int ShouldAct() const override;
    bool Inscribe(const QString&) override;
    void ReceiveSignal(const QString&) override;
    wearable Wearable() const override;
    inner_actions ActInner() override;
    usage_types Use(Active* who) override;

protected:
    void SaveAttributes(QDataStream&) const override;

private:
    static QString sharedMessage;
    bool isReceiver;
};

class MedKit : public Block {
public:
    using Block::Block;
    wearable Wearable() const override;
    usage_types Use(Active* user) override;
};

/** The Informer class provides various information.
 *  Can be used inside inventory and outside.
 *  Iron informer is compass. */
class Informer : public Block {
public:
    using Block::Block;

    QString FullName() const override;
    wearable Wearable() const override;
    usage_types Use(Active* user) override;
};

#endif // BLOCKS_H
