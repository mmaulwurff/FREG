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

#include "World.h"
#include "WorldMap.h"
#include "blocks/Animal.h"
#include "blocks/Teleport.h"
#include "header.h"
#include "RandomManager.h"

#include <QDataStream>
#include <QTextStream>

Teleport::Teleport(const kinds kind, const subs sub)
    : Active(kind, sub)
    , targetWorldName(World::WorldName())
    , targetLongitude( RandomManager::rand()
                     % World::GetCWorld()->GetMap()->GetHeight() )
    , targetLatitude(  RandomManager::rand()
                     % World::GetCWorld()->GetMap()->GetWidth(targetLongitude) )
{}

Teleport::Teleport(QDataStream& stream, const kinds kind, const subs sub)
    : Active(stream, kind, sub)
    , targetWorldName()
    , targetLongitude()
    , targetLatitude()
{
    stream >> targetWorldName >> targetLongitude >> targetLatitude;
}

void Teleport::SaveAttributes(QDataStream& stream) const {
    stream << targetWorldName << targetLongitude << targetLatitude;
}

bool Teleport::Inscribe(const QString& note) {
    QString input(note);
    QTextStream stream(&input);
    stream >> targetLongitude >> targetLatitude >> targetWorldName;
    --targetLatitude;
    --targetLongitude;
    if ( targetWorldName.isEmpty() ) {
        targetWorldName = World::WorldName();
    }
    return Block::Inscribe(input);
}

void Teleport::Damage(const int damage, const int damage_kind) {
    if ( damage_kind >= DAMAGE_PUSH_UP && damage_kind != DAMAGE_PUSH_DOWN) {
        World* const world = World::GetWorld();
        int x, y, z;
        world->Focus(X(), Y(), Z(), &x, &y, &z,
            World::Anti(MakeDirFromDamage(damage_kind)));
        world->ReloadAllShreds(targetWorldName,
            targetLatitude, targetLongitude,
            (world->NumShreds() / 2 + 1) * SHRED_WIDTH,
            (world->NumShreds() / 2 + 1) * SHRED_WIDTH, 0);
        Animal* const animal = world->GetBlock(x, y, z)->IsAnimal();
        if ( animal ) animal->CauseTeleportation();
    } else {
        Block::Damage(damage, damage_kind);
    }
}
