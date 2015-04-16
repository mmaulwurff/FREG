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

#ifndef ACTIVE_H
#define ACTIVE_H

#include "blocks/Block.h"
#include "Xyz.h"
#include <QObject>

/// Frequency can be "never", "rare", "rare | first", "rare | second".
enum active_frequency {
    FREQUENT_NEVER  = 0,
    FREQUENT_RARE   = 1,
    FREQUENT_FIRST  = 2,
    FREQUENT_SECOND = 4,
};

/// See Shred::PhysEventsRare() for details.
enum inner_actions {
    INNER_ACTION_ONLY,
    INNER_ACTION_NONE,
    INNER_ACTION_EXPLODE, // like in Fallout
    INNER_ACTION_MESSAGE
};

class Active : public QObject, public Block, protected Xyz {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Active)

    int X() const;
    int Y() const;
    using Xyz::Z;
    using Xyz::SetXyz;
    const XyzInt GetXyz() const;

    void Move(dirs dir) override;
    void ReceiveSignal(const QString&) override;
    Active* ActiveBlock() override final;

    virtual void ActFrequent();
    virtual inner_actions ActInner();
    virtual int ShouldAct() const;

    void ActRare();
    void SetShred(class Shred* const new_shred) { shred = new_shred; }
    class Shred* GetShred() const { return shred; }

    void Farewell();
    void Unregister();

signals:
    void Moved(int);
    void ReceivedText(const QString);

protected:
    void UpdateLightRadius(int old_radius);
    void SendSignalAround(QString) const;
    void DamageAround() const;
    /// Damages block, if it is broken returns true and destroys block.
    bool TryDestroy(int x, int y, int z) const;
    /// Returns true if there is at least 1 block of substance sub around.
    bool IsSubAround(int sub) const;
    bool Gravitate(int range, int down, int up, int calmness);
    bool IsInside() const;

    virtual void DoRareAction();
    virtual int  Attractive(int sub) const;

private:
    void ReRegister(dirs);

    class Shred* shred = nullptr;
}; // Active

class Falling : public Active {
    Q_OBJECT
public:
    BLOCK_CONSTRUCTORS(Falling)

    void Move(dirs dir) override;
    QString FullName() const override;
    Falling* ShouldFall() override final;
    push_reaction PushResult(dirs) const override;

    void FallDamage();
    bool IsFalling() const { return falling; }
    void SetFalling(bool set);

protected:
    void SaveAttributes(QDataStream& out) const override;

private:
    quint8 fallHeight;
    bool falling;
}; // Falling

#endif // ACTIVE_H
