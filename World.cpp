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
#include "Shred.h"
#include "BlockFactory.h"
#include "ShredStorage.h"
#include "worldmap.h"
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include <QDir>
#include <QMutexLocker>
#include <QTimer>
#include <QSettings>

World * World::GetWorld() { return world; }

bool World::ShredInCentralZone(const qint64 longi, const qint64  lati) const {
    return ( qAbs(longi - longitude) <= 1 ) && ( qAbs(lati - latitude) <= 1 );
}

int World::ShredPos(const int x, const int y) const { return y*NumShreds()+x; }

Shred * World::GetShred(const int x, const int y) const {
    return shreds[ShredPos(Shred::CoordOfShred(x), Shred::CoordOfShred(y))];
}

Shred * World::GetShredByPos(const int x, const int y) const {
    return shreds[ShredPos(x, y)];
}

Shred * World::GetNearShred(Shred * const shred, const dirs dir) const {
    switch ( dir ) {
    default: Q_UNREACHABLE(); break;
    case NORTH: return GetShredByPos(shred->ShredX(),   shred->ShredY()-1);
    case EAST:  return GetShredByPos(shred->ShredX()+1, shred->ShredY());
    case SOUTH: return GetShredByPos(shred->ShredX(),   shred->ShredY()+1);
    case WEST:  return GetShredByPos(shred->ShredX()-1, shred->ShredY());
    }
}

int World::TimeOfDay() const { return time % SECONDS_IN_DAY; }
int World::MiniTime() const { return timeStep; }
quint64 World::Time() const { return time; }
qint64 World::Longitude() const { return longitude; }
qint64 World::Latitude()  const { return latitude; }
const WorldMap * World::GetMap() const { return map; }

QByteArray * World::GetShredData(qint64 longi, qint64 lati) const {
    return shredStorage->GetShredData(longi, lati);
}

void World::SetShredData(QByteArray * const data,
        const qint64 longi, const qint64 lati)
{
    shredStorage->SetShredData(data, longi, lati);
}

times_of_day World::PartOfDay() const {
    return static_cast<times_of_day>(TimeOfDay() / SECONDS_IN_NIGHT);
}

QString World::TimeOfDayStr() const {
    return tr("Time is %1:%2.").
        arg(TimeOfDay()/60).
        arg(TimeOfDay()%60, 2, 10, QChar::fromLatin1('0'));
}

bool World::Drop(Block* const block_from, const_int(x_to, y_to, z_to),
        const_int(src, dest, num))
{
    Block* const block_to = BlockFactory::NewBlock(BOX, DIFFERENT);
    if ( not Build(block_to, x_to, y_to, z_to) ) {
        delete block_to;
    }
    return Exchange(block_from, GetBlock(x_to, y_to, z_to), src, dest, num);
}

bool World::Get(Block* const block_to, const_int(x_from, y_from, z_from),
        const_int(src, dest, num))
{
    Block     * const block_from = GetBlock(x_from, y_from, z_from);
    Inventory * const inv_from   = block_from->HasInventory();
    if ( inv_from ) {
        if ( inv_from->Access() ) {
            return Exchange(block_from, block_to, src, dest, num);
        }
    } else if ( Exchange(block_from, block_to, src, dest, num) ) {
        Block* const air = BlockFactory::Normal(AIR);
        ReEnlightenCheck(air, block_from, x_from, y_from, z_from);
        GetShred(x_from, y_from)->PutBlock(air,
            Shred::CoordInShred(x_from), Shred::CoordInShred(y_from), z_from);
        return true;
    }
    return false;
}

bool World::InBounds(const int x, const int y) const {
    return ( (0 <= x && x <= GetBound()) && (0 <= y && y <= GetBound()) );
}

bool World::InBounds(const_int(x, y, z)) const {
    return ( InBounds(x, y) && Shred::InBounds(z) );
}

int World::GetBound() {
    static const int bound = World::GetWorld()->NumShreds() * SHRED_WIDTH - 1;
    return bound;
}

int World::SetNote(const QString note) {
    notes.append(note);
    return notes.size();
}

int World::ChangeNote(const QString note, const int noteId) {
    if ( noteId > notes.size() ) {
        return SetNote(note);
    } else {
        notes[noteId - 1] = note;
        return noteId;
    }
}

QString World::GetNote(const int noteId) const { return notes.at(noteId-1); }

void World::ReloadAllShreds(const QString new_world,
        const qint64 lati, const qint64 longi,
        const int new_x, const int new_y, const int new_z)
{
    newLati  = lati;
    newLongi = longi;
    newX = new_x;
    newY = new_y;
    newZ = new_z;
    newWorld = new_world;
}

void World::SaveToDisk() const {
    for (Shred ** s = shreds + NumShreds()*NumShreds(); --s >= shreds; ) {
        (*s)->SaveShred(false);
    }
    shredStorage->WriteToFileAllShredData();
    SaveState();
}

void World::ActivateFullReload() { toResetDir = DOWN; }

dirs World::TurnRight(const dirs dir) {
    return static_cast<dirs>(((dir - 2 + 1) & 3) + 2);
}

dirs World::TurnLeft(const dirs dir) {
    return static_cast<dirs>(((dir + 4 - 2 - 1) & 3) + 2);
}

dirs World::Anti(const dirs dir) {
    return static_cast<dirs>( ( dir <= DOWN ) ?
        not dir :
        (dir & 3) + 2 );
}

Block* World::GetBlock(const_int(x, y, z)) const {
    return GetShred(x, y)->
        GetBlock(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

Shred ** World::FindShred(const int x, const int y) const {
    return &shreds[ShredPos(x, y)];
}

void World::ReloadShreds() {
    switch ( toResetDir ) {
    case UP: return; // no reset
    case DOWN:       // full reset
        // clean old
        emit StartReloadAll();
        qDeleteAll(shreds, shreds + NumShreds()*NumShreds());
        delete map;
        delete shredStorage;
        SaveNotes();
        SaveState();
        // load new
        map = new WorldMap((worldName = newWorld));
        LoadState();
        longitude = newLongi;
        latitude  = newLati;
        shredStorage = new ShredStorage(numShreds+2, longitude, latitude);
        LoadNotes();
        LoadAllShreds();
        emit NeedPlayer(newX, newY, newZ);
        emit UpdatedAll();
        emit FinishReloadAll();
        toResetDir = UP; // set no reset
        return;
    // while reloading, reuse memory places.
    case NORTH:
        emit StartMove(NORTH);
        --longitude;
        for (int x=0; x<NumShreds(); ++x) {
            Shred * const shred = *FindShred(x, NumShreds()-1);
            shred->~Shred();
            for (int y=NumShreds()-1; y>0; --y) {
                ( *FindShred(x, y) = *FindShred(x, y-1) )->ReloadTo(NORTH);
            }
            *FindShred(x, 0) = new(shred) Shred(x, 0,
                longitude - NumShreds()/2,
                latitude  - NumShreds()/2 + x);
        }
        break;
    case EAST:
        emit StartMove(EAST);
        ++latitude;
        for (int y=0; y<NumShreds(); ++y) {
            Shred * const shred = *FindShred(0, y);
            shred->~Shred();
            for (int x=0; x<NumShreds()-1; ++x) {
                ( *FindShred(x, y) = *FindShred(x+1, y) )->ReloadTo(EAST);
            }
            *FindShred(NumShreds()-1, y) = new(shred) Shred(NumShreds()-1, y,
                longitude - NumShreds()/2 + y,
                latitude  + NumShreds()/2);
        }
        break;
    case SOUTH:
        emit StartMove(SOUTH);
        ++longitude;
        for (int x=0; x<NumShreds(); ++x) {
            Shred * const shred = *FindShred(x, 0);
            shred->~Shred();
            for (int y=0; y<NumShreds()-1; ++y) {
                ( *FindShred(x, y) = *FindShred(x, y+1) )->ReloadTo(SOUTH);
            }
            *FindShred(x, NumShreds()-1) = new(shred) Shred(x, NumShreds()-1,
                longitude + NumShreds()/2,
                latitude  - NumShreds()/2 + x);
        }
        break;
    case WEST:
        emit StartMove(WEST);
        --latitude;
        for (int y=0; y<NumShreds(); ++y) {
            Shred * const shred = *FindShred(NumShreds()-1, y);
            for (int x=NumShreds()-1; x>0; --x) {
                ( *FindShred(x, y) = *FindShred(x-1, y) )->ReloadTo(WEST);
            }
            *FindShred(0, y) = new(shred) Shred(0, y,
                longitude - NumShreds()/2 + y,
                latitude  - NumShreds()/2);
        }
        break;
    }
    shredStorage->Shift(toResetDir, longitude, latitude);
    ReEnlightenMove(toResetDir);
    emit Moved(toResetDir);
    toResetDir = UP; // set no reset
} // void World::ReloadShreds()

void World::SetReloadShreds(const int direction) {
    toResetDir = static_cast<dirs>(direction);
}

void World::run() {
    timer = new QTimer;
    timer->setInterval(1000 / TIME_STEPS_IN_SEC);
    connect(timer, &QTimer::timeout, this, &World::PhysEvents);
    connect(this, &World::Pause, timer, &QTimer::stop);
    connect(this, &World::Start, timer,
        static_cast<void (QTimer::*)()>(&QTimer::start));

    timer->start();
    exec();
    delete timer;
}

void World::PhysEvents() {
    static const int start = NumShreds()/2 - numActiveShreds/2;
    static const int end   = start + numActiveShreds;
    GetLock()->lock();
    for (int i=start; i<end; ++i)
    for (int j=start; j<end; ++j) {
        shreds[ShredPos(i, j)]->PhysEventsFrequent();
    }
    if ( TIME_STEPS_IN_SEC > timeStep ) {
        ++timeStep;
    } else {
        for (int i=start; i<end; ++i)
        for (int j=start; j<end; ++j) {
            shreds[ShredPos(i, j)]->PhysEventsRare();
        }
        timeStep = 0;
        ++time;
    }
    ReEnlighten();
    ReloadShreds();
    GetLock()->unlock();
    emit UpdatesEnded();
}

bool World::DirectlyVisible(
        int x_from, int y_from, int z_from,
        int x,      int y,      int z)
const {
    /// optimized DDA line with integers only.
    unsigned max = Abs(Abs(x-=x_from) > Abs(y-=y_from) ? x : y);
    if ( Abs(z-=z_from) > max) {
        max = Abs(z);
    }
    x_from *= max;
    y_from *= max;
    z_from *= max;
    for (int i=max; --i > 0; ) { // unsigned / is faster than signed
        if ( not (GetBlock( // floor
                    static_cast<unsigned>(x_from+=x)/max,
                    static_cast<unsigned>(y_from+=y)/max,
                    static_cast<unsigned>(z_from+=z)/max )->Transparent()
                && GetBlock( // ceil
                    // force signed / to prevent anti-overflow
                    (x_from-1)/static_cast<int>(max) + 1,
                    (y_from-1)/static_cast<int>(max) + 1,
                    static_cast<unsigned>(z_from-1)/max + 1)->Transparent()) )
        {
            return false;
        }
    }
    return true;
}

bool World::Visible(
        const_int(x_from, y_from, z_from),
        const_int(x_to,   y_to,   z_to) )
const {
    int temp;
    return ( DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)
        || (x_to != x_from &&
            GetBlock(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to)->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to))
        || (y_to != y_from &&
            GetBlock(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to)->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to))
        || (z_to != z_from &&
            GetBlock(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1))->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp))
        );
}

bool World::Move(const_int(x, y, z), const dirs dir) {
    int newx, newy, newz;
    if ( Focus(x, y, z, &newx, &newy, &newz, dir) ) {
        switch ( CanMove(x, y, z, newx, newy, newz, dir) ) {
        case CAN_MOVE_OK:
            NoCheckMove(x, y, z, newx, newy, newz, dir);
            return true;
        case CAN_MOVE_CANNOT:    return false;
        case CAN_MOVE_DESTROYED: return true;
        }
        return true;
    } else {
        return false;
    }
}

World::can_move_results World::CanMove(
        const_int(x, y, z),
        const_int(newx, newy, newz), const dirs dir)
{
    Block* const block    = GetBlock(x, y, z);
    Block*       block_to = GetBlock(newx, newy, newz);
    Falling * const falling = block->ShouldFall();
    if ( DOWN != dir
            && block->Weight() != 0
            && falling != nullptr
            && falling->IsFalling()
            && AIR == GetBlock(x, y, z-1)->Sub()
            && AIR == GetBlock(newx, newy, newz-1)->Sub() )
    { // prevent moving while falling
        return CAN_MOVE_CANNOT;
    }

    switch ( block->PushResult(dir) ) {
    case DAMAGE:
    case MOVABLE:
        if ( Damage(newx, newy, newz, 1, DAMAGE_PUSH_UP + dir) <= 0 ) {
            DestroyAndReplace(newx, newy, newz);
        }
        block_to = GetBlock(newx, newy, newz);
        break;
    case ENVIRONMENT:
        if ( not (*block == *block_to) ) { // prevent useless flow
            break;
        } // no break;
    default: return CAN_MOVE_CANNOT;
    }

    switch ( block_to->PushResult(dir) ) {
    case MOVABLE:
        return ( block->Weight() > block_to->Weight()
            && Move(newx, newy, newz, dir) ) ?
                CAN_MOVE_OK : CAN_MOVE_CANNOT;
    case ENVIRONMENT: return CAN_MOVE_OK;
    case NOT_MOVABLE: break;
    case MOVE_UP:
        if ( dir > DOWN ) { // not UP and not DOWN
            Move(x, y, z, UP);
        }
        break;
    case JUMP:
        if ( dir > DOWN ) { // not UP and not DOWN
            Jump(x, y, z, dir);
        }
        break;
    case DAMAGE:
        if ( Damage(x, y, z, block_to->DamageLevel(), block_to->DamageKind())
                <= 0 )
        {
            DestroyAndReplace(x, y, z);
            return CAN_MOVE_DESTROYED;
        }
        break;
    }
    return CAN_MOVE_CANNOT;
} // bool World::CanMove(const int x, y, z, newx, newy, newz, int dir)

void World::NoCheckMove(const_int(x, y, z),
                        const_int(newx, newy, newz), const dirs dir)
{
    Shred * const shred_from = GetShred(   x,    y);
    Shred * const shred_to   = GetShred(newx, newy);
    const int    x_in = Shred::CoordInShred(x);
    const int    y_in = Shred::CoordInShred(y);
    const int newx_in = Shred::CoordInShred(newx);
    const int newy_in = Shred::CoordInShred(newy);
    Block* const block    = shred_from->GetBlock(   x_in,    y_in,    z);
    Block* const block_to = shred_to  ->GetBlock(newx_in, newy_in, newz);

    ReEnlightenCheck(block, block_to, x, y, z);

    shred_from->PutBlock(block_to,    x_in,    y_in,    z);
    shred_to  ->PutBlock(block,    newx_in, newy_in, newz);

    block->Move(dir);

    shred_from->AddFalling(shred_from->GetBlock(   x_in,    y_in,    z+1));
    shred_to  ->AddFalling(shred_to  ->GetBlock(newx_in, newy_in, newz+1));
    shred_to  ->AddFalling(block);

    if ( block_to->Sub() != AIR ) {
        block_to->Move(Anti(dir));
        shred_from->AddFalling(block_to);
    }
}

void World::Jump(const_int(x, y, z), const dirs dir) {
    if ( not (AIR == GetBlock(x, y, z-1)->Sub() && GetBlock(x, y, z)->Weight())
            && Move(x, y, z, UP) )
    {
        Move(x, y, z+1, dir);
    }
}

bool World::Focus(const_int(x, y, z),
        int * x_to, int * y_to, int * z_to, const dirs dir)
const {
    *x_to = x;
    *y_to = y;
    *z_to = z;
    switch ( dir ) {
    case UP:    ++*z_to; break;
    case DOWN:  --*z_to; break;
    case NORTH: --*y_to; break;
    case EAST:  ++*x_to; break;
    case SOUTH: ++*y_to; break;
    case WEST:  --*x_to; break;
    }
    return InBounds(*x_to, *y_to, *z_to);
}

int World::Damage(int x, int y, const int z, const int dmg, const int dmg_kind)
{
    Shred * const shred = GetShred(x, y);
    x = Shred::CoordInShred(x);
    y = Shred::CoordInShred(y);
    Block* temp = shred->GetBlock(x, y, z);
    if ( AIR == temp->Sub() ) return Block::MAX_DURABILITY;
    if ( temp == BlockFactory::Normal(temp->Sub()) ) {
        temp = BlockFactory::NewBlock(temp->Kind(), temp->Sub());
        shred->SetBlockNoCheck(temp, x, y, z);
    }
    temp->Damage(dmg, dmg_kind);
    return temp->GetDurability();
}

void World::DestroyAndReplace(const_int(x, y, z)) {
    Shred * const  shred = GetShred(x, y);
    const int x_in_shred = Shred::CoordInShred(x);
    const int y_in_shred = Shred::CoordInShred(y);
    Block* const block  = shred->GetBlock(x_in_shred, y_in_shred, z);
    bool delete_block = true;
    Block* const new_block = block->DropAfterDamage(&delete_block);

    ReEnlightenCheck(block, new_block, x, y, z);

    shred->SetBlockNoCheck(new_block, x_in_shred, y_in_shred, z);

    if ( delete_block ) {
        BlockFactory::DeleteBlock(block);
    } else {
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
    }
    shred->AddFalling(shred->GetBlock(x_in_shred, y_in_shred, z+1));
}

bool World::Build(Block* const block, const_int(x, y, z), Block* const who) {
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    Block* const target_block = shred->GetBlock(x_in, y_in, z);
    if ( ENVIRONMENT!=target_block->PushResult(ANYWHERE) ) {
        if ( who != nullptr ) {
            who->ReceiveSignal(tr("Cannot build here."));
        }
        return false;
    } // else:
    block->Restore();
    ReEnlightenCheck(block, target_block, x, y, z);
    shred->SetBlock(block, x_in, y_in, z);
    shred->AddFalling(shred->GetBlock(x_in, y_in, z+1));
    return true;
}

void World::ReEnlightenCheck(Block* const block1, Block* const block2,
        const_int(x, y, z))
{
    if (
            block1->Transparent() != block2->Transparent() ||
            block1->LightRadius() != block2->LightRadius() )
    {
        UnShine(x, y, z);
    } else {
        emit UpdatedAround(x, y, z);
    }
}

bool World::Inscribe(const_int(x, y, z)) {
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    Block* block  = shred->GetBlock(x_in, y_in, z);
    if ( block == BlockFactory::Normal(block->Sub()) ) {
        block = BlockFactory::NewBlock(block->Kind(), block->Sub());
        shred->SetBlockNoCheck(block, x_in, y_in, z);
    }
    QString str;
    emit GetString(str);
    return block->Inscribe(str);
}

bool World::Exchange(Block* const block_from, Block* const block_to,
        const_int(src, dest, num))
{
    Inventory * const inv_to = block_to->HasInventory();
    if ( not inv_to ) {
        block_from->ReceiveSignal(tr("No room there."));
        return false;
    }
    Inventory * const inv_from = block_from->HasInventory();
    if ( not inv_from ) {
        if ( block_from->Wearable() > WEARABLE_NOWHERE
                && inv_to->Get(block_from, src) )
        {
            return true;
        } else {
            block_to->ReceiveSignal(tr("%1 cannot be obtained.").
                arg(block_from->FullName()));
            return false;
        }
    }
    if ( src >= inv_from->Size() || inv_from->IsEmpty(src) ) {
        block_from->ReceiveSignal(tr("Nothing here."));
        block_to  ->ReceiveSignal(tr("Nothing in %1 at slot '%2'.").
            arg(block_from->FullName()).
            arg(char(src + 'a')));
    } else {
        block_from->Damage(1, DAMAGE_INVENTORY_ACTION);
        block_to  ->Damage(1, DAMAGE_INVENTORY_ACTION);
        inv_from->Drop(src, dest, num, inv_to);
    }
    return false;
}

void World::LoadAllShreds() {
    for (qint64 j=longitude-NumShreds()/2, y=0; y<NumShreds(); ++j, ++y)
    for (qint64 i=latitude -NumShreds()/2, x=0; x<NumShreds(); ++i, ++x) {
        *FindShred(x, y) = new Shred(x, y, j, i);
    }
    ReEnlightenAll();
}

int World::CorrectNumShreds(int num) {
    num += ( (num & 1) == 0 ); // make odd
    return qMax(num, MIN_WORLD_SIZE);
}

int World::CorrectNumActiveShreds(int num, const int num_shreds) {
    num += ( (num & 1) == 0 ); // make odd
    return qBound(3, num, num_shreds);
}

World * World::world = nullptr;

World::World(const QString world_name, bool * const error) :
        worldName(world_name),
        map(new WorldMap(world_name)),
        time(), timeStep(),
        shreds(),
        longitude(), latitude(),
        gameSettings(new QSettings(home_path + Str("freg.ini"),
            QSettings::IniFormat)),
        numShreds(CorrectNumShreds(gameSettings->value(
            Str("number_of_shreds"), MIN_WORLD_SIZE + 1).toInt())),
        numActiveShreds(CorrectNumActiveShreds(gameSettings->value(
            Str("number_of_active_shreds"), numShreds).toInt(), numShreds)),
        mutex(),
        newLati(), newLongi(),
        newX(), newY(), newZ(),
        newWorld(),
        toResetDir(UP),
        timer(nullptr),
        shredStorage(),
        notes(),
        tempShiningList()
{
    Q_ASSERT(world == nullptr); // world is a singleton.
    world = this;

    LoadState();
    gameSettings->setValue(Str("number_of_shreds"), numShreds);
    gameSettings->setValue(Str("number_of_active_shreds"), numActiveShreds);
    delete gameSettings;

    shreds = new Shred *[NumShreds()*NumShreds()];
    if ( not QDir(home_path).mkpath(worldName + Str("/texts")) ) {
        *error = true;
    }

    shredStorage = new ShredStorage(numShreds+2, longitude, latitude);

    LoadNotes();
    LoadAllShreds();
    emit UpdatedAll();
}

World::~World() {
    GetLock()->lock();
    quit();
    wait();
    GetLock()->unlock();

    qDeleteAll(shreds, shreds + NumShreds()*NumShreds());
    delete [] shreds;
    delete shredStorage;
    delete map;

    SaveState();
    QSettings(home_path + Str("freg.ini"), QSettings::IniFormat).
        setValue(Str("current_world"), worldName);

    SaveNotes();
}

void World::SaveState() const {
    QSettings settings(WorldPath()+Str("/settings.ini"), QSettings::IniFormat);
    settings.setValue(Str("time"), time);
    settings.setValue(Str("time_step"), timeStep);
    settings.setValue(Str("longitude"), longitude);
    settings.setValue(Str("latitude"),  latitude );
}

void World::LoadNotes() {
    notes.clear();
    QFile notes_file(WorldPath() + Str("/notes.txt"));
    if ( not notes_file.open(QIODevice::ReadOnly | QIODevice::Text) ) return;
    char note[MAX_NOTE_LENGTH*2];
    while ( notes_file.readLine(note, MAX_NOTE_LENGTH*2) > 0 ) {
        notes.append(QString::fromUtf8(note).trimmed());
    }
}

void World::SaveNotes() const {
    QFile notes_file(WorldPath() + Str("/notes.txt"));
    if ( not notes_file.open(QIODevice::WriteOnly | QIODevice::Text) ) return;
    for (const QString & note : notes) {
        notes_file.write(note.toUtf8().constData());
        notes_file.putChar('\n');
    }
}

void World::LoadState() {
    const QSettings setting(WorldPath() + Str("/settings.ini"),
        QSettings::IniFormat);
    time = setting.value(Str("time"), END_OF_NIGHT).toULongLong();
    timeStep  = setting.value(Str("time_step"), 0).toInt();
    longitude = setting.value(Str("longitude"),
        map->GetSpawnLongitude()).toLongLong();
    latitude  = setting.value(Str("latitude"),
        map->GetSpawnLatitude()).toLongLong();
}

unsigned World::Abs(const int x) {
    const unsigned mask = x >> (sizeof(unsigned)*8 - 1);
    return (x ^ mask) - mask;
}
