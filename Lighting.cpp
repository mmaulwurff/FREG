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

/**\file Lighting-inertia.cpp
    * \brief This file provides simple lighting for freg.
    *
    * There is only ulluminator light (no sun). */

#include "World.h"
#include "Shred.h"
#include "blocks/Block.h"
#include "blocks/Active.h"
#include "AroundCoordinates.h"

void World::Shine(const Xyz& xyz, int level) {
    AddLight(xyz, level);
    level -= (0 < level) - (level < 0); // level sign
    if ( level == 0 ) return;
    for (const Xyz& next_xyz : AroundCoordinates(xyz)) {
        if ( GetBlock(XYZ(next_xyz))->Transparent() != BLOCK_OPAQUE ) {
            Shine(next_xyz, level);
        } else {
            AddLight(next_xyz, level);
        }
    }
}

void World::UnShine(const_int(x, y, z)) {
    // cycle over shreds around xyz, unshine all lights in radius
    const int xShredCenter = Shred::CoordOfShred(x);
    const int yShredCenter = Shred::CoordOfShred(y);
    const int xShredBegin = qMax(0, xShredCenter-1);
    const int yShredBegin = qMax(0, yShredCenter-1);
    const int xShredEnd = qMin(NumShreds()-1, xShredCenter+1);
    const int yShredEnd = qMin(NumShreds()-1, yShredCenter+1);

    for (int xShred=xShredBegin; xShred<=xShredEnd; ++xShred)
    for (int yShred=yShredBegin; yShred<=yShredEnd; ++yShred) {
        Shred * const shred = GetShredByPos(xShred, yShred);
        if ( shred == nullptr ) continue;
        for (auto shining : shred->GetShiningList()) {
            const int x_diff = shining->X() - x;
            const int y_diff = shining->Y() - y;
            const int z_diff = shining->Z() - z;
            if ( not tempShiningList.contains(shining)
                    && x_diff <= MAX_LIGHT_RADIUS - 1
                    && y_diff <= MAX_LIGHT_RADIUS - 1
                    && z_diff <= MAX_LIGHT_RADIUS - 1 )
            {
                const int radius = shining->LightRadius();
                Shine(shining->GetXyz(), -radius);
                tempShiningList.insert(shining, radius);
            }
        }
    }
}

void World::ReEnlighten() {
    for (auto shining  = tempShiningList.cbegin();
              shining != tempShiningList.cend(); ++shining )
    {
        const Xyz xyz = shining.key()->GetXyz();
        Shine(xyz, shining.value());
        emit UpdatedAround(XYZ(xyz));
    }
    tempShiningList.clear();
}

void World::ReEnlightenAll() {
    blockSignals(true);
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        shreds[i]->ShineAll();
    }
    blockSignals(false);
}

void World::ReEnlightenMove(const dirs dir) {
    blockSignals(true);
    switch ( dir ) {
    case NORTH:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[i            ]->ShineAll();
            shreds[i+NumShreds()]->ShineAll();
        }
        break;
    case EAST:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[NumShreds()*i+NumShreds()-1]->ShineAll();
            shreds[NumShreds()*i+NumShreds()-2]->ShineAll();
        }
        break;
    case SOUTH:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[i+NumShreds()*(NumShreds()-1)]->ShineAll();
            shreds[i+NumShreds()*(NumShreds()-2)]->ShineAll();
        }
        break;
    case WEST:
        for (int i=0; i<NumShreds(); ++i) {
            shreds[NumShreds()*i  ]->ShineAll();
            shreds[NumShreds()*i+1]->ShineAll();
        }
        break;
    default: Q_UNREACHABLE(); break;
    }
    blockSignals(false);
}

void World::AddLight(const Xyz& xyz, const int level) {
    GetShred(xyz.X(), xyz.Y())->AddLight(
        Shred::CoordInShred(xyz.X()),
        Shred::CoordInShred(xyz.Y()), xyz.Z(), level);
}

int World::Enlightened(const_int(x, y, z)) const {
    return GetShred(x, y)->
        Lightmap(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

int World::Enlightened(const_int(i, j, k), const dirs dir) const {
    int x, y, z;
    Focus(i, j, k, &x, &y, &z, dir);
    return qMax(Enlightened(i, j, k), Enlightened(x, y, z));
}

// Shred methods

int  Shred::Lightmap(const_int(x, y, z)) const { return lightMap[x][y][z]; }
void Shred::AddLight(const_int(x, y, z), const int level) {
    lightMap[x][y][z] += level;
}

void Shred::ShineAll() {
    World * const world = World::GetWorld();
    for (auto shining : GetShiningList()) {
        world->Shine(shining->GetXyz(), shining->LightRadius());
        ++shining;
    }
}
