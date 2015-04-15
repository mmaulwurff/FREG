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

#include "blocks/Accumulator.h"
#include "TrManager.h"
#include <QDataStream>

Accumulator::Accumulator(const kinds kind, const subs sub) :
        Block(kind, sub),
        charge(0)
{}

Accumulator::Accumulator(QDataStream& stream, const kinds kind, const subs sub)
    :
        Block(stream, kind, sub),
        charge()
{
    stream >> charge;
}

void Accumulator::SaveAttributes(QDataStream& str) const { str << charge; }

QString Accumulator::FullName() const {
    QString name;
    if ( GetSubGroup(Sub()) == GROUP_METAL ) {
        name = QObject::tr("Battery (%1)").arg(TrManager::SubName(Sub()));
    } else {
        switch ( Sub() ) {
        case GLASS: name = QObject::tr("Thermos"); break;
        default:    name = Block::FullName(); break;
        }
    }
    return name.append(Str(" (charge: %1%)").arg(100*charge / MAX_CHARGE));
}

void Accumulator::Damage(const int dmg, const int dmg_kind) {
    if ( dmg_kind == EnergyType(Sub()) ) {
        charge = std::min(charge+dmg, int(MAX_CHARGE));
    } else {
        Block::Damage(dmg, dmg_kind);
    }
}

damage_kinds Accumulator::EnergyType(const int substance) {
    switch ( substance ) {
    default:    return DAMAGE_HEAT;
    case IRON:
    case ADAMANTINE:
    case GOLD:  return DAMAGE_ELECTRO;
    }
}
