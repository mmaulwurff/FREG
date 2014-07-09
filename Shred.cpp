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

#include <QDataStream>
#include "Shred.h"
#include "world.h"
#include "BlockManager.h"
#include "blocks/Active.h"
#include "blocks/Inventory.h"

const quint8 DATASTREAM_VERSION = QDataStream::Qt_5_2;
const quint8 CURRENT_SHRED_FORMAT_VERSION = 6;

const int RAIN_IS_DEW = 1;

long Shred::Longitude() const { return longitude; }
long Shred::Latitude()  const { return latitude;  }
int  Shred::ShredX() const { return shredX; }
int  Shred::ShredY() const { return shredY; }
char Shred::GetTypeOfShred() const { return type; }
World * Shred::GetWorld() const { return world; }
Shred * Shred::GetShredMemory() const { return memory; }
Block * Shred::Normal(const int sub) { return block_manager.NormalBlock(sub); }

bool Shred::LoadShred() {
    const QByteArray * const data =
        GetWorld()->GetShredData(longitude, latitude);
    if ( data == nullptr ) return false;
    QDataStream in(*data);
    quint8  dataStreamVersion;
    quint8 shredFormatVersion;
    in >> dataStreamVersion >> shredFormatVersion;
    if ( Q_UNLIKELY(DATASTREAM_VERSION != dataStreamVersion) ) {
        fprintf(stderr, "%s: %d (must be %d). Generating new shred.\n",
            Q_FUNC_INFO, dataStreamVersion, DATASTREAM_VERSION);
        return false;
    } // else:
    if ( Q_UNLIKELY(CURRENT_SHRED_FORMAT_VERSION != shredFormatVersion) ) {
        fprintf(stderr,
            "%s: Shred format: %d (must be %d). Generating new shred.\n",
            Q_FUNC_INFO, shredFormatVersion, CURRENT_SHRED_FORMAT_VERSION);
        return false;
    } // else:
    in.setVersion(DATASTREAM_VERSION);
    quint8 read_type;
    in >> read_type;
    type = read_type;
    Block * const null_stone = Normal(NULLSTONE);
    Block * const air = Normal(AIR);
    SetAllLightMapNull();
    for (int x=0; x<SHRED_WIDTH; ++x)
    for (int y=0; y<SHRED_WIDTH; ++y) {
        PutBlock(null_stone, x, y, 0);
        for (int z=1; ; ++z) {
            int kind, sub;
            const bool normal = block_manager.KindSubFromFile(in, &kind, &sub);
            if ( normal ) {
                if ( sub==SKY || sub==STAR ) {
                    for ( ; z < HEIGHT-1; ++z) {
                        PutBlock(air, x, y, z);
                    }
                    PutBlock(Normal(sub), x, y, HEIGHT-1);
                    break;
                } else {
                    PutBlock(Normal(sub), x, y, z);
                }
            } else {
                SetBlockNoCheck(block_manager.BlockFromFile(in, kind, sub),
                    x, y, z);
            }
        }
    }
    return true;
} // bool Shred::LoadShred()

Shred::Shred(const int shred_x, const int shred_y,
        const long longi, const long lati, Shred * const mem)
    :
        longitude(longi), latitude(lati),
        shredX(shred_x), shredY(shred_y),
        memory(mem)
{
    if ( LoadShred() ) return; // successfull loading
    // new shred generation:
    Block * const null_stone = Normal(NULLSTONE);
    Block * const air  = Normal(AIR);
    Block * const sky  = Normal(SKY);
    Block * const star = Normal(STAR);
    SetAllLightMapNull();
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j) {
        PutBlock(null_stone, i, j, 0);
        for (int k=1; k<HEIGHT-1; ++k) {
            PutBlock(air, i, j, k);
        }
        PutBlock(((qrand()%5) ? sky : star), i, j, HEIGHT-1);
    }
    switch ( type=GetWorld()->TypeOfShred(longi, lati) ) {
    case SHRED_WATER:     Water();     break;
    case SHRED_PLAIN:     Plain();     break;
    case SHRED_FOREST:    Forest();    break;
    case SHRED_HILL:      Hill();      break;
    case SHRED_MOUNTAIN:  Mountain();  break;
    case SHRED_DESERT:    Desert();    break;
    case SHRED_NULLMOUNTAIN: NullMountain(); break;
    case SHRED_TESTSHRED: TestShred(); break;
    case SHRED_PYRAMID:   Pyramid();   break;
    case SHRED_CASTLE:    Castle();    break;
    case SHRED_CHAOS:     ChaosShred(); break;
    case SHRED_NORMAL_UNDERGROUND: NormalUnderground(); break;
    case SHRED_EMPTY: break;
    case SHRED_ACID_LAKE: Water(ACID ); break;
    case SHRED_LAVA_LAKE: Water(STONE); break;
    default:
        fprintf(stderr, "%s: type: %c, code %d?\n", Q_FUNC_INFO, type, type);
        Plain();
    }
} // Shred::Shred(int shred_x, shred_y, long longi, lati, Shred * mem)

Shred::~Shred() {
    QByteArray * const shred_data = new QByteArray();
    shred_data->reserve(30000);
    QDataStream outstr(shred_data, QIODevice::WriteOnly);
    outstr << DATASTREAM_VERSION << CURRENT_SHRED_FORMAT_VERSION;
    outstr.setVersion(DATASTREAM_VERSION);
    outstr << quint8(GetTypeOfShred());
    for (int x=0; x<SHRED_WIDTH; ++x)
    for (int y=0; y<SHRED_WIDTH; ++y) {
        int height = HEIGHT-2;
        for ( ; blocks[x][y][height]->Sub()==AIR; --height);
        for (int z=1; z <= height; ++z) {
            Block * const block = blocks[x][y][z];
            if ( block == block_manager.NormalBlock(block->Sub()) ) {
                block->SaveNormalToFile(outstr);
            } else {
                block->SaveToFile(outstr);
                delete block;
            }
        }
        blocks[x][y][HEIGHT-1]->SaveNormalToFile(outstr);
    }
    GetWorld()->SetShredData(shred_data, longitude, latitude);
}

long Shred::GlobalX(const int x) const {
    return (Latitude()  - CoordOfShred(x))*SHRED_WIDTH + x;
}

long Shred::GlobalY(const int y) const {
    return (Longitude() - CoordOfShred(y))*SHRED_WIDTH + y;
}

void Shred::PhysEventsFrequent() {
    for (auto i = fallList.begin(); i != fallList.end(); ++i) {
        if ( *i == nullptr ) {
            continue;
        } else if ( (*i)->Weight() <= 0 ) {
            (*i)->SetFalling(false);
            *i = nullptr;
        } else if ( not world->Move((*i)->X(), (*i)->Y(), (*i)->Z(), DOWN) ) {
            (*i)->FallDamage();
            *i = nullptr;
        }
    }
    for (auto i  = activeListFrequent.constBegin();
              i != activeListFrequent.constEnd(); ++i)
    {
        if ( *i != nullptr ) {
            (*i)->ActFrequent();
        }
    }
}

void Shred::PhysEventsRare() {
    for (auto i=activeListAll.constBegin(); i!=activeListAll.constEnd(); ++i) {
        if ( *i != nullptr ) {
            switch ( (*i)->ActInner() ) {
            case INNER_ACTION_MESSAGE:
            case INNER_ACTION_EXPLODE: // TODO: add explosion
            case INNER_ACTION_NONE: (*i)->ActRare(); break;
            }
        }
    }
    activeListAll.removeAll(nullptr);
    activeListFrequent.removeAll(nullptr);
    fallList.removeAll(nullptr);
}

void Shred::Register(Active * const active) {
    active->SetShred(this);
    activeListAll.append(active);
    const int should_act = active->ShouldAct();
    if ( should_act & FREQUENT_FIRST ) {
        activeListFrequent.prepend(active);
    } else if ( should_act & FREQUENT_SECOND ) {
        activeListFrequent.append(active);
    }
    AddFalling(active);
    AddShining(active);
}

void Shred::Unregister(Active * const active) {
    *qFind(activeListAll.begin(), activeListAll.end(), active) = nullptr;
    *qFind(activeListFrequent.begin(), activeListFrequent.end(), active) =
        nullptr;
    Falling * const falling = active->ShouldFall();
    if ( falling != nullptr ) {
        *qFind(fallList.begin(), fallList.end(), falling) = nullptr;
    }
    RemShining(active);
}

void Shred::AddFalling(Block * const block) {
    Falling * const falling = block->ShouldFall();
    if ( falling != nullptr ) {
        falling->SetFalling(true);
        fallList.append(falling);
    }
}

void Shred::AddShining(Active * const active) {
    if ( active->LightRadius() != 0 ) {
        shiningList.append(active);
    }
}

void Shred::RemShining(Active * const active) {shiningList.removeOne(active);}

QLinkedList<Active * const>::const_iterator Shred::ShiningBegin() const {
    return shiningList.constBegin();
}

QLinkedList<Active * const>::const_iterator Shred::ShiningEnd() const {
    return shiningList.constEnd();
}

void Shred::ReloadTo(const dirs direction) {
    void (Active::* reload)() = nullptr;
    switch ( direction ) {
    case NORTH: reload = &Active::ReloadToNorth; ++shredY; break;
    case SOUTH: reload = &Active::ReloadToSouth; --shredY; break;
    case EAST:  reload = &Active::ReloadToEast;  --shredX; break;
    case WEST:  reload = &Active::ReloadToWest;  ++shredX; break;
    default: Q_UNREACHABLE(); break;
    }
    activeListAll.removeAll(nullptr);
    for (auto i=activeListAll.constBegin(); i!=activeListAll.constEnd(); ++i) {
        ((*i)->*reload)();
    }
}

void Shred::SetBlock(Block * block, const int x, const int y, const int z) {
    Block * const to_delete = GetBlock(x, y, z);
    if ( to_delete != block ) {
        block_manager.DeleteBlock(to_delete);
        block = block_manager.ReplaceWithNormal(block);
        SetBlockNoCheck(block, x, y, z);
    }
}

void Shred::SetBlockNoCheck(Block * const block,
        const int x, const int y, const int z)
{
    Active * const active = ( blocks[x][y][z]=block )->ActiveBlock();
    if ( active != nullptr ) {
        active->SetXyz(
            (ShredX() << SHRED_WIDTH_SHIFT) | x,
            (ShredY() << SHRED_WIDTH_SHIFT) | y, z );
        Register(active);
    }
}

void Shred::SetNewBlock(const int kind, const int sub,
        const int x, const int y, const int z, const int dir)
{
    Block * const block = block_manager.NewBlock(kind, sub);
    block->SetDir(dir);
    SetBlock(block, x, y, z);
}

void Shred::PutBlock(Block * const block,
        const int x, const int y, const int z)
{
    blocks[x][y][z] = block;
}

QString Shred::FileName() const {
    return FileName(GetWorld()->WorldName(), longitude, latitude);
}

QString Shred::FileName(const QString world_name,
        const long longi, const long lati)
{
    return QString("%1/y%2x%3").arg(world_name).arg(longi).arg(lati);
}

// shred generators section
// these functions fill space between the lowest nullstone layer and sky.
// so use k from 1 to HEIGHT-2.
void Shred::CoverWith(const int kind, const int sub) {
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j) {
        int k = HEIGHT-2;
        for ( ; AIR==GetBlock(i, j, k)->Sub(); --k);
        SetBlock(block_manager.NewBlock(kind, sub), i, j, ++k);
    }
}

void Shred::RandomDrop(int num, const int kind, const int sub,
        const bool on_water)
{
    for ( ; num!=0; --num) {
        DropBlock(block_manager.NewBlock(kind, sub), on_water);
    }
}

void Shred::DropBlock(Block * const block, const bool on_water) {
    int y = qrand();
    const int x = CoordInShred(y);
    y = CoordInShred(unsigned(y) >> SHRED_WIDTH_SHIFT);
    int z = HEIGHT-2;
    for ( ; GetBlock(x, y, z)->Sub()==AIR; --z);
    if( on_water || GetBlock(x, y, z)->Sub()!=WATER ) {
        SetBlock(block, x, y, ++z);
    }
}

void Shred::PlantGrass() {
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j) {
        int k = HEIGHT - 2;
        for ( ; GetBlock(i, j, k)->Transparent(); --k);
        if ( SOIL==GetBlock(i, j, k)->Sub()
                && AIR==GetBlock(i, j, ++k)->Sub() )
        {
            SetBlock(block_manager.NewBlock(GRASS, GREENERY), i, j, k);
        }
    }
}

void Shred::TestShred() { // 7 items in a row
    const int level = FlatUndeground()+1;
    int row = 1, column = -1;
    // row 1
    SetNewBlock(CLOCK,     IRON, column+=2, row, level);
    SetNewBlock(CONTAINER, WOOD, column+=2, row, level);
    SetNewBlock(FALLING,   SAND, column+=2, row, level);
    PutBlock(Normal(GLASS),      column+=2, row, level);
    SetNewBlock(CONTAINER, DIFFERENT, column+=2, row, level);
    SetNewBlock(PLATE, STONE, column+=2, row, level);
    PutBlock(Normal(NULLSTONE), column+=2, row, level);
    // row 2
    column = -1;
    row += 2;
    SetNewBlock(LADDER, NULLSTONE, column+=2, row, level);
    SetNewBlock(LADDER, GREENERY,  column,    row, level+1);
    SetNewBlock(LADDER, STONE,     column,    row, level+2);
    // tall ladder
    for (int i=level+3; i<=level+20 && i<HEIGHT-1; ++i) {
        SetNewBlock(LADDER, WOOD, column, row, i);
    }
    SetNewBlock(DWARF, H_MEAT, column+=2, row, level);
    SetNewBlock(LIQUID, WATER, column+=2, row, level - 3);
    SetNewBlock(LIQUID, WATER, column, row, level - 2);
    PutBlock(Normal(AIR), column, row, level - 1);
    SetNewBlock(BUSH,   WOOD,   column+=2, row, level);
    SetNewBlock(RABBIT, A_MEAT, column+=2, row, level - 2);
    PutBlock(Normal(AIR), column, row, level - 1);
    SetNewBlock(WORKBENCH, IRON,  column+=2, row, level);
    SetNewBlock(DOOR,      GLASS, column+=2, row, level);
    blocks[column][row][level]->SetDir(NORTH);
    // row 3
    column = -1;
    row += 2;
    SetNewBlock(WEAPON,  IRON,  column+=2, row, level);
    SetNewBlock(BLOCK,   SAND,  column+=2, row, level);
    SetNewBlock(BLOCK,   WATER, column+=2, row, level);
    SetNewBlock(FALLING, WATER, column+=2, row, level);
    SetNewBlock(DOOR,   STONE, column+=2, row, level);
    blocks[column][row][level]->SetDir(NORTH);
    SetNewBlock(BLOCK,  CLAY,  column+=2, row, level);
    NormalCube(++column, row-1, level, 3, 3, 3, GLASS);
    SetNewBlock(LIQUID, STONE, ++column, row, level+1);
    // row 4
    column = -1;
    row += 2;
    SetNewBlock(TEXT, PAPER, column+=2, row, level);
    GetBlock(column, row, level)->Inscribe(".hidden");
    SetNewBlock(BELL,   IRON, column+=2, row, level);
    SetNewBlock(BUCKET, IRON, column+=2, row, level);
    SetNewBlock(PICK,   IRON, column+=2, row, level);
    SetNewBlock(SHOVEL, IRON, column+=2, row, level);
    SetNewBlock(HAMMER, IRON, column+=2, row, level);
    SetNewBlock(AXE,    IRON, column+=2, row, level);
    // row 5
    column = -1;
    row += 2;
    SetNewBlock(FALLING, STONE, column+=2, row, level);
    SetNewBlock(WEAPON,  STONE, column+=2, row, level);
    SetNewBlock(GRASS,   FIRE,  column+=2, row, level);
    SetNewBlock(BLOCK,   WOOD,  column, row, level-1);
    SetNewBlock(TEXT,    GLASS, column+=2, row, level);
    PutBlock(Normal(COAL), column+=2, row, level);
    SetNewBlock(CLOCK, EXPLOSIVE, column+=2, row, level);
    PutBlock(Normal(MOSS_STONE),  column+=2, row, level);
    // row 6
    column = -1;
    row += 2;
    SetNewBlock(ILLUMINATOR, STONE, column+=2, row, level);
    SetNewBlock(ILLUMINATOR, WOOD,  column+=2, row, level);
    SetNewBlock(ILLUMINATOR, IRON,  column+=2, row, level);
    SetNewBlock(ILLUMINATOR, GLASS, column+=2, row, level);
    SetNewBlock(CONTAINER,   IRON,  column+=2, row, level);
    SetNewBlock(CONTAINER,   WATER, column+=2, row, level);
    PutBlock(Normal(STONE), column+=2, row, level);
    // row 7
    column = -1;
    row += 2;
    SetNewBlock(WEAPON, SKY, column+=2, row, level);
    NormalCube(++column, row-1, level, 3, 3, 3, GLASS);
    SetNewBlock(LIQUID, ACID, ++column, row, level+1);
    SetNewBlock(LIQUID, SUB_CLOUD, column+=2, row, level);
    SetNewBlock(RAIN_MACHINE, IRON, column+=2, row, level);
} // void Shred::TestShred()

void Shred::NullMountain() {
    NormalUnderground();
    const int border_level = HEIGHT/2-2;
    NormalCube(0,SHRED_WIDTH/2-1,1, SHRED_WIDTH,2,border_level, NULLSTONE);
    NormalCube(SHRED_WIDTH/2-1,0,1, 2,SHRED_WIDTH,border_level, NULLSTONE);
    Block * const null_stone = Normal(NULLSTONE);
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j) {
        for (int k=border_level; k < HEIGHT-2; ++k) {
            const int surface =
                HEIGHT/2 * (pow(1./(i-7.5), 2) * pow(1./(j-7.5), 2)+1);
            if ( HEIGHT/2+1 < surface && surface >= k ) {
                PutBlock(null_stone, i, j, k);
            }
        }
    }
    World * const world = GetWorld();
    if ( SHRED_NULLMOUNTAIN == world->TypeOfShred(longitude-1, latitude) ) {//N
        NormalCube(7,0,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2, NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == world->TypeOfShred(longitude+1, latitude) ) {//S
        NormalCube(7,SHRED_WIDTH/2+1,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == world->TypeOfShred(longitude, latitude+1) ) {//E
        NormalCube(SHRED_WIDTH/2+1,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == world->TypeOfShred(longitude, latitude-1) ) {//W
        NormalCube(0,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2, NULLSTONE);
    }
}

void Shred::Pyramid() {
    const int level = qMin(FlatUndeground(), HEIGHT-1-16);
    Block * const stone = Normal(STONE);
    for (int z=level+1, dz=0; dz<SHRED_WIDTH/2; z+=2, ++dz) { // pyramid
        for (int x=dz, y=dz; x<(SHRED_WIDTH - dz); ++x, ++y) {
            blocks[x][dz][z] =
                blocks[x][SHRED_WIDTH-1-dz][z  ] =
                blocks[x][              dz][z+1] =
                blocks[x][SHRED_WIDTH-1-dz][z+1] =
            blocks[dz][y][z] =
                blocks[SHRED_WIDTH-1-dz][y][z  ] =
                blocks[              dz][y][z+1] =
                blocks[SHRED_WIDTH-1-dz][y][z+1] = stone;
        }
    }
    Block * const air = Normal(AIR);
    PutBlock(air, SHRED_WIDTH/2, 0, level+1); // entrance
    // room below
    NormalCube(1, 1, HEIGHT/2-60, SHRED_WIDTH-2, SHRED_WIDTH-2, 8, AIR);
    for (int z=HEIGHT/2-52; z<=level; ++z) { // horizontal tunnel
        PutBlock(air, SHRED_WIDTH/2, SHRED_WIDTH/2, z);
    }
    SetNewBlock(CONTAINER, STONE, SHRED_WIDTH-2, SHRED_WIDTH-2, level+1);
    Inventory * const inv =
        GetBlock(SHRED_WIDTH-2,SHRED_WIDTH-2, level+1)->HasInventory();
    inv->Get(Normal(GOLD));
    SetNewBlock(PREDATOR, A_MEAT, SHRED_WIDTH-3, SHRED_WIDTH-2, level+1);
}

void Shred::Castle() {
    NormalUnderground();
    // basement
    NormalCube(0,0,HEIGHT/2-6, SHRED_WIDTH,  SHRED_WIDTH,  9, IRON);
    NormalCube(2,2,HEIGHT/2-4, SHRED_WIDTH-4,SHRED_WIDTH-4,5, AIR );
    // floors
    int level = HEIGHT/2-1;
    for (int floors=CountShredTypeAround(SHRED_CASTLE); ; --floors) {
        NormalCube(0,0,level,   SHRED_WIDTH,  SHRED_WIDTH,  6, STONE);
        NormalCube(2,2,level+1, SHRED_WIDTH-4,SHRED_WIDTH-4,1, WOOD );
        NormalCube(2,2,level+2, SHRED_WIDTH-4,SHRED_WIDTH-4,5, AIR  );
        // stairs down
        NormalCube(4,2,level, 5,2,2, AIR);
        for (int y=2; y<=3; ++y) {
            for (int step=0; step<5; ++step) {
                SetNewBlock(PLATE, STONE, 4+step, y, level-3+step);
            }
        }
        if ( floors != 1 ) { // not roof, lamps
            for (int x=3; x<SHRED_WIDTH-3; x+=3)
            for (int y=3; y<SHRED_WIDTH-3; y+=3) {
                SetNewBlock(ILLUMINATOR, GLASS, x, y, level+4);
            }
        } else {
            return;
        }
        World * const world = GetWorld();
        // north pass and lamps
        if ( world->TypeOfShred(longitude-1, latitude) == SHRED_CASTLE ) {
            NormalCube(2,0,level+2, SHRED_WIDTH-4,2,4, AIR);
            for (int x=3; x<SHRED_WIDTH-3; x+=3) {
                SetNewBlock(ILLUMINATOR, GLASS, x, 0, level+4);
            }
        }
        // south pass
        if ( world->TypeOfShred(longitude+1, latitude) == SHRED_CASTLE ) {
            NormalCube(2,SHRED_WIDTH-2,level+2, SHRED_WIDTH-4,2,4, AIR);
        }
        // west pass and lamps
        if ( world->TypeOfShred(longitude, latitude-1) == SHRED_CASTLE ) {
            NormalCube(0,2,level+2, 2,SHRED_WIDTH,4, AIR);
            for (int y=3; y<SHRED_WIDTH-3; y+=3) {
                SetNewBlock(ILLUMINATOR, GLASS, 0, y, level+4);
            }
        }
        // east pass
        if ( world->TypeOfShred(longitude, latitude+1) == SHRED_CASTLE ) {
            NormalCube(SHRED_WIDTH-2,2,level+2, 2,SHRED_WIDTH,4, AIR);
        }
        level += 5;
    }
}

void Shred::ChaosShred() {
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j)
    for (int k=1; k<HEIGHT/2; ++k) {
        int kind = qrand() % LAST_KIND;
        int sub  = qrand() % LAST_SUB;
        if ( kind==TELEGRAPH || kind==ANIMAL ) {
            kind = BLOCK;
        }
        if ( sub==AIR || sub==STAR || sub==SUN_MOON || sub==SKY ) {
            sub = STONE;
        }
        SetNewBlock(kind, sub, i, j, k);
    }
}

void Shred::NormalCube(
        const int x_start, const int y_start, const int z_start,
        const int x_size,  const int y_size,  const int z_size, const subs sub)
{
    Block * const block = Normal(sub);
    for (int x=x_start; x < x_start+x_size; ++x)
    for (int y=y_start; y < y_start+y_size; ++y)
    for (int z=z_start; z < z_start+z_size; ++z) {
        if ( InBounds(x, y, z) ) {
            PutBlock(block, x, y, z);
        }
    }
}

bool Shred::Tree(const int x, const int y, const int z, const int height) {
    if ( not InBounds(x+2, y+2, height+z) ) return false;
    // check for room
    for (int i=x; i<=x+2; ++i)
    for (int j=y; j<=y+2; ++j)
    for (int k=z; k<z+height; ++k) {
        if ( AIR != GetBlock(i, j, k)->Sub() ) {
            return false;
        }
    }
    const int leaves_level = z+height/2;
    Block * const leaves = Normal(GREENERY);
    for (int i=x; i<=x+2; ++i)
    for (int j=y; j<=y+2; ++j) {
        for (int k=leaves_level; k<z+height; ++k ) {
            PutBlock(leaves, i, j, k);
        }
    }
    for (int k=qMax(z-1, 1); k < z+height-1; ++k) { // trunk
        SetBlock(Normal(WOOD), x+1, y+1, k);
    }
    // branches
    const int r = qrand();
    if ( r & 0x1 ) SetNewBlock(BLOCK, WOOD, x,   y+1, leaves_level, WEST);
    if ( r & 0x2 ) SetNewBlock(BLOCK, WOOD, x+2, y+1, leaves_level, EAST);
    if ( r & 0x4 ) SetNewBlock(BLOCK, WOOD, x+1, y,   leaves_level, NORTH);
    if ( r & 0x8 ) SetNewBlock(BLOCK, WOOD, x+1, y+2, leaves_level, SOUTH);
    return true;
}

int Shred::CountShredTypeAround(const int type) const {
    int result = 0;
    World * const world = GetWorld();
    for (long i=longitude-1; i<=longitude+1; ++i)
    for (long j=latitude -1; j<=latitude +1; ++j) {
        result += ( type == world->TypeOfShred(i, j) );
    }
    return result;
}

bool Shred::InBounds(const int x, const int y, const int z) {
    return ( (0 <= x && x < SHRED_WIDTH) && (0 <= y && y < SHRED_WIDTH)
        && 0 <=z && z < HEIGHT-1 );
}

void Shred::Dew(const int kind, const int sub) {
    DropBlock(block_manager.NewBlock(kind, sub), true);
}

void Shred::Rain(const int kind, const int sub) {
    if ( RAIN_IS_DEW == 1 ) { // RAIN_IS_DEW is defined in Freg.pro
        Dew(kind, sub);
        return;
    } // else:
    static const int CLOUD_HEIGHT = HEIGHT*3/4;
    int y = qrand();
    const int x = CoordInShred(y);
    y = CoordInShred(unsigned(y) >> SHRED_WIDTH_SHIFT);
    const int to_replace_sub = GetBlock(x, y, CLOUD_HEIGHT)->Sub();
    if ( to_replace_sub == AIR || to_replace_sub == SUB_CLOUD ) {
        SetBlock(block_manager.NewBlock(kind, sub), x, y, CLOUD_HEIGHT);
    }
}

