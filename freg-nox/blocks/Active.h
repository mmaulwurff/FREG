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

class Active : public QObject, public Block {
    Q_OBJECT
public:
     Active(int sub, quint16 id, quint8 transp = UNDEF);
     Active(QDataStream & str, int sub, quint16 id, quint8 transp = UNDEF);
    ~Active();

    Shred * GetShred() const;
    World * GetWorld() const;
    QString FullName() const override;
    quint8 Kind() const;
    int PushResult(int dir) const;

    Active * ActiveBlock();
    /// Returns true if shred border is overstepped.
    void SetFalling(bool set);
    bool Move(int dir);
    bool IsFalling() const;
    void FallDamage();

    ushort X() const;
    ushort Y() const;
    ushort Z() const;

    void ActFrequent();
    void ActRare();
    virtual INNER_ACTIONS ActInner();
    virtual int ShouldAct() const;
    virtual bool ShouldFall() const;

    void SetDeferredAction(DeferredAction *);
    DeferredAction * GetDeferredAction() const;

    void Damage(ushort dmg, int dmg_kind);
    void ReceiveSignal(QString);

    void ReloadToNorth();
    void ReloadToSouth();
    void ReloadToWest();
    void ReloadToEast();

    void EmitUpdated();

    void SetXYZ(ushort x, ushort y, ushort z);
    void SetToDelete();
    void SetShred(Shred *);
signals:
    void Moved(int);
    void Destroyed();
    void Updated();
    void ReceivedText(const QString);
protected:
    void SendSignalAround(QString) const;
    void SaveAttributes(QDataStream & out) const;
    /// Returns true if there is at least 1 block of substance sub around.
    bool IsSubAround(quint8 sub) const;
    bool Gravitate(ushort range, ushort down, ushort up, ushort calmness);

    virtual void DoFrequentAction();
    virtual void DoRareAction();
    virtual short Attractive(int sub) const;
private:
    void UpdateShred();
    bool IsToDelete() const;

    quint8 fall_height;
    bool falling;
    bool frozen; // don't do actions when frozen
    DeferredAction * deferredAction;
    /// Coordinates in loaded world zone.
    ushort x_self, y_self, z_self;
    Shred * shred;
};

#endif // ACTIVE_H
