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

#include "blocks/Inventory.h"
#include "blocks/Active.h"
#include "Shred.h"

class RainMachine : public Active, public Inventory {
    Q_OBJECT
public:
    RainMachine(int sub, int id);
    RainMachine(QDataStream & stream, int sub, int id);

    int  Sub() const override;
    int  Kind() const override;
    void DoRareAction() override;
    int  ShouldAct() const override;
    void Push(dirs, Block *) override;
    QString FullName() const override;
    Inventory * HasInventory() override;
    void ReceiveSignal(QString) override;
    usage_types Use(Block * who) override;
    void SaveAttributes(QDataStream &) const override;

private:
    bool isOn;
};
