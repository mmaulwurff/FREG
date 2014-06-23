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

#ifndef ACTIVE_H
#define ACTIVE_H

#include <QObject>
#include "Block.h"
#include "Xyz.h"

/// Frequency can be "never", "rare", "rare & first", "rare & second".
enum ACTIVE_FREQUENCY {
    FREQUENT_NEVER  = 0,
    FREQUENT_RARE   = 1,
    FREQUENT_FIRST  = 2,
    FREQUENT_SECOND = 4,
};

enum INNER_ACTIONS {
    INNER_ACTION_NONE,
    INNER_ACTION_EXPLODE, // like in Fallout
    INNER_ACTION_MESSAGE
};

class Shred;
class World;
class DeferredAction;

class Active : public QObject, public Block, public Xyz {
    Q_OBJECT
public:
     Active(int sub, int id, int transp = UNDEF);
     Active(QDataStream & str, int sub, int id, int transp = UNDEF);
    ~Active();

    bool Move(int dir) override;
    void Damage(int dmg, int dmg_kind) override;
    void ReceiveSignal(QString) override;
    int  PushResult(int dir) const override;
    int  Kind() const override = 0;
    QString  FullName() const override = 0;
    Active * ActiveBlock() override;

    Shred * GetShred() const;
    World * GetWorld() const;

    void ActFrequent();
    void ActRare();

    virtual INNER_ACTIONS ActInner();
    virtual int  ShouldAct()  const;

    void SetDeferredAction(DeferredAction *);
    DeferredAction * GetDeferredAction() const;

    void ReloadToNorth();
    void ReloadToSouth();
    void ReloadToWest();
    void ReloadToEast();

    void EmitUpdated();

    void SetToDelete();
    void SetShred(Shred *);

signals:
    void Moved(int);
    void Destroyed();
    void Updated();
    void ReceivedText(const QString);

protected:
    void SendSignalAround(QString) const;
    void DamageAround() const;
    /// Damages block and destroys it if it is broken.
    void TryDestroy(int x, int y, int z) const;
    /// Returns true if there is at least 1 block of substance sub around.
    bool IsSubAround(int sub) const;
    bool Gravitate(int range, int down, int up, int calmness);

    virtual void DoFrequentAction();
    virtual void DoRareAction();
    virtual int  Attractive(int sub) const;

private:
    void UpdateShred();
    bool IsToDelete() const;

    /// Don't do actions when frozen.
    bool frozen;
    DeferredAction * deferredAction;
    Shred * shred;
};

class Falling : public Active {
    Q_OBJECT
public:
    Falling(int sub, int id, int transp = UNDEF);
    Falling(QDataStream & str, int sub, int id, int transp = UNDEF);

    int  Kind() const override;
    QString FullName() const override;
    Falling * ShouldFall() override;

    void FallDamage();
    bool IsFalling() const;
    /// Returns true if shred border is overstepped.
    void SetFalling(bool set);
    void IncreaseFallHeight();
    void SetNotFalling();

protected:
    void SaveAttributes(QDataStream & out) const override;

private:
    quint8 fallHeight;
    bool falling;
};

#endif // ACTIVE_H
