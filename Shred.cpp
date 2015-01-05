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
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include <algorithm>

const quint8 DATASTREAM_VERSION = QDataStream::Qt_5_2;
const quint8 CURRENT_SHRED_FORMAT_VERSION = 15;

const int RAIN_IS_DEW = 1;

World * Shred::GetWorld() const { return world; }

bool Shred::LoadShred() {
    const QByteArray * const data =
        GetWorld()->GetShredData(longitude, latitude);
    if ( data == nullptr ) return false;
    QDataStream in(*data);
    quint8 read;
    in >>  read;
    if ( Q_UNLIKELY(CURRENT_SHRED_FORMAT_VERSION != read) ) {
        qDebug("%s: Shred format: %d (must be %d). Generating new shred.",
            Q_FUNC_INFO, read, CURRENT_SHRED_FORMAT_VERSION);
        return false;
    } // else:
    in.setVersion(DATASTREAM_VERSION);
    in >> read;
    type = static_cast<shred_type>(read);
    in >> read;
    weather = static_cast<weathers>(read);
    SetAllLightMapNull();
    Block * const null_stone = blockFactory->Normal(NULLSTONE);
    Block * const air        = blockFactory->Normal(AIR);
    for (int x=SHRED_WIDTH; x--; )
    for (int y=SHRED_WIDTH; y--; ) {
        PutBlock(null_stone, x, y, 0);
        for (int z=1; ; ++z) {
            quint8 kind, sub;
            if ( BlockFactory::KindSubFromFile(in, &kind, &sub) ) { // normal
                if ( sub==SKY || sub==STAR ) {
                    std::fill(blocks[x][y] + z, blocks[x][y] + HEIGHT-1, air);
                    PutBlock(blockFactory->Normal(sub), x, y, HEIGHT-1);
                    break;
                } else {
                    PutBlock(blockFactory->Normal(sub), x, y, z);
                }
            } else {
                Active * const active = (blocks[x][y][z] =
                    blockFactory->BlockFromFile(in, kind, sub))->ActiveBlock();
                if ( active != nullptr ) {
                    active->SetXyz(x, y, z);
                    RegisterInit(active);
                    Falling * const falling = active->ShouldFall();
                    if ( falling != nullptr && falling->IsFalling() ) {
                        fallList.push_front(falling);
                    }
                }
            }
        }
    }
    delete data;
    return true;
} // bool Shred::LoadShred()

Shred::Shred(const int shred_x, const int shred_y,
        const long longi, const long lati)
    :
        Weather(),
        longitude(longi), latitude(lati),
        shredX(shred_x), shredY(shred_y),
        type(),
        activeListFrequent(),
        activeListAll(),
        shiningList(),
        fallList(),
        weather(WEATHER_CLEAR)
{
    if ( LoadShred() ) return; // successfull loading
    // new shred generation:
    Block * const null_stone = blockFactory->Normal(NULLSTONE);
    Block * const air  = blockFactory->Normal(AIR);
    Block * const sky  = blockFactory->Normal(SKY);
    Block * const star = blockFactory->Normal(STAR);
    SetAllLightMapNull();
    for (int i=SHRED_WIDTH; i--; )
    for (int j=SHRED_WIDTH; j--; ) {
        PutBlock(null_stone, i, j, 0);
        std::fill(blocks[i][j] + 1, blocks[i][j] + HEIGHT - 1, air);
        PutBlock(((qrand() & 3) ? sky : star), i, j, HEIGHT-1);
    }
    switch ( type = static_cast<shred_type>
            (GetWorld()->GetMap()->TypeOfShred(longi, lati)) )
    {
    case SHRED_WASTE:       WasteShred(); break;
    case SHRED_WATER:       Water();      break;
    case SHRED_PLAIN:       Plain();      break;
    case SHRED_MOUNTAIN:    Mountain();   break;
    case SHRED_DESERT:      Desert();     break;
    case SHRED_HILL:        Hill(false);  break;
    case SHRED_DEAD_HILL:   Hill(true);   break;
    case SHRED_FOREST:      Forest(false); break;
    case SHRED_DEAD_FOREST: Forest(true);  break;
    case SHRED_NULLMOUNTAIN: NullMountain(); break;
    case SHRED_TESTSHRED:   TestShred();  break;
    case SHRED_PYRAMID:     Pyramid();    break;
    case SHRED_CASTLE:      Castle();     break;
    case SHRED_CHAOS:       ChaosShred(); break;
    case SHRED_EMPTY: break;
    case SHRED_ACID_LAKE:   Water(ACID ); break;
    case SHRED_LAVA_LAKE:   Water(STONE); break;
    case SHRED_CRATER:      Water(AIR);   break;
    case SHRED_UNDERGROUND: NormalUnderground(); break;
    }
} // Shred::Shred(int shred_x, shred_y, long longi, lati, Shred * mem)

Shred::~Shred() { SaveShred(true); }

void Shred::SaveShred(const bool isQuitGame) {
    QByteArray * const shred_data = new QByteArray();
    shred_data->reserve(40 * 1024);
    QDataStream outstr(shred_data, QIODevice::WriteOnly);
    outstr << CURRENT_SHRED_FORMAT_VERSION;
    outstr.setVersion(DATASTREAM_VERSION);
    outstr << quint8(GetTypeOfShred()) << quint8(weather);
    for (int x=SHRED_WIDTH; x--; )
    for (int y=SHRED_WIDTH; y--; ) {
        int height = HEIGHT - 1;
        while (GetBlock(x, y, --height)->Sub() == AIR);
        for (int z=1; z<=height; ++z) {
            Block * const block = GetBlock(x, y, z);
            if ( block == blockFactory->Normal(block->Sub()) ) {
                block->SaveNormalToFile(outstr);
            } else {
                block->SaveToFile(outstr);
                if ( isQuitGame ) {
                    delete block; // without unregistering.
                } else {
                    block->RestoreDurabilityAfterSave();
                }
            }
        }
        GetBlock(x, y, HEIGHT-1)->SaveNormalToFile(outstr);
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
    for (auto & i : fallList) {
        if ( i == nullptr ) {
            continue;
        } // else:
        const int x_in = CoordInShred(i->X());
        const int y_in = CoordInShred(i->Y());
        Block * const floor_block = GetBlock(x_in, y_in, i->Z()-1);
        if ( i->Weight() <= 0
                || ( floor_block->PushResult(ANYWHERE) == ENVIRONMENT
                    && floor_block->Sub() != AIR ) )
        {
            i->SetFalling(false);
            i = nullptr;
        } else if ( not world->Move(i->X(), i->Y(), i->Z(), DOWN) ) {
            i->FallDamage();
            i = nullptr;
        }
    }
    for (const auto i : activeListFrequent) {
        if ( i != nullptr ) {
            i->ActFrequent();
        }
    }
}

void Shred::PhysEventsRare() {
    for (const auto i : activeListAll) {
        if ( i != nullptr ) {
            switch ( i->ActInner() ) {
            case INNER_ACTION_ONLY:    break;
            case INNER_ACTION_NONE: i->ActRare(); break;
            case INNER_ACTION_EXPLODE: break; /// \todo add explosion
            case INNER_ACTION_MESSAGE: break;
            }
        }
    }
    activeListAll.remove(nullptr);
    activeListFrequent.removeAll(nullptr);
    fallList.remove(nullptr);
}

void Shred::RegisterInit(Active * const active) {
    active->SetShred(this);
    activeListAll.push_front(active);
    const int should_act = active->ShouldAct();
    if ( should_act & FREQUENT_FIRST ) {
        activeListFrequent.prepend(active);
    } else if ( should_act & FREQUENT_SECOND ) {
        activeListFrequent.append(active);
    }
    AddShining(active);
}

void Shred::Register(Active * const active) {
    RegisterInit(active);
    AddFalling(active);
}

void Shred::Unregister(Active * const active) {
    std::replace(activeListAll.begin(), activeListAll.end(), active,
        static_cast<Active *>(nullptr));
    std::replace(activeListFrequent.begin(), activeListFrequent.end(), active,
        static_cast<Active *>(nullptr));
    Falling * const falling = active->ShouldFall();
    if ( falling != nullptr ) {
        std::replace(fallList.begin(), fallList.end(), falling,
            static_cast<Falling *>(nullptr));
        falling->SetFalling(false);
    }
    RemShining(active);
}

void Shred::AddFalling(Block * const block) {
    Falling * const falling = block->ShouldFall();
    if ( falling != nullptr && not falling->IsFalling() ) {
        falling->SetFalling(true);
        fallList.push_front(falling);
    }
}

void Shred::AddShining(Active * const active) {
    if ( active->LightRadius() != 0 ) {
        shiningList.push_front(active);
    }
}

void Shred::RemShining(Active * const active) { shiningList.remove(active); }

void Shred::ReloadTo(const dirs direction) {
    switch ( direction ) {
    case NORTH: ++shredY; break;
    case SOUTH: --shredY; break;
    case EAST:  --shredX; break;
    case WEST:  ++shredX; break;
    default: Q_UNREACHABLE(); break;
    }
}

void Shred::SetBlock(Block * block, const int x, const int y, const int z) {
    Block * const to_delete = GetBlock(x, y, z);
    if ( to_delete != block ) {
        Active * const active = to_delete->ActiveBlock();
        if ( active ) {
            active->Unregister();
        }
        SetBlockNoCheck(block, x, y, z);
    }
}

void Shred::SetBlockNoCheck(Block * const block,
        const int x, const int y, const int z)
{
    Active * const active = ( blocks[x][y][z]=block )->ActiveBlock();
    if ( active != nullptr ) {
        active->SetXyz(x, y, z);
        Register(active);
    }
}

void Shred::SetNewBlock(const int kind, const int sub,
        const int x, const int y, const int z, const int dir)
{
    Block * const block = blockFactory->NewBlock(kind, sub);
    block->SetDir(dir);
    SetBlock(block, x, y, z);
}

QString Shred::FileName(const QString world_name,
        const long longi, const long lati)
{
    return QString("%1%2/%3-%4.fm").
        arg(home_path).arg(world_name).arg(longi).arg(lati);
}

// shred generators section
// these functions fill space between the lowest nullstone layer and sky.
// so use k from 1 to HEIGHT-2.
void Shred::CoverWith(const int kind, const int sub) {
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j) {
        int k = HEIGHT-2;
        for ( ; AIR==GetBlock(i, j, k)->Sub(); --k);
        SetBlock(blockFactory->NewBlock(kind, sub), i, j, ++k);
    }
}

void Shred::RandomDrop(int num, const int kind, const int sub,
        const bool on_water)
{
    while ( num-- ) {
        DropBlock(blockFactory->NewBlock(kind, sub), on_water);
    }
}

void Shred::DropBlock(Block * const block, const bool on_water) {
    int y = qrand();
    const int x = CoordInShred(y);
    y = CoordInShred(unsigned(y) >> SHRED_WIDTH_BITSHIFT);
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
            SetBlock(blockFactory->NewBlock(GRASS, GREENERY), i, j, k);
        }
    }
}

void Shred::TestShred() {
    const int level = FlatUndeground()+1;
    struct {
        kinds kind;
        subs  sub;
    } set[SHRED_WIDTH/2][SHRED_WIDTH] = {
        { // rows
            {CLOCK, IRON}, {CONTAINER, WOOD}, {FALLING, SAND},
            {BLOCK, GLASS}, {BOX, DIFFERENT}, {PLATE, STONE},
            {BLOCK, NULLSTONE}, {LADDER, NULLSTONE}, {LADDER, GREENERY},
            {LADDER, STONE}, {DWARF, ADAMANTINE}, {BUSH, WOOD},
            {WORKBENCH, IRON}, {DOOR, GLASS}, {WEAPON, IRON},
            {BLOCK, SAND}
        }, {
            {BLOCK, WATER}, {FALLING, WATER}, {DOOR, STONE},
            {BLOCK, CLAY}, {KIND_TEXT, PAPER}, {BELL, IRON},
            {BUCKET, IRON}, {PICK, IRON}, {SHOVEL, IRON},
            {HAMMER, IRON}, {AXE, IRON}, {FALLING, STONE},
            {WEAPON, STONE}, {BLOCK, WOOD}, {KIND_TEXT, GLASS},
            {BLOCK, COAL}
        }, {
            {CLOCK, EXPLOSIVE}, {BLOCK, MOSS_STONE}, {ILLUMINATOR, STONE},
            {ILLUMINATOR, WOOD}, {ILLUMINATOR, IRON}, {ILLUMINATOR, GLASS},
            {CONTAINER, IRON}, {CONTAINER, WATER}, {WEAPON, SKY},
            {LIQUID, SUB_CLOUD}, {RAIN_MACHINE, IRON}, {FALLING, SUB_DUST},
            {BLOCK, ROSE}, {CONVERTER, STONE}, {TELEGRAPH, IRON},
            {MEDKIT, IRON}
        }, {
            {ARMOUR, STEEL}, {HELMET, STEEL}, {BOOTS, STEEL},
            {ACCUMULATOR, ADAMANTINE},
        }
    };
    for (int i=0; i<SHRED_WIDTH/2; ++i)
    for (int j=0; j<SHRED_WIDTH  ; ++j) {
        if ( set[i][j].kind == 0 && set[i][j].sub == 0) continue;
        SetNewBlock(set[i][j].kind, set[i][j].sub, j, i*2, level);
    }
} // void Shred::TestShred()

void Shred::NullMountain() {
    NormalUnderground();
    const int border_level = HEIGHT/2-2;
    NormalCube(0,SHRED_WIDTH/2-1,1, SHRED_WIDTH,2,border_level, NULLSTONE);
    NormalCube(SHRED_WIDTH/2-1,0,1, 2,SHRED_WIDTH,border_level, NULLSTONE);
    Block * const null_stone = blockFactory->Normal(NULLSTONE);
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
    const WorldMap * const map = GetWorld()->GetMap();
    if ( SHRED_NULLMOUNTAIN == map->TypeOfShred(longitude-1, latitude) ) { //N
        NormalCube(7,0,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2, NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == map->TypeOfShred(longitude+1, latitude) ) { //S
        NormalCube(7,SHRED_WIDTH/2+1,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == map->TypeOfShred(longitude, latitude+1) ) { //E
        NormalCube(SHRED_WIDTH/2+1,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == map->TypeOfShred(longitude, latitude-1) ) { //W
        NormalCube(0,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2, NULLSTONE);
    }
}

void Shred::Pyramid() {
    const int level = qMin(FlatUndeground(), HEIGHT-1-16);
    Block * const stone = blockFactory->Normal(STONE);
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
    Block * const air = blockFactory->Normal(AIR);
    PutBlock(air, SHRED_WIDTH/2, 0, level+1); // entrance
    // room below
    NormalCube(1, 1, HEIGHT/2-60, SHRED_WIDTH-2, SHRED_WIDTH-2, 8, AIR);
    for (int z=HEIGHT/2-52; z<=level; ++z) { // horizontal tunnel
        PutBlock(air, SHRED_WIDTH/2, SHRED_WIDTH/2, z);
    }
    SetNewBlock(CONTAINER, STONE, SHRED_WIDTH-2, SHRED_WIDTH-2, level+1);
    Inventory * const inv =
        GetBlock(SHRED_WIDTH-2,SHRED_WIDTH-2, level+1)->HasInventory();
    inv->Get(blockFactory->Normal(GOLD));
    SetNewBlock(PREDATOR, A_MEAT, SHRED_WIDTH-3, SHRED_WIDTH-2, level+1);
}

void Shred::Castle() {
    NormalUnderground();
    const int bottom_level = HEIGHT/2 - 1;
    // basement
    NormalCube(0,0,bottom_level-5, SHRED_WIDTH,  SHRED_WIDTH,  9, IRON);
    NormalCube(2,2,bottom_level-3, SHRED_WIDTH-4,SHRED_WIDTH-4,5, AIR );
    // floors
    int level = bottom_level;
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
        if ( floors == 1 ) return; // roof
        const WorldMap * const map = GetWorld()->GetMap();
        if ( map->TypeOfShred(longitude-1, latitude) == SHRED_CASTLE
                || level == bottom_level )
        {// north pass
            NormalCube(2,0,level+2, SHRED_WIDTH-4,2,4, AIR);
        }
        if ( map->TypeOfShred(longitude+1, latitude) == SHRED_CASTLE
                || level == bottom_level )
        { // south pass
            NormalCube(2,SHRED_WIDTH-2,level+2, SHRED_WIDTH-4,2,4, AIR);
        }
        if ( map->TypeOfShred(longitude, latitude-1) == SHRED_CASTLE
                || level == bottom_level )
        { // west pass
            NormalCube(0,2,level+2, 2,SHRED_WIDTH-4,4, AIR);
        }
        if ( map->TypeOfShred(longitude, latitude+1) == SHRED_CASTLE
                || level == bottom_level )
        { // east pass
            NormalCube(SHRED_WIDTH-2,2,level+2, 2,SHRED_WIDTH-4,4, AIR);
        }
        if ( level == bottom_level + 5 ) {
            LoadRoom(level + 2);
        }
        level += 5;
    }
} // Shred::Castle()

void Shred::ChaosShred() {
    for (int i=0; i<SHRED_WIDTH; ++i)
    for (int j=0; j<SHRED_WIDTH; ++j)
    for (int k=1; k<HEIGHT/2; ++k) {
        int kind = qrand() % LAST_KIND;
        int sub  = qrand() % LAST_SUB;
        SetNewBlock(kind, sub, i, j, k);
    }
}

void Shred::NormalCube(
        const int x_start, const int y_start, const int z_start,
        const int x_size,  const int y_size,  const int z_size, const subs sub)
{
    Q_ASSERT(InBounds(x_start, y_start, z_start) &&
        InBounds(x_start + x_size-1, y_start + y_size-1, z_start + z_size-1));
    Block * const block = blockFactory->Normal(sub);
    for (int x=x_start; x < x_start+x_size; ++x)
    for (int y=y_start; y < y_start+y_size; ++y) {
        std::fill(blocks[x][y]+z_start, blocks[x][y]+z_start+z_size, block);
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
    for (int i=x; i<=x+2; ++i)
    for (int j=y; j<=y+2; ++j) {
        std::fill(blocks[i][j] + leaves_level, blocks[i][j] + z + height,
            blockFactory->Normal(GREENERY));
    }
    std::fill(blocks[x+1][y+1] + qMax(z-1, 1), blocks[x+1][y+1] + z+height-1,
        blockFactory->Normal(WOOD)); // trunk
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
    const WorldMap * const map = GetWorld()->GetMap();
    for (long i=longitude-1; i<=longitude+1; ++i)
    for (long j=latitude -1; j<=latitude +1; ++j) {
        result += ( type == map->TypeOfShred(i, j) );
    }
    return result;
}

bool Shred::InBounds(const int z) { return (0 <= z && z < HEIGHT-1 ); }

bool Shred::InBounds(const int x, const int y) {
    return (0 <= x && x < SHRED_WIDTH) && (0 <= y && y < SHRED_WIDTH);
}

bool Shred::InBounds(const int x, const int y, const int z) {
    return InBounds(x, y) && InBounds(z);
}

void Shred::Dew(const int kind, const int sub) {
    DropBlock(blockFactory->NewBlock(kind, sub), true);
}

void Shred::Rain(const int kind, const int sub) {
    if ( RAIN_IS_DEW == 1 ) { // RAIN_IS_DEW is defined in Freg.pro
        Dew(kind, sub);
        return;
    } // else:
    const int CLOUD_HEIGHT = HEIGHT*3/4;
    int y = qrand();
    const int x = CoordInShred(y);
    y = CoordInShred(unsigned(y) >> SHRED_WIDTH_BITSHIFT);
    const int to_replace_sub = GetBlock(x, y, CLOUD_HEIGHT)->Sub();
    if ( to_replace_sub == AIR || to_replace_sub == SUB_CLOUD ) {
        SetBlock(blockFactory->NewBlock(kind, sub), x, y, CLOUD_HEIGHT);
    }
}

bool Shred::LoadRoom(const int level, const int index) {
    QFile file(QString("%1%2.room").
            arg(FileName(GetWorld()->WorldName(), longitude, latitude)).
            arg((index >= 1 ) ?
                QString("-%1").arg(index) : ""));
    if ( not file.open(QIODevice::ReadOnly | QIODevice::Text) ) return false;
    for (int lines = 0; lines < SHRED_WIDTH; ++lines) {
        char buffer[SHRED_WIDTH + 1]{0};
        file.readLine(buffer, sizeof(buffer));
        for (unsigned i=0; i<SHRED_WIDTH; ++i) {
            switch ( buffer[i] ) {
            case '#':
                PutBlock(blockFactory->Normal(STONE), i, lines, level);
                break;
            case '+':
                NormalCube(i, lines, level, 1, 1, qrand()%3+5, STONE);
                break;
            case '0':
                NormalCube(i, lines, level, 1, 1, 6, NULLSTONE);
                break;
            case '|':
                NormalCube(i, lines, level, 1, 1, 5, STONE);
                break;
            case '[': { // window
                Block * const stone = blockFactory->Normal(STONE);
                PutBlock(stone, i, lines, level);
                PutBlock(stone, i, lines, level+1); // level+2 is missing
                PutBlock(stone, i, lines, level+3); // because it is window.
                PutBlock(stone, i, lines, level+4);
                } break;
            case '=': // floor and ceiling
                PutBlock(blockFactory->Normal(WOOD),  i, lines, level);
                PutBlock(blockFactory->Normal(STONE), i, lines, level+4);
                break;
            case '^':
                for (int z=level; z<level+5; ++z) {
                    SetNewBlock(LADDER, WOOD, i, lines, z);
                }
                break;

            case '*': SetNewBlock(WORKBENCH,    IRON,  i, lines, level); break;
            case '&': SetNewBlock(CONTAINER,    WOOD,  i, lines, level); break;
            case 't': SetNewBlock(TELEGRAPH,    IRON,  i, lines, level); break;
            case 'c': SetNewBlock(CLOCK,        IRON,  i, lines, level); break;
            case 'V': SetNewBlock(CONVERTER,    STONE, i, lines, level); break;
            case 'b': SetNewBlock(BELL,         IRON,  i, lines, level); break;
            case 'i': SetNewBlock(ILLUMINATOR,  IRON,  i, lines, level); break;
            case '~': SetNewBlock(LIQUID,       WATER, i, lines, level); break;
            case 'M': SetNewBlock(MEDKIT,       IRON,  i, lines, level); break;
            case 'R': SetNewBlock(RAIN_MACHINE, IRON,  i, lines, level); break;
            case 'F': SetNewBlock(FILTER,       IRON,  i, lines, level); break;

            case '.': // reserved for nothing
            case 0:
            case '\n':
            default: break;
            }
        }
    }
    return true;
} // bool Shred::LoadRoom(const int level, const int index)
