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

#include "World.h"
#include "Shred.h"
#include "BlockManager.h"
#include "ShredStorage.h"
#include "worldmap.h"
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include <QDir>
#include <QMutexLocker>
#include <QTimer>
#include <QSettings>

const int MIN_WORLD_SIZE = 5;

World * world;

bool World::ShredInCentralZone(const long longi, const long  lati) const {
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
    case NORTH: return GetShredByPos(shred->ShredX(),   shred->ShredY()-1);
    case SOUTH: return GetShredByPos(shred->ShredX(),   shred->ShredY()+1);
    case EAST:  return GetShredByPos(shred->ShredX()+1, shred->ShredY());
    case WEST:  return GetShredByPos(shred->ShredX()-1, shred->ShredY());
    default: Q_UNREACHABLE(); break;
    }
}

int World::NumShreds() const { return numShreds; }
int World::TimeOfDay() const { return time % SECONDS_IN_DAY; }
int World::MiniTime() const { return timeStep; }
quint64 World::Time() const { return time; }
qint64 World::Longitude() const { return longitude; }
qint64 World::Latitude()  const { return latitude; }
bool World::GetEvernight() const { return evernight; }
QString World::WorldName() const { return worldName; }
const WorldMap * World::GetMap() const { return map; }

QByteArray * World::GetShredData(long longi, long lati) const {
    return shredStorage->GetShredData(longi, lati);
}

void World::SetShredData(QByteArray * const data,
        const long longi, const long lati)
{
    shredStorage->SetShredData(data, longi, lati);
}

times_of_day World::PartOfDay() const {
    return static_cast<times_of_day>(TimeOfDay() / SECONDS_IN_NIGHT);
}

QString World::TimeOfDayStr() const {
    return tr("Time is %1:%2.").
        arg(TimeOfDay()/60).
        arg(TimeOfDay()%60, 2, 10, QChar('0'));
}

bool World::Drop(Block * const block_from,
        const int x_to, const int y_to, const int z_to,
        const int src, const int dest, const int num)
{
    Block * const block_to = BlockManager::NewBlock(BOX, DIFFERENT);
    if ( not Build(block_to, x_to, y_to, z_to) ) {
        delete block_to;
    }
    return Exchange(block_from, GetBlock(x_to, y_to, z_to), src, dest, num);
}

bool World::Get(Block * const block_to,
        const int x_from, const int y_from, const int z_from,
        const int src, const int dest, const int num)
{
    Block     * const block_from = GetBlock(x_from, y_from, z_from);
    Inventory * const inv_from   = block_from->HasInventory();
    if ( inv_from ) {
        if ( inv_from->Access() ) {
            return Exchange(block_from, block_to, src, dest, num);
        }
    } else if ( Exchange(block_from, block_to, src, dest, num) ) {
        Build(block_manager->Normal(AIR), x_from, y_from, z_from, UP,
            nullptr, true);
        return true;
    }
    return false;
}

bool World::InBounds(const int x, const int y) const {
    static const int max_xy = GetBound();
    return ( (0 <= x && x <= max_xy) && (0 <= y && y <= max_xy) );
}

bool World::InBounds(const int x, const int y, const int z) const {
    return ( InBounds(x, y) && InVertBounds(z) );
}

int World::GetBound() const {
    static const int bound = NumShreds() * SHRED_WIDTH - 1;
    return bound;
}

bool World::InVertBounds(const int z) { return ( 0 <= z && z < HEIGHT ); }

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

void World::ActivateFullReload() { toResetDir = DOWN; }

QMutex * World::GetLock() { return &mutex; }
void World::Lock() { mutex.lock(); }
bool World::TryLock() { return mutex.tryLock(); }
void World::Unlock() { mutex.unlock(); }

dirs World::TurnRight(const dirs dir) {
    switch ( dir ) {
    case UP:
    case DOWN:  return dir;
    case NORTH: return EAST;
    case SOUTH: return WEST;
    case EAST:  return SOUTH;
    case WEST:  return NORTH;
    }
    Q_UNREACHABLE();
    return NORTH;
}

dirs World::TurnLeft(const dirs dir) {
    switch ( dir ) {
    case UP:
    case DOWN:  return dir;
    case NORTH: return WEST;
    case SOUTH: return EAST;
    case EAST:  return NORTH;
    case WEST:  return SOUTH;
    }
    Q_UNREACHABLE();
    return NORTH;
}

dirs World::Anti(const dirs dir) {
    switch ( dir ) {
    case UP:    return DOWN;
    case DOWN:  return UP;
    case NORTH: return SOUTH;
    case SOUTH: return NORTH;
    case EAST:  return WEST;
    case WEST:  return EAST;
    }
    Q_UNREACHABLE();
    return NORTH;
}

Block * World::GetBlock(const int x, const int y, const int z) const {
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
        DeleteAllShreds();
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
            delete *FindShred(x, NumShreds()-1);
            for (int y=NumShreds()-1; y>0; --y) {
                ( *FindShred(x, y) = *FindShred(x, y-1) )->ReloadTo(NORTH);
            }
            *FindShred(x, 0) = new Shred(x, 0,
                longitude - NumShreds()/2,
                latitude  - NumShreds()/2+x);
        }
        break;
    case SOUTH:
        emit StartMove(SOUTH);
        ++longitude;
        for (int x=0; x<NumShreds(); ++x) {
            delete *FindShred(x, 0);
            for (int y=0; y<NumShreds()-1; ++y) {
                ( *FindShred(x, y) = *FindShred(x, y+1) )->ReloadTo(SOUTH);
            }
            *FindShred(x, NumShreds()-1) = new Shred(x, NumShreds()-1,
                longitude + NumShreds()/2,
                latitude  - NumShreds()/2+x);
        }
        break;
    case EAST:
        emit StartMove(EAST);
        ++latitude;
        for (int y=0; y<NumShreds(); ++y) {
            delete *FindShred(0, y);
            for (int x=0; x<NumShreds()-1; ++x) {
                ( *FindShred(x, y) = *FindShred(x+1, y) )->ReloadTo(EAST);
            }
            *FindShred(NumShreds()-1, y) = new Shred(NumShreds()-1, y,
                longitude - NumShreds()/2+y,
                latitude  + NumShreds()/2);
        }
        break;
    case WEST:
        emit StartMove(WEST);
        --latitude;
        for (int y=0; y<NumShreds(); ++y) {
            delete *FindShred(NumShreds()-1, y);
            for (int x=NumShreds()-1; x>0; --x) {
                ( *FindShred(x, y) = *FindShred(x-1, y) )->ReloadTo(WEST);
            }
            *FindShred(0, y) = new Shred(0, y,
                longitude - NumShreds()/2+y,
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
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), SLOT(PhysEvents()));
    timer.start(1000 / TIME_STEPS_IN_SEC);
    exec();
}

void World::PhysEvents() {
    static const int start = NumShreds()/2 - numActiveShreds/2;
    static const int end   = start + numActiveShreds;
    Lock();
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
        switch ( TimeOfDay() ) {
        default: break;
        case END_OF_NIGHT:
        case END_OF_EVENING: ReEnlightenTime(); break;
        }
    }
    ReloadShreds();
    Unlock();
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
    for (int i=max-1; i-- > 0; ) {
        if ( not (GetBlock(
                    static_cast<unsigned>(x_from+=x)/max,
                    static_cast<unsigned>(y_from+=y)/max,
                    static_cast<unsigned>(z_from+=z)/max )->Transparent()
                && GetBlock(
                    static_cast<unsigned>(x_from+max-1)/max,
                    static_cast<unsigned>(y_from+max-1)/max,
                    static_cast<unsigned>(z_from+max-1)/max )->Transparent()) )
        {
                return false;
        }
    }
    return true;
}

bool World::Visible(
        const int x_from, const int y_from, const int z_from,
        const int x_to,   const int y_to,   const int z_to)
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

bool World::Move(const int x, const int y, const int z, const dirs dir) {
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

can_move_results World::CanMove(const int x, const int y, const int z,
        const int newx, const int newy, const int newz, const dirs dir)
{
    Block * const block    = GetBlock(x, y, z);
    Block *       block_to = GetBlock(newx, newy, newz);
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
        if ( *block != *block_to ) { // prevent useless flow
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

void World::NoCheckMove(const int x, const int y, const int z,
        const int newx, const int newy, const int newz, const dirs dir)
{
    Shred * const shred_from = GetShred(   x,    y);
    Shred * const shred_to   = GetShred(newx, newy);
    const int    x_in = Shred::CoordInShred(x);
    const int    y_in = Shred::CoordInShred(y);
    const int newx_in = Shred::CoordInShred(newx);
    const int newy_in = Shred::CoordInShred(newy);
    Block * const block    = shred_from->GetBlock(   x_in,    y_in,    z);
    Block * const block_to = shred_to  ->GetBlock(newx_in, newy_in, newz);

    shred_from->PutBlock(block_to,    x_in,    y_in,    z);
    shred_to  ->PutBlock(block,    newx_in, newy_in, newz);

    if ( block_to->Transparent() != block->Transparent() ) {
        ReEnlighten(newx, newy, newz);
        ReEnlighten(x, y, z);
    }

    block->Move(dir);
    emit Updated(newx, newy, newz);

    shred_from->AddFalling(shred_from->GetBlock(   x_in,    y_in,    z+1));
    shred_to  ->AddFalling(shred_to  ->GetBlock(newx_in, newy_in, newz+1));
    shred_to  ->AddFalling(block);

    if ( block_to->Sub() != AIR ) {
        emit Updated(x, y, z);
        block_to->Move(Anti(dir));
        shred_from->AddFalling(block_to);
    }
}

void World::Jump(const int x, const int y, const int z, const dirs dir) {
    if ( not (AIR == GetBlock(x, y, z-1)->Sub() && GetBlock(x, y, z)->Weight())
            && Move(x, y, z, UP) )
    {
        Move(x, y, z+1, dir);
    }
}

bool World::Focus(const int x, const int y, const int z,
        int * x_to, int * y_to, int * z_to, const dirs dir)
const {
    *x_to = x;
    *y_to = y;
    *z_to = z;
    switch ( dir ) {
    case UP:    ++*z_to; break;
    case DOWN:  --*z_to; break;
    case NORTH: --*y_to; break;
    case SOUTH: ++*y_to; break;
    case EAST:  ++*x_to; break;
    case WEST:  --*x_to; break;
    }
    return InBounds(*x_to, *y_to, *z_to);
}

int World::Damage(const int x, const int y, const int z,
        const int dmg, const int dmg_kind)
{
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    Block * temp = shred->GetBlock(x_in, y_in, z);
    const int sub  = temp->Sub();
    if ( AIR == sub ) return 0;
    if ( temp == block_manager->Normal(sub) ) {
        temp = BlockManager::NewBlock(temp->Kind(), sub);
        shred->SetBlockNoCheck(temp, x_in, y_in, z);
    }
    temp->Damage(dmg, dmg_kind);
    return temp->GetDurability();
}

void World::DestroyAndReplace(const int x, const int y, const int z) {
    Shred * const  shred = GetShred(x, y);
    const int x_in_shred = Shred::CoordInShred(x);
    const int y_in_shred = Shred::CoordInShred(y);
    Block * const block  = shred->GetBlock(x_in_shred, y_in_shred, z);
    bool delete_block = true;
    Block * const new_block = block->DropAfterDamage(&delete_block);
    shred->SetBlockNoCheck(new_block, x_in_shred, y_in_shred, z);
    if (    block->Transparent() != new_block->Transparent() ||
            block->LightRadius() != new_block->LightRadius() )
    {
        ReEnlighten(x, y, z);
    }
    if ( delete_block ) {
        block_manager->DeleteBlock(block);
    } else {
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
    }
    shred->AddFalling(shred->GetBlock(x_in_shred, y_in_shred, z+1));
    emit Updated(x, y, z);
}

bool World::Build(Block * const block, const int x, const int y, const int z,
        const int dir, Block * const who, const bool anyway)
{
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    Block * const target_block = shred->GetBlock(x_in, y_in, z);
    if ( ENVIRONMENT!=target_block->PushResult(ANYWHERE) && not anyway ) {
        if ( who != nullptr ) {
            who->ReceiveSignal(tr("Cannot build here."));
        }
        return false;
    } // else:
    block->Restore();
    block->SetDir(dir);
    const int old_transparency = target_block->Transparent();
    const int new_transparency = block->Transparent();
    const int block_light = block->LightRadius();
    shred->SetBlock(block, x_in, y_in, z);
    if ( old_transparency != new_transparency ) {
        ReEnlighten(x, y, z);
    }
    if ( block_light ) {
        AddFireLight(x, y, z, block_light);
    }
    shred->AddFalling(shred->GetBlock(x_in, y_in, z+1));
    return true;
}

bool World::Inscribe(const int x, const int y, const int z) {
    Shred * const shred = GetShred(x, y);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
    Block * block  = shred->GetBlock(x_in, y_in, z);
    if ( block == block_manager->Normal(block->Sub()) ) {
        block = BlockManager::NewBlock(block->Kind(), block->Sub());
        shred->SetBlockNoCheck(block, x_in, y_in, z);
    }
    QString str;
    emit GetString(str);
    return block->Inscribe(str);
}

bool World::Exchange(Block * const block_from, Block * const block_to,
        const int src, const int dest, const int num)
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
            block_to->ReceiveSignal(tr("Nothing can be obtained."));
            return false;
        }
    }
    if ( src >= inv_from->Size() || inv_from->Number(src) == 0 ) {
        block_from->ReceiveSignal(tr("Nothing here."));
        block_to  ->ReceiveSignal(tr("Nothing here."));
    } else {
        Active * active = block_from->ActiveBlock();
        if ( active != nullptr ) {
            emit active->Updated();
        }
        active = block_to->ActiveBlock();
        if ( active != nullptr) {
            emit active->Updated();
        }
        inv_from->Drop(src, dest, num, inv_to);
    }
    return false;
}

void World::LoadAllShreds() {
    for (long j=longitude-NumShreds()/2, y=0; y<NumShreds(); ++j, ++y)
    for (long i=latitude -NumShreds()/2, x=0; x<NumShreds(); ++i, ++x) {
        *FindShred(x, y) = new Shred(x, y, j, i);
    }
    sunMoonFactor = evernight ?
        0 : ( TIME_NIGHT==PartOfDay() ) ?
            MOON_LIGHT_FACTOR : SUN_LIGHT_FACTOR;
    ReEnlightenAll();
}

void World::DeleteAllShreds() {
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        delete shreds[i];
    }
}

int World::CorrectNumShreds(int num) {
    num += ( num%2 == 0 ); // must be odd
    return qMax(num, MIN_WORLD_SIZE);
}

int World::CorrectNumActiveShreds(int num, const int num_shreds) {
    num += ( num%2 == 0 ); // must be odd
    return qBound(3, num, num_shreds);
}

World::World(const QString world_name, bool * error) :
        worldName(world_name),
        map(new WorldMap(world_name)),
        time(), timeStep(),
        shreds(),
        longitude(), latitude(),
        gameSettings(
            new QSettings(home_path + "freg.ini", QSettings::IniFormat)),
        numShreds(CorrectNumShreds(
            gameSettings->value("number_of_shreds", MIN_WORLD_SIZE).toInt())),
        numActiveShreds(CorrectNumActiveShreds(
            gameSettings->value("number_of_active_shreds", numShreds).toInt(),
                numShreds)),
        mutex(),
        evernight(),
        newLati(), newLongi(),
        newX(), newY(), newZ(),
        newWorld(),
        toResetDir(UP),
        sunMoonFactor(),
        shredStorage(),
        initial_lighting(),
        notes()
{
    world = this;

    LoadState();
    gameSettings->setValue("number_of_shreds", numShreds);
    gameSettings->setValue("number_of_active_shreds", numActiveShreds);
    delete gameSettings;

    shreds = new Shred *[NumShreds()*NumShreds()];
    if ( not QDir(home_path).mkpath(worldName + "/texts") ) {
        *error = true;
    }

    shredStorage = new ShredStorage(numShreds+2, longitude, latitude);

    LoadNotes();
    LoadAllShreds();
    emit UpdatedAll();
}

World::~World() {
    Lock();
    quit();
    wait();
    Unlock();

    DeleteAllShreds();
    delete [] shreds;
    delete shredStorage;
    delete map;

    SaveState();
    QSettings(home_path + "freg.ini", QSettings::IniFormat).
        setValue("current_world", worldName);

    SaveNotes();
}

void World::SaveState() const {
    QSettings settings(home_path + worldName + "/settings.ini",
        QSettings::IniFormat);
    settings.setValue("time", qlonglong(time));
    settings.setValue("time_step", timeStep);
    settings.setValue("longitude", qlonglong(longitude));
    settings.setValue("latitude", qlonglong(latitude));
    settings.setValue("evernight", evernight);
}

void World::LoadNotes() {
    notes.clear();
    QFile notes_file(home_path + worldName + "/notes.txt");
    if ( notes_file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        char note[MAX_NOTE_LENGTH*2];
        while ( notes_file.readLine(note, MAX_NOTE_LENGTH*2) > 0 ) {
            notes.append(QString(note).trimmed());
        }
    }
}

void World::SaveNotes() const {
    QFile notes_file(home_path + worldName + "/notes.txt");
    if ( notes_file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        for (int i=0; i<notes.size(); ++i) {
            notes_file.write(qPrintable(notes.at(i)));
            notes_file.putChar('\n');
        }
    }
}

void World::LoadState() {
    const QSettings settings(home_path + worldName +"/settings.ini",
        QSettings::IniFormat);
    time = settings.value("time", END_OF_NIGHT).toLongLong();
    timeStep = settings.value("time_step", 0).toInt();
    longitude = settings.value("longitude",
        qlonglong(map->GetSpawnLongitude())).toLongLong();
    latitude  = settings.value("latitude",
        qlonglong(map->GetSpawnLatitude ())).toLongLong();
    evernight = settings.value("evernight", false).toBool();
}
