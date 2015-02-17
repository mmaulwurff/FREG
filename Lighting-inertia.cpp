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
    * \brief This file provides simple (also mad) lighting for freg.
    *
    * It has light inertia, meaning that block, enlightened by outer
    * source, will remain enlightened when light source is removed.
    * Light is divided to sunlight and other light. Sunlight is
    * changing over the day.
    *
    * This lighting can be used to develop more intelligent one.
    * Intended to be fast (and is!).
    *
    * LightMap is uchar:
    * & 0xF0 bits are for non-sun light,
    * & 0x0F bits for sun. */

#include "World.h"
#include "Shred.h"
#include "blocks/Block.h"
#include "blocks/Active.h"
#include "AroundCoordinates.h"

void World::Shine(const Xyz& xyz, int level) {
    AddLight(XYZ(xyz), level);
    const int transparent = GetBlock(XYZ(xyz))->Transparent();
    if ( INVISIBLE != transparent && not initial_lighting ) {
        emit Updated(XYZ(xyz));
    }
    if ( BLOCK_OPAQUE != transparent && level > 1 ) {
        --level;
        for (const Xyz& next_xyz : AroundCoordinates(xyz)) {
            Shine(next_xyz, level);
        }
    }
}

void World::SunShineVertical(const int x, const int y) {
    /* 2 1 3
     *   *   First, light goes down, then divides to 4 branches
     * ^ | ^ to N-S-E-W, and goes up.
     * | | |
     * |<v>|
     * # # #   */
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    shred->AddLightOne(x_in, y_in, HEIGHT-1);
    for (int z = HEIGHT-2; ; --z) {
        shred->AddLightOne(x_in, y_in, z);
        switch ( shred->GetBlock(x_in, y_in, z)->Transparent() ) {
        case BLOCK_OPAQUE: return;
        case BLOCK_TRANSPARENT:
            if ( not initial_lighting ) {
                emit Updated(x, y, z);
            }
            break;
        }
    }
}

void World::ReEnlighten(const int x, const int y, const int z) {
    if ( not GetEvernight() ) {
        SunShineVertical(x, y);
    }
    const int radius = GetBlock(x, y, z)->LightRadius();
    if ( radius != 0 ) {
        Shine({x, y, z}, radius);
    }
}

void World::ReEnlightenAll() {
    initial_lighting = true;
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        shreds[i]->ShineAll();
    }
    initial_lighting = false;
}

void World::ReEnlightenMove(const dirs dir) {
    initial_lighting = true;
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
    initial_lighting = false;
}

void World::AddLight(const int x, const int y, const int z, const int level) {
    GetShred(x, y)->
        AddLight(Shred::CoordInShred(x), Shred::CoordInShred(y), z, level);
}

int World::Enlightened(const int x, const int y, const int z) const {
    return GetShred(x, y)->
        Lightmap(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

int World::Enlightened(const int i, const int j, const int k, const dirs dir)
const {
    int x, y, z;
    Focus(i, j, k, &x, &y, &z, dir);
    return qMax(Enlightened(i, j, k), Enlightened(x, y, z));
}

// Shred methods

int Shred::Lightmap(const int x, const int y, const int z) const {
    return lightMap[x][y][z];
}

void Shred::AddLight(const int x, const int y, const int z, const int level) {
    lightMap[x][y][z] += level;
}

void Shred::AddLightOne(const int x, const int y, const int z) {
    ++lightMap[x][y][z];
}

void Shred::SetAllLightMapNull() { memset(lightMap, 0, sizeof(lightMap)); }

/// Makes all shining blocks of shred shine.
void Shred::ShineAll() {
    World * const world = World::GetWorld();
    for (const Active * const shining : shiningList) {
        world->Shine(shining->GetXyz(), shining->LightRadius());
    }
    if ( not world->GetEvernight() ) {
        const int start_x = shredX * SHRED_WIDTH;
        const int start_y = shredY * SHRED_WIDTH;
        const int end_x = start_x + SHRED_WIDTH;
        const int end_y = start_y + SHRED_WIDTH;
        for (int i=start_x; i<end_x; ++i)
        for (int j=start_y; j<end_y; ++j) {
            world->SunShineVertical(i, j);
        }
    }
}
