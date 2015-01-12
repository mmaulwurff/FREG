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

#include "blocks/Teleport.h"
#include "World.h"
#include "worldmap.h"
#include "Animal.h"
#include <QTextStream>

Teleport::Teleport(const int sub, const int kind) :
        Active(sub, kind),
        targetWorldName(World::GetWorld()->WorldName()),
        targetLatitude( qrand()%(World::GetWorld()->GetMap()->GetSize())),
        targetLongitude(qrand()%(World::GetWorld()->GetMap()->GetSize()))
{}

Teleport::Teleport(QDataStream & stream, const int sub, const int kind) :
        Active(stream, sub, kind),
        targetWorldName(),
        targetLatitude(),
        targetLongitude()
{
    stream >> targetWorldName >> targetLongitude >> targetLatitude;
}

void Teleport::SaveAttributes(QDataStream & stream) const {
    stream << targetWorldName << targetLongitude << targetLatitude;
}

bool Teleport::Inscribe(QString input) {
    QTextStream stream(&input);
    stream >> targetLongitude >> targetLatitude >> targetWorldName;
    --targetLatitude;
    --targetLongitude;
    if ( targetWorldName.isEmpty() ) {
        targetWorldName = World::GetWorld()->WorldName();
    }
    return Block::Inscribe(input);
}

void Teleport::Damage(const int damage, const int damage_kind) {
    if ( damage_kind >= DAMAGE_PUSH_UP && damage_kind != DAMAGE_PUSH_DOWN) {
        World * const world = World::GetWorld();
        int x, y, z;
        world->Focus(X(), Y(), Z(), &x, &y, &z,
            World::Anti(MakeDirFromDamage(damage_kind)));
        world->ReloadAllShreds(targetWorldName,
            targetLatitude, targetLongitude,
            (world->NumShreds() / 2 + 1) * SHRED_WIDTH,
            (world->NumShreds() / 2 + 1) * SHRED_WIDTH, 0);
        Animal * const animal = world->GetBlock(x, y, z)->IsAnimal();
        if ( animal != nullptr ) {
            emit animal->CauseTeleportation();
        }
    } else {
        Block::Damage(damage, damage_kind);
    }
}
