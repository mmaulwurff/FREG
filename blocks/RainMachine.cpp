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

#include "blocks/RainMachine.h"
#include "Shred.h"

RainMachine::RainMachine(const int kind, const int sub) :
    Active(kind, sub),
    Inventory(1),
    isOn(false)
{}

RainMachine::RainMachine(QDataStream & stream, const int kind, const int sub) :
    Active(stream, kind, sub),
    Inventory(stream, 1),
    isOn()
{
    stream >> isOn;
}

void RainMachine::SaveAttributes(QDataStream & stream) const {
    Inventory::SaveAttributes(stream);
    stream << isOn;
}

void RainMachine::DoRareAction() {
    if ( not isOn ) return;
    if ( AIR == GetInvSub(0) ) {
        if ( qrand() & 3 ) {
            GetShred()->Rain(LIQUID, SUB_CLOUD);
        }
    } else if ( qrand() % (20 - Number(0)*2) ) {
        GetShred()->Rain(LIQUID, GetInvSub(0));
    }
}

QString RainMachine::FullName() const {
    return QString("%1: %2").
        arg(tr_manager->KindName(RAIN_MACHINE)).
        arg(tr_manager->OffOn(isOn));
}

int  RainMachine::ShouldAct() const { return FREQUENT_RARE; }
void RainMachine::ReceiveSignal(QString str) { Active::ReceiveSignal(str); }
usage_types RainMachine::Use(Active *) { return USAGE_TYPE_OPEN; }
Inventory * RainMachine::HasInventory() { return this; }

void RainMachine::Damage(const int dmg, const int dmg_kind) {
    if ( dmg_kind >= DAMAGE_PUSH_UP ) {
        GetShred()->SetWeather( (isOn = not isOn) ?
            WEATHER_RAIN : WEATHER_CLEAR );
    } else {
        Block::Damage(dmg, dmg_kind);
    }
}

inner_actions RainMachine::ActInner() { return INNER_ACTION_NONE; }
