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

#ifndef RAIN_MACHINE_H
#define RAIN_MACHINE_H

#include "blocks/Inventory.h"
#include "blocks/Active.h"

class RainMachine : public Active, public Inventory {
public:

    BLOCK_CONSTRUCTORS(RainMachine)

    void DoRareAction() override;
    int  ShouldAct() const override;
    void ReceiveSignal(const QString&) override;
    void Damage(int dmg, int dmg_kind) override;
    void SaveAttributes(QDataStream&) const override;
    QString FullName() const override;
    usage_types Use(Active* user) override;
    inner_actions ActInner() override;

private:
    bool isOn;
};

#endif // RAIN_MACHINE_H
