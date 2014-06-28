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

#include "blocks/RainMachine.h"

RainMachine::RainMachine(const int sub, const int id) :
    Active(sub, id),
    Inventory(1),
    isOn(true)
{}

RainMachine::RainMachine(QDataStream & stream, const int sub, const int id) :
    Active(stream, sub, id),
    Inventory(stream, 1)
{
    stream >> isOn;
}

void RainMachine::SaveAttributes(QDataStream & stream) const {
    Active::SaveAttributes(stream);
    Inventory::SaveAttributes(stream);
    stream << isOn;
}

void RainMachine::DoRareAction() {
    const int number = Number(0);
    if ( isOn
            && number > 0
            && (qrand() % (20-number*2)) == 0 )
    {
        GetShred()->Rain(LIQUID, ShowBlock(0)->Sub());
    }
}

QString RainMachine::FullName() const {
    return tr("Rain Machine") + ( isOn ?
        tr(": on") : tr(": off") );
}

int  RainMachine::Sub() const { return Block::Sub(); }
int  RainMachine::Kind() const { return RAIN_MACHINE; }
int  RainMachine::ShouldAct() const { return FREQUENT_RARE; }
void RainMachine::Push(dirs, Block *) { isOn = not isOn; }
void RainMachine::ReceiveSignal(QString str) { Active::ReceiveSignal(str); }
usage_types RainMachine::Use(Block *) { return USAGE_TYPE_OPEN; }
Inventory * RainMachine::HasInventory() { return this; }
