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

#include "blocks/Pipe.h"
#include "World.h"
#include "TrManager.h"

void Pipe::ReceiveSignal(const QString& str) { signal = str; }

void Pipe::SaveAttributes(QDataStream& out_stream) const {
    Active::SaveAttributes(out_stream);
    out_stream << signal;
}

QString Pipe::Description() const {
    return tr("Direction: ")
        + TrManager::DirName(GetDir()).toLower()
        + Str(".");
}

usage_types Pipe::Use(Active* const user) {
    SetDir((GetDir() + 1) % DIRS_COUNT);
    user->ReceiveSignal(Description());
    return USAGE_TYPE_INNER;
}

int Pipe::ShouldAct() const { return FREQUENT_SECOND; }

void Pipe::ActFrequent() {
    if (not signal.isNull()) {
        World* const world = World::GetWorld();
        int x, y, z;
        world->Focus(X(), Y(), Z(), &x, &y, &z, GetDir());
        world->GetBlock(x, y, z)->ReceiveSignal(signal);
        signal = QString();
    }
}

Pipe::Pipe(const kinds kind, const subs sub) :
        Active(kind, sub)
{}

Pipe::Pipe(QDataStream& load_stream, const kinds kind, const subs sub) :
        Active(load_stream, kind, sub)
{
    load_stream >> signal;
}
