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
#include "World.h"
#include "header.h"
#include "blocks/Block.h"
#include "BlockFactory.h"
#include "worldmap.h"

int Shred::FlatUndeground(int) {
    NormalUnderground();
    return HEIGHT/2;
}

void Shred::NormalUnderground(const int depth, const subs sub) {
    NormalCube(0,0,1, SHRED_WIDTH,SHRED_WIDTH,HEIGHT/2-depth-5, STONE);
    Block * const block = BlockFactory::Normal(sub);
    Block * const stone = BlockFactory::Normal(STONE);
    for (int x=0; x<SHRED_WIDTH; ++x) {
        int rand = qrand();
        for (int y=0; y<SHRED_WIDTH; ++y) {
            PutBlock( ((rand & 1) ? stone : block), x, y, HEIGHT/2-depth-6);
            rand >>= 1;
        }
    }
    NormalCube(0,0,HEIGHT/2-depth-5, SHRED_WIDTH,SHRED_WIDTH,6, sub);
}

void Shred::Plain() {
    NormalUnderground();
    LoadRoom(HEIGHT/2);
    RandomDrop(qrand() & 3, BUSH,   WOOD);
    RandomDrop(qrand() & 3, BLOCK,  ROSE);
    RandomDrop(qrand() & 3, RABBIT, A_MEAT);
    PlantGrass();
}

void Shred::Forest(const bool dead) {
    NormalUnderground();
    RandomDrop(qrand() & 3, BUSH, WOOD);
    for (int number_of_trees = CountShredTypeAround(type);
            number_of_trees != 0; --number_of_trees)
    {
        const int x=qrand()%(SHRED_WIDTH-2);
        const int y=qrand()%(SHRED_WIDTH-2);
        for (int k=HEIGHT-2; ; --k) {
            if ( GetBlock(x, y, k)->Sub() == SOIL ) {
                Tree(x, y, k+1, 4 + (qrand() & 7));
                break;
            }

        }
    }
    RandomDrop(qrand() & 3, WEAPON, WOOD);
    if ( not dead ) {
        RandomDrop(qrand() & 1, BLOCK, ROSE);
        PlantGrass();
    }
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
    const int depth = CountShredTypeAround(GetTypeOfShred()) + 1;
    NormalUnderground(depth, shore);
    int z_start = HEIGHT/2-depth;
    NormalCube(0,0,z_start++, SHRED_WIDTH,SHRED_WIDTH,1, shore); // bottom
    const WorldMap * const map = World::GetWorld()->GetMap();
    if ( type != map->TypeOfShred(longitude-1, latitude) ) { // north
        NormalCube(0,0,z_start, SHRED_WIDTH,1,depth, shore);
    }
    if ( type != map->TypeOfShred(longitude+1, latitude) ) { // south
        NormalCube(0,SHRED_WIDTH-1,z_start, SHRED_WIDTH,1,depth, shore);
    }
    if ( type != map->TypeOfShred(longitude, latitude+1) ) { // east
        NormalCube(SHRED_WIDTH-1,0,z_start, 1,SHRED_WIDTH,depth, shore);
    }
    if ( type != map->TypeOfShred(longitude, latitude-1) ) { // west
        NormalCube(0,0,z_start, 1,SHRED_WIDTH,depth, shore);
    }
    Block * const air = BlockFactory::Normal(AIR);
    for (int i=SHRED_WIDTH; i--; )
    for (int j=SHRED_WIDTH; j--; )
    for (int k=z_start; k<=HEIGHT/2; ++k) {
        if ( shore != GetBlock(i, j, k)->Sub() ) {
            if ( AIR == sub ) {
                PutBlock(air, i, j, k);
            } else {
                SetNewBlock(LIQUID, sub, i, j, k);
            }
        }
    }
}

void Shred::Hill(const bool dead) {
    NormalUnderground();
    Block * const soil = BlockFactory::Normal(SOIL);
    for (int x=SHRED_WIDTH; x--; )
    for (int y=SHRED_WIDTH; y--; )
    for (int z=SHRED_WIDTH/2-2; z--; ) {
        if ( z <= -qAbs(x-SHRED_WIDTH/2) + SHRED_WIDTH/2-2 ) {
            PutBlock(soil, x, y, z+HEIGHT/2); // north-south '^'
        }
        if ( z <= -qAbs(y-SHRED_WIDTH/2) + SHRED_WIDTH/2-2 ) {
            PutBlock(soil, x, y, z+HEIGHT/2); // east-west '^'
        }
    }
    RandomDrop(qrand() & 3, WEAPON, STONE);
    if ( not dead ) {
        int rand = qrand();
        RandomDrop(rand & 3, BLOCK, ROSE);
        RandomDrop((rand >>= 4) & 3, RABBIT, A_MEAT);
        RandomDrop((rand >>= 4) & 3, BUSH, WOOD);
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
    const int mount_top = 3*HEIGHT/4;
    NormalCube(0, 0, 1, SHRED_WIDTH/2, SHRED_WIDTH/2, mount_top, STONE);
    const WorldMap * const map = World::GetWorld()->GetMap();
    // south bridge
    if ( SHRED_MOUNTAIN == map->TypeOfShred(longitude+1, latitude) ) {
        NormalCube(qrand()%(SHRED_WIDTH/2-1), SHRED_WIDTH/2, mount_top,
            2, SHRED_WIDTH/2, 1, STONE);
    }
    // east bridge
    if ( SHRED_MOUNTAIN == map->TypeOfShred(longitude, latitude+1) ) {
        NormalCube(SHRED_WIDTH/2, qrand()%(SHRED_WIDTH/2-1), mount_top,
            SHRED_WIDTH/2, 2, 1, STONE);
    }
    // water pool
    if ( (qrand() & 7) == 0 ) {
        for (int i=1; i<SHRED_WIDTH/2-1; ++i)
        for (int j=1; j<SHRED_WIDTH/2-1; ++j)
        for (int k=mount_top-3; k<=mount_top; ++k) {
            SetNewBlock(LIQUID, WATER, i, j, k);
        }
    }
    // cavern
    if ( (qrand() & 7) == 0 ) {
        NormalCube(SHRED_WIDTH/4-2, SHRED_WIDTH/4-2, HEIGHT/2+1, 4, 4, 3, AIR);
        const int entries = qrand() & 15;
        if ( entries & 1 ) { // north entry
            NormalCube(SHRED_WIDTH/4-1, 0, HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 2 ) { // east entry
            NormalCube(SHRED_WIDTH/2-4, SHRED_WIDTH/4-1,
                HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 4 ) { // south entry
            NormalCube(SHRED_WIDTH/4-1, SHRED_WIDTH/2-2,
                HEIGHT/2+1, 2, 2, 2, AIR);
        }
        if ( entries & 8 ) { // west entry
            NormalCube(0, SHRED_WIDTH/4-1, HEIGHT/2+1, 2, 2, 2, AIR);
        }
    }
    RandomDrop(qrand() & 7, WEAPON, STONE);
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
    int random = qrand();
    RandomDrop((random & 0x7)+3, FALLING, SUB_DUST);
    random >>= 3;
    RandomDrop(random & 0x1, WEAPON, BONE);
    RandomDrop((random & 0x7)+3, WEAPON, STONE);
    random >>= 3;
    if ( random & 0x1 && not room_loaded ) {
        random >>= 1;
        const subs pool_sub = ( random & 0x1 ) ? WATER : STONE;
        random >>= 1;
        const int x = random & 0xF;
        random >>= 4;
        const int y = random & 0xF;
        SetNewBlock(LIQUID,  pool_sub, x, y, HEIGHT/2-1);
        SetNewBlock(LIQUID,  pool_sub, x, y, HEIGHT/2  );
        SetNewBlock(FALLING, SUB_DUST, x, y, HEIGHT/2+1);
        SetNewBlock(FALLING, SUB_DUST, x, y, HEIGHT/2+2);
    }
}
