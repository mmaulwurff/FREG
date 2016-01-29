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

#include "Shred.h"
#include "blocks/Block.h"
#include "BlockFactory.h"

int Shred::NormalUnderground(const int depth, const subs sub) {
    NormalCube(0,0,1, SHRED_WIDTH,SHRED_WIDTH,HEIGHT/2-depth-5, STONE);
    Block* const block = Normal(sub), *stone = Normal(STONE);
    for (CoordinateIterator i; i.notEnd(); i.step()) {
        PutBlock(((rand() % 2) ? stone : block),
                 i.X(), i.Y(), HEIGHT/2 - depth - 6);
    };
    NormalCube(0,0,HEIGHT/2-depth-5, SHRED_WIDTH,SHRED_WIDTH,6, sub);
    return HEIGHT/2;
}

void Shred::Plain() {
    NormalUnderground();
    LoadRoom(HEIGHT/2);
    RandomDrop(rand() % 4, BUSH,   WOOD);
    RandomDrop(rand() % 4, BLOCK,  ROSE);
    RandomDrop(rand() % 4, RABBIT, A_MEAT);
    PlantGrass();
}

void Shred::Forest(const bool dead) {
    NormalUnderground();
    RandomDrop(rand() % 4, BUSH, WOOD);
    //SetNewBlock(FALLING, STEEL, 0, 0, HEIGHT / 2 + 5);
    //SetNewBlock(FALLING, GOLD, 0, 0, HEIGHT / 2 + 5);
    if ( not dead ) {
        RandomDrop(rand() % 2, BLOCK, ROSE);
        PlantGrass();
    }
    for (int number = AroundShredTypes(longitude, latitude).Count(type);
            number; --number)
    {
        const int x = rand()%(SHRED_WIDTH-2) + 1;
        const int y = rand()%(SHRED_WIDTH-2) + 1;
        const int z = FindTopNonAir(x, y);
        if ( GetBlock(x, y, z)->Sub() == SOIL ) {
            Tree(x-1, y-1, z+1);
        } else if ( GetBlock(x, y, z-1)->Sub() == SOIL ) {
            Tree(x-1, y-1, z);
        }
    }
    RandomDrop(rand() % 4, WEAPON, WOOD);
}

void Shred::Water(const subs sub) {
    subs shore;
    switch ( sub ) {
    case WATER: shore = SAND;  break;
    case AIR:
    case STONE: shore = STONE; break;
    default:
    case ACID:  shore = GLASS; break;
    }
    const AroundShredTypes shred_types(longitude, latitude);
    const int depth = shred_types.Count(GetTypeOfShred()) + 1;
    NormalUnderground(depth, shore);
    const int z_start = HEIGHT/2 - depth + 1;
    NormalCube(0,0,z_start-1, SHRED_WIDTH,SHRED_WIDTH,1, shore); // bottom
    if ( type != shred_types.To(TO_NORTH) ) {
        NormalCube(0,0,z_start, SHRED_WIDTH,1,depth, shore);
    }
    if ( type != shred_types.To(TO_SOUTH) ) {
        NormalCube(0,SHRED_WIDTH-1,z_start, SHRED_WIDTH,1,depth, shore);
    }
    if ( type != shred_types.To(TO_EAST) ) {
        NormalCube(SHRED_WIDTH-1,0,z_start, 1,SHRED_WIDTH,depth, shore);
    }
    if ( type != shred_types.To(TO_WEST) ) {
        NormalCube(0,0,z_start, 1,SHRED_WIDTH,depth, shore);
    }
    Block* const air = Normal(AIR);
    for (CoordinateIterator i; i.notEnd(); i.step()) {
        const int x = i.X(), y = i.Y();
        Block** const pos = blocks[x][y];
        for (Block** block=pos+z_start; block<=pos+HEIGHT/2; ++block) {
            if ( shore != (*block)->Sub() ) {
                if ( AIR == sub ) {
                    *block = air;
                } else {
                    SetNewBlock(LIQUID, sub, x, y, block - pos);
                }
            }
        }
    }
}

void Shred::Hill(const bool dead) {
    NormalUnderground();
    Block* const soil = Normal(SOIL);
    for (CoordinateIterator i; i.notEnd(); i.step()) {
        const int x = i.X(), y = i.Y();
        for (int z=SHRED_WIDTH/2-2; z--; ) {
            if ( z <= -abs(x-SHRED_WIDTH/2) + SHRED_WIDTH/2-2 ) {
                PutBlock(soil, x, y, z+HEIGHT/2); // north-south '^'
            }
            if ( z <= -abs(y-SHRED_WIDTH/2) + SHRED_WIDTH/2-2 ) {
                PutBlock(soil, x, y, z+HEIGHT/2); // east-west '^'
            }
        }
    }
    RandomDrop(rand() % 4, WEAPON, STONE);
    if ( not dead ) {
        int random = rand();
        RandomDrop(random % 4, BLOCK, ROSE);
        RandomDrop((random >>= 4) % 4, RABBIT, A_MEAT);
        RandomDrop((random >>= 4) % 4, BUSH, WOOD);
        PlantGrass();
    }
}

void Shred::Mountain() {
    NormalUnderground();
    /* ###
     * #~#??? east bridge
     * ###
     *  ?
     *  ? south bridge
     *  ?  */
    const AroundShredTypes shred_types(longitude, latitude);
    const int mount_top =
        HEIGHT/2 + HEIGHT/4 * shred_types.Count(GetTypeOfShred()) / 9;
    NormalCube(0, 0, 1, SHRED_WIDTH/2, SHRED_WIDTH/2, mount_top, STONE);
    // bridges
    if ( SHRED_MOUNTAIN == shred_types.To(TO_SOUTH) ) {
        NormalCube(rand()%(SHRED_WIDTH/2-1), SHRED_WIDTH/2, mount_top,
            2, SHRED_WIDTH/2, 1, STONE);
    }
    if ( SHRED_MOUNTAIN == shred_types.To(TO_EAST) ) {
        NormalCube(SHRED_WIDTH/2, rand()%(SHRED_WIDTH/2-1), mount_top,
            SHRED_WIDTH/2, 2, 1, STONE);
    }
    // water pool
    if ( (rand() % 8) == 0 ) {
        for (int i=1; i<SHRED_WIDTH/2-1; ++i)
        for (int j=1; j<SHRED_WIDTH/2-1; ++j)
        for (int k=mount_top-3; k<=mount_top; ++k) {
            SetNewBlock(LIQUID, WATER, i, j, k);
        }
    }
    // cavern
    if ( (rand() % 8) == 0 ) {
        NormalCube(SHRED_WIDTH/4-2, SHRED_WIDTH/4-2, HEIGHT/2+1, 4, 4, 3, AIR);
        const int entries = rand() % 16;
        if ( entries & 0b0001 ) { // north entry
            NormalCube(SHRED_WIDTH/4-1, 0, HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 0b0010 ) { // east entry
            NormalCube(SHRED_WIDTH/2-4, SHRED_WIDTH/4-1,
                HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 0b0100 ) { // south entry
            NormalCube(SHRED_WIDTH/4-1, SHRED_WIDTH/2-2,
                HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 0b1000 ) { // west entry
            NormalCube(0, SHRED_WIDTH/4-1, HEIGHT/2+1, 2, 2, 2, AIR);
        }
    }
    RandomDrop(rand() % 8, WEAPON, STONE);
    PlantGrass();
} // Shred::Mountain()

void Shred::Desert() {
    NormalUnderground(4, SAND);
    for (int i=0; i<4; ++i) {
        CoverWith(FALLING, SAND);
    }
}

void Shred::WasteShred() {
    NormalUnderground(0, STONE);
    bool room_loaded;
    if ( (room_loaded = LoadRoom(HEIGHT/2)) ) {
        LoadRoom(HEIGHT/2 + 1, 1);
    }
    int random = rand();
    RandomDrop((random % 8)+3, FALLING, SUB_DUST);
    random >>= 3;
    RandomDrop(random % 2, WEAPON, BONE);
    RandomDrop((random % 8)+3, WEAPON, STONE);
    random >>= 3;
    if ( random % 2 && not room_loaded ) {
        random >>= 1;
        const subs pool_sub = ( random % 2 ) ? WATER : STONE;
        random >>= 1;
        const int x = random & 0b1111;
        random >>= 4;
        const int y = random & 0b1111;
        SetNewBlock(LIQUID,  pool_sub, x, y, HEIGHT/2-1);
        SetNewBlock(LIQUID,  pool_sub, x, y, HEIGHT/2  );
        SetNewBlock(FALLING, SUB_DUST, x, y, HEIGHT/2+1);
        SetNewBlock(FALLING, SUB_DUST, x, y, HEIGHT/2+2);
    }
}
