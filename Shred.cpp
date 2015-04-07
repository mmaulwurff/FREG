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
#include "WorldMap.h"
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include "BlockFactory.h"
#include "TrManager.h"
#include "AroundCoordinates.h"
#include "ShredStorage.h"
#include <QTextStream>
#include <QFile>

const quint8 Shred::DATASTREAM_VERSION = QDataStream::Qt_5_2;

bool Shred::LoadShred() {
    ShredStorage* const shredStorage = World::GetWorld()->GetShredStorage();
    QByteArray* const data = shredStorage->GetShredData(longitude,latitude);
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
    SetWeather(static_cast<weathers>(read));
    Block* const null_stone = BlockFactory::Normal(NULLSTONE);
    Block* const air        = BlockFactory::Normal(AIR);
    Block* const sky        = BlockFactory::Normal(SKY); // used as column end.
    FOR_ALL_SHRED_AREA(x, y) {
        PutBlock(null_stone, x, y, 0);
        for (int z = 1; ; ++z) {
            quint8 kind, sub;
            if ( BlockFactory::KindSubFromFile(in, &kind, &sub) ) { // normal
                if ( sub != SKY ) {
                    PutBlock(BlockFactory::Normal(sub), x, y, z);
                } else {
                    std::fill(blocks[x][y] + z, blocks[x][y] + HEIGHT-1, air);
                    PutBlock(sky, x, y, HEIGHT-1);
                    break;
                }
            } else if ( Active* const active = (blocks[x][y][z] =
                    BlockFactory::BlockFromFile( in,
                        static_cast<kinds>(kind),
                        static_cast<subs> (sub)) )->ActiveBlock() )
            {
                active->SetXyz(x, y, z);
                RegisterInit(active);
                Falling* const falling = active->ShouldFall();
                if ( Q_UNLIKELY(falling && falling->IsFalling()) ) {
                    fallList.push_front(falling);
                }
            }
        }
    }
    shredStorage->ReleaseByteArray(data);
    return true;
} // bool Shred::LoadShred()

Shred::Shred(const int shred_x, const int shred_y,
        const qint64 longi, const qint64 lati)
    :
        Weather(WEATHER_CLEAR),
        lightMap { { {0} } },
        longitude(longi),
        latitude(lati),
        shredX(shred_x),
        shredY(shred_y),
        type(static_cast<shred_type>(
            World::GetConstWorld()->GetMap()->TypeOfShred(longi, lati) )),
        activeListFrequent(),
        activeListAll(),
        shiningList(),
        fallList()
{
    if ( LoadShred() ) return; // successful loading
    // new shred generation:
    Block* pattern[HEIGHT];
    pattern[0] = BlockFactory::Normal(NULLSTONE);
    std::fill(pattern + 1, pattern + HEIGHT - 1, BlockFactory::Normal(AIR));
    pattern[HEIGHT - 1] = BlockFactory::Normal(SKY);
    FOR_ALL_SHRED_AREA(x, y) {
        memcpy(blocks[x][y], pattern, sizeof(Block*) * HEIGHT);
    }
    switch ( type ) {
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
    case SHRED_TEST_SHRED:  TestShred();  break;
    case SHRED_PYRAMID:     Pyramid();    break;
    case SHRED_CASTLE:      Castle();     break;
    case SHRED_CHAOS:       ChaosShred(); break;
    case SHRED_EMPTY: break;
    case SHRED_ACID_LAKE:   Water(ACID ); break;
    case SHRED_LAVA_LAKE:   Water(STONE); break;
    case SHRED_CRATER:      Water(AIR);   break;
    case SHRED_FLAT:        Layers();     break;
    case SHRED_UNDERGROUND: NormalUnderground(); break;
    }
} // Shred::Shred(int shred_x, shred_y, qint64 longi, lati)

Shred::~Shred() { SaveShred(true); }

void Shred::SaveShred(const bool isQuitGame) {
    ShredStorage* const storage = World::GetWorld()->GetShredStorage();
    QByteArray* const shred_data = storage->GetByteArray();
    QDataStream out_stream(shred_data, QIODevice::WriteOnly);
    out_stream << CURRENT_SHRED_FORMAT_VERSION;
    out_stream.setVersion(DATASTREAM_VERSION);
    out_stream << quint8(GetTypeOfShred()) << quint8(GetWeather());
    const Block* const sky = BlockFactory::Normal(SKY);
    FOR_ALL_SHRED_AREA(x, y) {
        const int ground_z = FindTopNonAir(x, y);
        for (int z=1; z<=ground_z; ++z) {
            Block* const block = GetBlock(x, y, z);
            if ( block == BlockFactory::Normal(block->Sub()) ) {
                block->SaveNormalToFile(out_stream);
            } else {
                block->SaveToFile(out_stream);
                if ( isQuitGame ) {
                    delete block; // without unregistering.
                } else {
                    block->RestoreDurabilityAfterSave();
                }
            }
        }
        sky->SaveNormalToFile(out_stream);
    }
    storage->SetShredData(shred_data, longitude, latitude);
}

int Shred::FindTopNonAir(const int x, const int y) {
    int z = HEIGHT - 1;
    while ( blocks[x][y][--z]->Sub() == AIR );
    return z;
}

const Block* Shred::FindFirstVisible(const int x, const int y, int* const z,
        const int step)
const {
    for (; GetBlock(x, y, *z)->Transparent() == INVISIBLE; *z += step);
    return GetBlock(x, y, *z);
}

qint64 Shred::GlobalX(const int x) const {
    return (Latitude()  - CoordOfShred(x))*SHRED_WIDTH + x;
}

qint64 Shred::GlobalY(const int y) const {
    return (Longitude() - CoordOfShred(y))*SHRED_WIDTH + y;
}

void Shred::PhysEventsFrequent() {
    for (auto& i : fallList) {
        if ( i == nullptr ) {
            continue;
        } // else:
        const int x_in = CoordInShred(i->X());
        const int y_in = CoordInShred(i->Y());
        Block* const floor_block = GetBlock(x_in, y_in, i->Z()-1);
        if ( i->Weight() <= 0
                || ( floor_block->PushResult(ANYWHERE) == ENVIRONMENT
                    && floor_block->Sub() != AIR ) )
        {
            i->SetFalling(false);
            i = nullptr;
        } else if (not World::GetWorld()->Move(i->X(), i->Y(), i->Z(), DOWN)) {
            i->FallDamage();
            i = nullptr;
        }
    }
    for (Active* const i : activeListFrequent) {
        if ( i ) i->ActFrequent();
    }
}

void Shred::PhysEventsRare() {
    for (Active* const i : activeListAll) {
        if ( i ) {
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

const std::forward_list<Active*>& Shred::GetShiningList() const {
    return shiningList;
}

Block* Shred::GetBlock(const_int(x, y, z)) const { return blocks[x][y][z]; }

void Shred::PutBlock(Block* const block, const_int(x, y, z)) {
    blocks[x][y][z] = block;
}

void Shred::RegisterInit(Active* const active) {
    active->SetShred(this);
    activeListAll.push_front(active);
    const int should_act = active->ShouldAct();
    if ( Q_UNLIKELY(should_act & FREQUENT_FIRST) ) {
        activeListFrequent.prepend(active);
    } else if ( should_act & FREQUENT_SECOND ) {
        activeListFrequent.append(active);
    }
    AddShining(active);
}

void Shred::Register(Active* const active) {
    RegisterInit(active);
    AddFalling(active);
}

void Shred::Unregister(Active* const active) {
    std::replace(ALL(activeListAll), active, static_cast<Active*>(nullptr));
    std::replace(ALL(activeListFrequent), active,
        static_cast<Active*>(nullptr));
    if ( Falling* const falling = active->ShouldFall() ) {
        std::replace(ALL(fallList), falling, static_cast<Falling*>(nullptr));
        falling->SetFalling(false);
    }
    RemShining(active);
}

void Shred::AddFalling(Block* const block) {
    Falling* const falling = block->ShouldFall();
    if ( falling && not falling->IsFalling() ) {
        falling->SetFalling(true);
        fallList.push_front(falling);
    }
}

void Shred::AddShining(Active* const active) {
    if ( Q_UNLIKELY(active->LightRadius() != 0) ) {
        shiningList.push_front(active);
    }
}

void Shred::RemShining(Active* const active) { shiningList.remove(active); }

void Shred::ReloadTo(const dirs direction) {
    switch ( direction ) {
    default: Q_UNREACHABLE(); break;
    case NORTH: ++shredY; break;
    case EAST:  --shredX; break;
    case SOUTH: --shredY; break;
    case WEST:  ++shredX; break;
    }
}

void Shred::SetBlock(Block* const block, const_int(x, y, z)) {
    Block* const to_delete = GetBlock(x, y, z);
    if ( to_delete != block ) {
        Active* const active = to_delete->ActiveBlock();
        if ( active ) active->Unregister();
        SetBlockNoCheck(block, x, y, z);
    }
}

void Shred::SetBlockNoCheck(Block* const block, const_int(x, y, z)) {
    if ( Active* const active = ( blocks[x][y][z]=block )->ActiveBlock() ) {
        active->SetXyz(x, y, z);
        Register(active);
    }
}

void Shred::SetNewBlock(const kinds kind, const subs sub, const_int(x, y, z)) {
    SetBlock(BlockFactory::NewBlock(kind, sub), x, y, z);
}

QString Shred::FileName(const qint64 longi, const qint64 lati) {
    return Str("%1%2/%3-%4.fm").
        arg(home_path).arg(World::WorldName()).arg(longi).arg(lati);
}

// shred generators section
// these functions fill space between the lowest nullstone layer and sky.
// so use k from 1 to HEIGHT-2.

void Shred::CoverWith(const kinds kind, const subs sub) {
    FOR_ALL_SHRED_AREA(i, j) {
        SetBlock(BlockFactory::NewBlock(kind, sub), i, j,
            FindTopNonAir(i, j) + 1);
    }
}

void Shred::RandomDrop(int num, const kinds kind, const subs sub,
        const bool on_water)
{
    while ( num-- ) {
        DropBlock(BlockFactory::NewBlock(kind, sub), on_water);
    }
}

void Shred::DropBlock(Block* const block, const bool on_water) {
    int y = qrand();
    const int x = CoordInShred(y);
    y = CoordInShred(unsigned(y) >> SHRED_WIDTH_BITSHIFT);
    int z = FindTopNonAir(x, y);
    if( on_water || GetBlock(x, y, z)->Sub()!=WATER ) {
        SetBlock(block, x, y, ++z);
    }
}

void Shred::PlantGrass() {
    FOR_ALL_SHRED_AREA(x, y) {
        const int z = FindTopNonAir(x, y);
        if ( SOIL == GetBlock(x, y, z)->Sub() ) {
            SetBlock(BlockFactory::NewBlock(GRASS, GREENERY), x, y, z + 1);
        }
    }
}

void Shred::TestShred() {
    const int level = NormalUnderground() + 1;
    const KindSub set[SHRED_WIDTH/2][SHRED_WIDTH] = {
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
        if ( set[i][j].kind == 0 && set[i][j].sub == 0 ) continue;
        SetNewBlock(set[i][j].kind, set[i][j].sub, j, i*2, level);
    }
} // void Shred::TestShred()

void Shred::NullMountain() {
    NormalUnderground();
    const int border_level = HEIGHT/2-2;
    NormalCube(0,SHRED_WIDTH/2-1,1, SHRED_WIDTH,2,border_level, NULLSTONE);
    NormalCube(SHRED_WIDTH/2-1,0,1, 2,SHRED_WIDTH,border_level, NULLSTONE);
    Block* const null_stone = BlockFactory::Normal(NULLSTONE);
    FOR_ALL_SHRED_AREA(i, j) {
        for (int k=border_level; k < HEIGHT-2; ++k) {
            const int surface =
                HEIGHT/2 * (pow(1./(i-7.5), 2) * pow(1./(j-7.5), 2)+1);
            if ( HEIGHT/2+1 < surface && surface >= k ) {
                PutBlock(null_stone, i, j, k);
            }
        }
    }
    const AroundShredTypes shred_types(longitude, latitude);
    if ( SHRED_NULLMOUNTAIN == shred_types.To(TO_NORTH) ) {
        NormalCube(7,0,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2, NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == shred_types.To(TO_SOUTH) ) {
        NormalCube(7,SHRED_WIDTH/2+1,HEIGHT/2, 2,SHRED_WIDTH/2-1,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == shred_types.To(TO_EAST) ) {
        NormalCube(SHRED_WIDTH/2+1,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2,
            NULLSTONE);
    }
    if ( SHRED_NULLMOUNTAIN == shred_types.To(TO_WEST) ) {
        NormalCube(0,7,HEIGHT/2, SHRED_WIDTH/2-1,2,HEIGHT/2-2, NULLSTONE);
    }
}

void Shred::Pyramid() {
    const int level = qMin(NormalUnderground(), HEIGHT-1-16);
    Block* const stone = BlockFactory::Normal(STONE);
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
    Block* const air = BlockFactory::Normal(AIR);
    PutBlock(air, SHRED_WIDTH/2, 0, level+1); // entrance
    // room below
    NormalCube(1, 1, HEIGHT/2-60, SHRED_WIDTH-2, SHRED_WIDTH-2, 8, AIR);
    for (int z=HEIGHT/2-52; z<=level; ++z) { // horizontal tunnel
        PutBlock(air, SHRED_WIDTH/2, SHRED_WIDTH/2, z);
    }
    SetNewBlock(CONTAINER, STONE, SHRED_WIDTH-2, SHRED_WIDTH-2, level+1);
    Inventory* const inv =
        GetBlock(SHRED_WIDTH-2,SHRED_WIDTH-2, level+1)->HasInventory();
    inv->Get(BlockFactory::Normal(GOLD));
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
    const AroundShredTypes shred_types(longitude, latitude);
    for (int floors=shred_types.Count(SHRED_CASTLE); ; --floors) {
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
        // exits:
        if ( shred_types.To(TO_NORTH)==SHRED_CASTLE || level==bottom_level ) {
            NormalCube(2,0,level+2, SHRED_WIDTH-4,2,4, AIR);
        }
        if ( shred_types.To(TO_SOUTH)==SHRED_CASTLE || level==bottom_level ) {
            NormalCube(2,SHRED_WIDTH-2,level+2, SHRED_WIDTH-4,2,4, AIR);
        }
        if ( shred_types.To(TO_WEST) ==SHRED_CASTLE || level==bottom_level ) {
            NormalCube(0,2,level+2, 2,SHRED_WIDTH-4,4, AIR);
        }
        if ( shred_types.To(TO_EAST) ==SHRED_CASTLE || level==bottom_level ) {
            NormalCube(SHRED_WIDTH-2,2,level+2, 2,SHRED_WIDTH-4,4, AIR);
        }
        if ( level == bottom_level + 5 ) {
            LoadRoom(level + 2);
        }
        level += 5;
    }
} // Shred::Castle()

void Shred::Layers() {
    static std::vector<KindSub> layers;
    if ( layers.empty() ) {
        QFile file(World::WorldPath() + Str("/layers.txt"));
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            while ( not stream.atEnd() ) {
                QString kind_string, sub_string;
                int repeat;
                stream >> kind_string >> sub_string >> repeat;
                while (repeat--) {
                    layers.push_back(KindSub{
                        static_cast<kinds>(TrManager::StrToKind(kind_string)),
                        static_cast<subs> (TrManager::StrToSub (sub_string))});
                    if ( layers.back().kind == LAST_KIND ) {
                        layers.back().kind = BLOCK;
                    }
                    if ( layers.back().sub == LAST_SUB ) {
                        layers.back().sub = STONE;
                    }
                }
            }
        }
        if ( layers.empty() ) {
            layers.insert(layers.end(), {
                {BLOCK, SOIL },
                {BLOCK, STONE}
            });
        }
        std::reverse(ALL(layers));
    }

    FOR_ALL_SHRED_AREA(x, y) {
        for (uint z = 1, i = 0; z < HEIGHT-1 && i < layers.size(); ++z, ++i) {
            SetNewBlock(layers[i].kind, layers[i].sub, x, y, z);
        }
    }
}

void Shred::ChaosShred() {
    FOR_ALL_SHRED_AREA(x, y) {
        for (int z=1; z<HEIGHT/2; ++z) {
            SetNewBlock(static_cast<kinds>(qrand() % LAST_KIND),
                        static_cast<subs> (qrand() % LAST_SUB ), x, y, z);
        }
    }
}

void Shred::NormalCube(const_int(x_start, y_start, z_start),
                       const_int(x_size,  y_size,  z_size), const subs sub)
{
    Q_ASSERT(InBounds(x_start, y_start, z_start) &&
        InBounds(x_start + x_size-1, y_start + y_size-1, z_start + z_size-1));
    Block* const block = BlockFactory::Normal(sub);
    for (int x=x_start; x < x_start+x_size; ++x)
    for (int y=y_start; y < y_start+y_size; ++y) {
        std::fill(blocks[x][y]+z_start, blocks[x][y]+z_start+z_size, block);
    }
}

bool Shred::Tree(const_int(x, y, z)) {
    int rand = qrand();
    const int height = 4 + (qrand() % 8);
    if ( not InBounds(x+2, y+2, height+z) ) return false;
    // check for room
    const int leaves_level = z+height/2;
    for (int i=x; i<=x+2; ++i)
    for (int j=y; j<=y+2; ++j)
    for (int k=leaves_level; k<z+height; ++k) {
        if ( AIR != GetBlock(i, j, k)->Sub() ) {
            return false;
        }
    }
    if ( GetTypeOfShred() != SHRED_DEAD_FOREST ) {
        NormalCube(x, y, leaves_level, 3, 3, z+height-leaves_level, GREENERY);
    }
    Block* const wood = BlockFactory::Normal(WOOD);
    for (int i=qMax(z-1, 1); i<qMin(HEIGHT-2, z+height-1); ++i) { // trunk
        SetBlock(wood, x+1, y+1, i);
    }
    // branches
    for (const Xyz& xyz : AroundCoordinates4({x+1, y+1, leaves_level})) {
        if ( (rand >>= 1) % 2 ) {
            PutBlock(wood, XYZ(xyz));
        }
    }
    return true;
}

bool Shred::InBounds(const int z) { return Q_LIKELY(0 <= z && z < HEIGHT-1 ); }

bool Shred::InBounds(const int x, const int y) {
    return Q_LIKELY(0 <= x && x < SHRED_WIDTH) && (0 <= y && y < SHRED_WIDTH);
}

bool Shred::InBounds(const_int(x, y, z)) {
    return InBounds(x, y) && InBounds(z);
}

void Shred::Dew(const kinds kind, const subs sub) {
    DropBlock(BlockFactory::NewBlock(kind, sub), true);
}

void Shred::Rain(const kinds kind, const subs sub) {
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
        SetBlock(BlockFactory::NewBlock(kind, sub), x, y, CLOUD_HEIGHT);
    }
}

bool Shred::LoadRoom(const int level, const int index) {
    QFile file(Str("%1%2.room").
            arg(FileName(longitude, latitude)).
            arg((index >= 1 ) ?
                Str("-%1").arg(index) : Str("")));
    if ( not file.open(QIODevice::ReadOnly | QIODevice::Text) ) return false;
    for (int lines = 0; lines < SHRED_WIDTH; ++lines) {
        char buffer[SHRED_WIDTH + 1]{0};
        file.readLine(buffer, sizeof(buffer));
        for (unsigned i=0; i<SHRED_WIDTH; ++i) {
            switch ( buffer[i] ) {
            case '#':
                PutBlock(BlockFactory::Normal(STONE), i, lines, level);
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
                Block* const stone = BlockFactory::Normal(STONE);
                PutBlock(stone, i, lines, level);
                PutBlock(stone, i, lines, level+1); // level+2 is missing
                PutBlock(stone, i, lines, level+3); // because it is window.
                PutBlock(stone, i, lines, level+4);
                } break;
            case '=': // floor and ceiling
                PutBlock(BlockFactory::Normal(WOOD),  i, lines, level);
                PutBlock(BlockFactory::Normal(STONE), i, lines, level+4);
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

// AroundShredTypes:: section

AroundShredTypes::AroundShredTypes(const qint64 longitude, qint64 latitude) :
        types()
{
    int position = 0;
    for (qint64 i=longitude-1; i<=longitude+1; ++i)
    for (qint64 j=latitude -1; j<=latitude +1; ++j) {
        types[position++] = World::GetConstWorld()->GetMap()->TypeOfShred(i,j);
    }
}

char AroundShredTypes::To(const to_dirs dir) const { return types[dir]; }

int AroundShredTypes::Count(const char type) const {
    return std::count(ALL(types), type);
}
