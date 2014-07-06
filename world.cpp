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

#include <QTimer>
#include <QDir>
#include <QSettings>
#include <QMutexLocker>
#include <memory>
#include "blocks/Active.h"
#include "blocks/Inventory.h"
#include "Shred.h"
#include "world.h"
#include "BlockManager.h"
#include "ShredStorage.h"
#include "CraftManager.h"

const int MIN_WORLD_SIZE = 7;

World * world;

CraftManager * World::GetCraftManager() const { return craftManager; }

int World::ShredPos(const int x, const int y) const { return y*NumShreds()+x; }

Shred * World::GetShred(const int x, const int y) const {
    return shreds[ShredPos(Shred::CoordOfShred(x), Shred::CoordOfShred(y))];
}

Shred * World::GetShredByPos(const int x, const int y) const {
    return shreds[ShredPos(x, y)];
}

int World::NumShreds() const { return numShreds; }
int World::TimeOfDay() const { return time % SECONDS_IN_DAY; }
int World::TimeStepsInSec() { return TIME_STEPS_IN_SEC; }
int World::MiniTime() const { return timeStep; }
ulong World::Time() const { return time; }
long World::GetSpawnLongi() const { return spawnLongi; }
long World::GetSpawnLati()  const { return spawnLati; }
long World::Longitude() const { return longitude; }
long World::Latitude()  const { return latitude; }
long World::MapSize() const { return map.MapSize(); }
bool World::GetEvernight() const { return evernight; }
QString World::WorldName() const { return worldName; }

char World::TypeOfShred(const long longi, const long lati) const {
    return map.TypeOfShred(longi, lati);
}

QByteArray * World::GetShredData(long longi, long lati) const {
    return shredStorage->GetShredData(longi, lati);
}

void World::SetShredData(QByteArray * const data,
        const long longi, const long lati)
{
    shredStorage->SetShredData(data, longi, lati);
}

int World::SunMoonX() const {
    return ( TIME_NIGHT == PartOfDay() ) ?
        TimeOfDay()*SHRED_WIDTH*NumShreds()/
            SECONDS_IN_NIGHT :
        (TimeOfDay()-SECONDS_IN_NIGHT)*SHRED_WIDTH*NumShreds()/
            SECONDS_IN_DAYLIGHT;
}

times_of_day World::PartOfDay() const {
    return static_cast<times_of_day>(TimeOfDay() / SECONDS_IN_NIGHT);
}

QString World::TimeOfDayStr() const {
    return tr("Time is %1:%2.").
        arg(TimeOfDay()/60).
        arg(TimeOfDay()%60, 2, 10, QChar('0'));
}

void World::Drop(Block * const block_from,
        const int x_to, const int y_to, const int z_to,
        const int src, const int dest, const int num)
{
    Block * block_to = GetBlock(x_to, y_to, z_to);
    if ( AIR == block_to->Sub() ) {
        SetBlock((block_to=NewBlock(CONTAINER, DIFFERENT)), x_to, y_to, z_to);
    } else if ( LIQUID == block_to->Kind() ) {
        Block * const pile = NewBlock(CONTAINER, DIFFERENT);
        SetBlock(pile, x_to, y_to, z_to);
        pile->HasInventory()->Get(block_to);
        block_to = pile;
    }
    Exchange(block_from, block_to, src, dest, num);
    emit Updated(x_to, y_to, z_to);
}

void World::Get(Block * const block_to,
        const int x_from, const int y_from, const int z_from,
        const int src, const int dest, const int num)
{
    Block * const block_from = GetBlock(x_from, y_from, z_from);
    Inventory * const inv = block_from->HasInventory();
    if ( inv == nullptr ) { // for vessel
        if ( block_from->Kind() == LIQUID ) {
            Inventory * const inv_to = block_to->HasInventory();
            if ( inv_to == nullptr ) return;
            Block * const vessel = inv_to->ShowBlock(src);
            if ( vessel == nullptr ) return;
            Inventory * const vessel_inv = vessel->HasInventory();
            if ( vessel_inv == nullptr ) return;
            Block * const tried = NewBlock(LIQUID, block_from->Sub());
            if ( vessel_inv->Get(tried, 0) ) {
                SetBlock(Normal(AIR), x_from, y_from, z_from);
                Shred * const shred = GetShred(x_from, y_from);
                shred->AddFalling(shred->GetBlock(
                    Shred::CoordInShred(x_from),
                    Shred::CoordInShred(y_from), z_from+1));
                emit Updated(x_from, y_from, z_from);
            } else {
                delete tried;
            }
        }
    } else if ( inv->Access() ) {
        Exchange(block_from, block_to, src, dest, num);
    }
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

void World::ReloadAllShreds(const long lati, const long longi,
    const int new_x, const int new_y, const int new_z)
{
    newLati  = lati;
    newLongi = longi;
    newX = new_x;
    newY = new_y;
    newZ = new_z;
    toResetDir = DOWN; // full reset
}

QMutex * World::GetLock() { return &mutex; }
void World::Lock() { mutex.lock(); }
bool World::TryLock() { return mutex.tryLock(); }
void World::Unlock() { mutex.unlock(); }

void World::run() {
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), SLOT(PhysEvents()),
        Qt::DirectConnection);
    timer.start(1000/TimeStepsInSec());
    exec();
}

dirs World::TurnRight(const dirs dir) {
    switch ( dir ) {
    case UP:
    case DOWN:  return dir;
    case NORTH: return EAST;
    case SOUTH: return WEST;
    case EAST:  return SOUTH;
    case WEST:
    case NOWHERE: return NOWHERE;
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
    case NOWHERE: return NOWHERE;
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
    case NOWHERE: return NOWHERE;
    }
    Q_UNREACHABLE();
    return NORTH;
}

void World::MakeSun() {
    behindSun =
        GetBlock( (sunMoonX=SunMoonX()), SHRED_WIDTH*NumShreds()/2, HEIGHT-1);
    PutBlock(Normal(SUN_MOON), sunMoonX, SHRED_WIDTH*NumShreds()/2, HEIGHT-1);
}

Block * World::GetBlock(const int x, const int y, const int z) const {
    return GetShred(x, y)->
        GetBlock(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

void World::SetBlock(Block * const block,
        const int x, const int y, const int z)
{
    GetShred(x, y)->SetBlock(block,
        Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

void World::PutBlock(Block * const block,
        const int x, const int y, const int z)
{
    GetShred(x, y)->PutBlock(block,
        Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

Block * World::Normal(const int sub) { return block_manager.NormalBlock(sub); }

Block * World::NewBlock(const int kind, const int sub) {
    return block_manager.NewBlock(kind, sub);
}

Shred ** World::FindShred(const int x, const int y) const {
    return &shreds[ShredPos(x, y)];
}

void World::ReloadShreds(const int direction) {
    emit StartMove(direction);
    RemSun();
    switch ( direction ) {
    case NORTH:
        --longitude;
        for (int x=0; x<NumShreds(); ++x) {
            const Shred * const shred = *FindShred(x, NumShreds()-1);
            Shred * const memory = shred->GetShredMemory();
            shred->~Shred();
            for (int y=NumShreds()-1; y>0; --y) {
                ( *FindShred(x, y) = *FindShred(x, y-1) )->ReloadTo(NORTH);
            }
            *FindShred(x, 0) = new(memory) Shred(x, 0,
                longitude - NumShreds()/2,
                latitude  - NumShreds()/2+x, memory);
        }
    break;
    case SOUTH:
        ++longitude;
        for (int x=0; x<NumShreds(); ++x) {
            const Shred * const shred = *FindShred(x, 0);
            Shred * const memory = shred->GetShredMemory();
            shred->~Shred();
            for (int y=0; y<NumShreds()-1; ++y) {
                ( *FindShred(x, y) = *FindShred(x, y+1) )->ReloadTo(SOUTH);
            }
            *FindShred(x, NumShreds()-1) = new(memory) Shred(x, NumShreds()-1,
                longitude + NumShreds()/2,
                latitude  - NumShreds()/2+x, memory);
        }
    break;
    case EAST:
        ++latitude;
        for (int y=0; y<NumShreds(); ++y) {
            const Shred * const shred = *FindShred(0, y);
            Shred * const memory = shred->GetShredMemory();
            shred->~Shred();
            for (int x=0; x<NumShreds()-1; ++x) {
                ( *FindShred(x, y) = *FindShred(x+1, y) )->ReloadTo(EAST);
            }
            *FindShred(NumShreds()-1, y) = new(memory) Shred(NumShreds()-1, y,
                longitude - NumShreds()/2+y,
                latitude  + NumShreds()/2, memory);
        }
    break;
    case WEST:
        --latitude;
        for (int y=0; y<NumShreds(); ++y) {
            const Shred * const shred = *FindShred(NumShreds()-1, y);
            Shred * const memory = shred->GetShredMemory();
            shred->~Shred();
            for (int x=NumShreds()-1; x>0; --x) {
                ( *FindShred(x, y) = *FindShred(x-1, y) )->ReloadTo(WEST);
            }
            *FindShred(0, y) = new(memory) Shred(0, y,
                longitude - NumShreds()/2+y,
                latitude  - NumShreds()/2, memory);
        }
    break;
    default: fprintf(stderr,
        "%s: invalid direction: %d.\n", Q_FUNC_INFO, direction);
    }
    shredStorage->Shift(direction, longitude, latitude);
    MakeSun();
    ReEnlightenMove(direction);
    emit Moved(direction);
} // void World::ReloadShreds(int direction)

void World::SetReloadShreds(const int direction) { toResetDir = direction; }

void World::PhysEvents() {
    Lock();
    switch ( toResetDir ) {
    case UP: break; // no reset
    default:
        ReloadShreds(toResetDir);
        toResetDir = UP; // set no reset
    break;
    case DOWN: // full reset
        emit StartReloadAll();
        DeleteAllShreds();
        longitude = newLongi;
        latitude  = newLati;
        LoadAllShreds();
        emit NeedPlayer(newX, newY, newZ);
        emit UpdatedAll();
        emit FinishReloadAll();
        toResetDir = UP; // set no reset
    break;
    }

    static const int start = NumShreds()/2 - numActiveShreds/2;
    static const int end   = start + numActiveShreds;
    for (int i=start; i<end; ++i)
    for (int j=start; j<end; ++j) {
        shreds[ShredPos(i, j)]->PhysEventsFrequent();
    }

    if ( TimeStepsInSec() > timeStep ) {
        ++timeStep;
        Unlock();
        emit UpdatesEnded();
        return;
    } // else:

    for (int i=start; i<end; ++i)
    for (int j=start; j<end; ++j) {
        shreds[ShredPos(i, j)]->PhysEventsRare();
    }
    timeStep = 0;
    ++time;
    // sun/moon moving
    if ( not GetEvernight() && sunMoonX != SunMoonX() ) {
        static const int y = SHRED_WIDTH*NumShreds()/2;
        PutBlock(behindSun, sunMoonX, y, HEIGHT-1);
        emit Updated(sunMoonX, y, HEIGHT-1);
        behindSun = GetBlock(( sunMoonX=SunMoonX() ), y, HEIGHT-1);
        PutBlock(Normal(SUN_MOON), sunMoonX, y, HEIGHT-1);
        emit Updated(sunMoonX, y, HEIGHT-1);
    }
    switch ( TimeOfDay() ) {
    default: break;
    case END_OF_NIGHT:
        emit Notify(tr("It's morning now."));
        ReEnlightenTime();
        break;
    case END_OF_MORNING: emit Notify(tr("It's day now.")); break;
    case END_OF_NOON:    emit Notify(tr("It's evening now.")); break;
    case END_OF_EVENING:
        emit Notify(tr("It's night now."));
        ReEnlightenTime();
        break;
    }
    Unlock();
    emit UpdatesEnded();
    // emit ExitReceived(); // close all after 1 turn
} // void World::PhysEvents()

bool World::DirectlyVisible(int x_from, int y_from, int z_from,
        int x, int y, int z)
const {
    /// optimized DDA line with integers only.
    int max = Abs(Abs(z-=z_from) > Abs(y-=y_from) ? z : y);
    if ( Abs(x-=x_from) > max) {
        max = Abs(x);
    }
    x_from *= max;
    y_from *= max;
    z_from *= max;
    for (int i=1; i<max; ++i) {
        if ( not (GetBlock((x_from+=x)/max, (y_from+=y)/max, (z_from+=z)/max)->
                    Transparent()
                || GetBlock( (x_from+max-1)/max, (y_from+max-1)/max,
                    (z_from+max-1)/max )->Transparent()) )
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
        || (x_to!=x_from &&
            GetBlock(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to)->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to))
        || (y_to!=y_from &&
            GetBlock(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to)->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to))
        || (z_to!=z_from &&
            GetBlock(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1))->
                Transparent()
            && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp))
        );
}

bool World::Move(const int x, const int y, const int z, const dirs dir) {
    int newx, newy, newz;
    if (      Focus(x, y, z, &newx, &newy, &newz, dir) &&
            CanMove(x, y, z,  newx,  newy,  newz, dir) )
    {
        NoCheckMove(x, y, z,  newx,  newy,  newz, dir);
        return true;
    } else {
        return false;
    }
}

bool World::CanMove(const int x, const int y, const int z,
        const int newx, const int newy, const int newz, const dirs dir)
{
    Block * const block    = GetBlock(x, y, z);
    Block * const block_to = GetBlock(newx, newy, newz);
    if ( DOWN != dir && block->Weight() != 0 ) {
        Falling * const falling = block->ShouldFall();
        if ( falling != nullptr
                && falling->IsFalling()
                && AIR==GetBlock(x, y, z-1)->Sub()
                && AIR==GetBlock(newx, newy, newz-1)->Sub() )
        {
            return false;
        }
    }
    switch ( block->PushResult(dir) ) {
    case MOVABLE: break;
    case ENVIRONMENT:
        if ( *block == *block_to ) { // prevent useless flow
            return false;
        }
        break;
    default:
    case NOT_MOVABLE: return false;
    }
    const push_reaction target_push = block_to->PushResult(dir);
    block_to->Push(dir, block);
    switch ( target_push ) {
    case MOVABLE:
        return ( (block->Weight() > block_to->Weight()) &&
                Move(newx, newy, newz, dir) );
    case ENVIRONMENT: return true;
    case NOT_MOVABLE: return false;
    case MOVE_UP:
        if ( dir > DOWN ) { // not UP and not DOWN
            Move(x, y, z, UP);
        }
        return false;
    case JUMP:
        if ( dir > DOWN ) { // not UP and not DOWN
            Jump(x, y, z, dir);
        }
        return false;
    }
    Q_UNREACHABLE();
    return false;
} // bool World::CanMove(const int x, y, z, newx, newy, newz, int dir)

void World::NoCheckMove(const int x, const int y, const int z,
        const int newx, const int newy, const int newz, const dirs dir)
{
    Shred * const shred_from = GetShred(x, y);
    Shred * const shred_to   = GetShred(newx, newy);
    const int x_in = Shred::CoordInShred(x);
    const int y_in = Shred::CoordInShred(y);
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

    shred_from->AddFalling(shred_from->GetBlock(x_in, y_in, z+1));
    shred_to  ->AddFalling(shred_from->GetBlock(newx_in, newy_in, newz+1));
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
    case NOWHERE: break;
    }
    return InBounds(*x_to, *y_to, *z_to);
}

int World::Damage(const int x, const int y, const int z,
        const int dmg, const int dmg_kind)
{
    Block * temp = GetBlock(x, y, z);
    const int sub  = temp->Sub();
    if ( AIR==sub ) return 0;
    const int kind = temp->Kind();
    if ( temp==Normal(sub) ) {
        temp = NewBlock(kind, sub);
    }
    temp->Damage(dmg, dmg_kind);
    int durability = temp->GetDurability();
    // SetBlock can alter temp (by ReplaceWithNormal) so put in last place.
    SetBlock(temp, x, y, z);
    return durability;
}

void World::DestroyAndReplace(const int x, const int y, const int z) {
    Shred * const shred = GetShred(x, y);
    const int x_in_shred = Shred::CoordInShred(x);
    const int y_in_shred = Shred::CoordInShred(y);
    Block * const block = shred->GetBlock(x_in_shred, y_in_shred, z);
    bool delete_block = true;
    Block * const new_block = block->DropAfterDamage(&delete_block);
    shred->SetBlockNoCheck(new_block, x_in_shred, y_in_shred, z);
    if (    block->Transparent() != new_block->Transparent() ||
            block->LightRadius() != new_block->LightRadius() )
    {
        ReEnlighten(x, y, z);
    }
    if ( delete_block ) {
        block_manager.DeleteBlock(block);
    } else {
        block->Restore();
        Active * const active = block->ActiveBlock();
        if ( active != nullptr ) {
            active->Unregister();
        }
    }
    shred->AddFalling(shred->GetBlock(x_in_shred, y_in_shred, z+1));
    emit Updated(x, y, z);
}

bool World::Build(Block * block, const int x, const int y, const int z,
        const int dir, Block * const who, const bool anyway)
{
    Block * const target_block = GetBlock(x, y, z);
    if ( ENVIRONMENT!=target_block->PushResult(NOWHERE) && not anyway ) {
        if ( who ) {
            who->ReceiveSignal(tr("Cannot build here."));
        }
        return false;
    } // else:
    block->Restore();
    block->SetDir(dir);
    const int old_transparency = target_block->Transparent();
    const int new_transparency = block->Transparent();
    const int block_light = block->LightRadius();
    SetBlock(block, x, y, z);
    if ( old_transparency != new_transparency ) {
        ReEnlighten(x, y, z);
    }
    if ( block_light ) {
        AddFireLight(x, y, z, block_light);
    }
    return true;
}

bool World::Inscribe(const int x, const int y, const int z) {
    Block * block = GetBlock(x, y, z);
    if ( LIQUID==block->Kind() || AIR==block->Sub() ) {
        return false;
    } // else:
    if ( block==Normal(block->Sub()) ) {
        block = NewBlock(block->Kind(), block->Sub());
    }
    QString str = tr("No note received.");
    emit GetString(str);
    const bool ok_flag = block->Inscribe(str);
    SetBlock(block, x, y, z);
    return ok_flag;
}

void World::Exchange(Block * const block_from, Block * const block_to,
        const int src, const int dest, const int num)
{
    Inventory * const inv_from = block_from->HasInventory();
    if ( inv_from == nullptr ) {
        block_from->ReceiveSignal(tr("No inventory."));
        return;
    }
    if ( inv_from->Number(src) == 0 ) {
        block_from->ReceiveSignal(tr("Nothing here."));
        block_to  ->ReceiveSignal(tr("Nothing here."));
        return;
    }
    Inventory * const inv_to = block_to->HasInventory();
    if ( inv_to == nullptr ) {
        block_from->ReceiveSignal(tr("No room there."));
        return;
    }
    if ( inv_from->Drop(src, dest, num, inv_to) ) {
        block_from->ReceiveSignal(tr("Your bag is lighter now."));
        block_to  ->ReceiveSignal(tr("Your bag is heavier now."));
    }
}

void World::RemSun() {
    if ( not GetEvernight() ) {
        PutBlock(behindSun, sunMoonX, SHRED_WIDTH*NumShreds()/2, HEIGHT-1);
    }
}

void World::LoadAllShreds() {
    shreds = new Shred *[NumShreds()*NumShreds()];
    shredMemoryPool = static_cast<Shred *>
        (operator new (sizeof(Shred)*NumShreds()*NumShreds()));
    for (long i=latitude -NumShreds()/2, x=0; x<NumShreds(); ++i, ++x)
    for (long j=longitude-NumShreds()/2, y=0; y<NumShreds(); ++j, ++y) {
        const int pos = ShredPos(x, y);
        shreds[pos] = new(shredMemoryPool + pos)
            Shred(x, y, j, i, shredMemoryPool + pos);
    }
    if ( evernight ) {
        sunMoonFactor = 0;
    } else {
        MakeSun();
        sunMoonFactor = ( TIME_NIGHT==PartOfDay() ) ?
            MOON_LIGHT_FACTOR : SUN_LIGHT_FACTOR;
    }
    ReEnlightenAll();
}

void World::DeleteAllShreds() {
    RemSun();
    for (int i=0; i<NumShreds()*NumShreds(); ++i) {
        shreds[i]->~Shred();
    }
    operator delete(shredMemoryPool);
    delete [] shreds;
}

void World::SetNumActiveShreds(const int num) {
    const QMutexLocker locker(&mutex);
    numActiveShreds = num;
    if ( 1 != numActiveShreds%2 ) {
        ++numActiveShreds;
        emit Notify(tr("Invalid active shreds number. Set to %1.").
            arg(numActiveShreds));
    }
    if ( numActiveShreds < 3 ) {
        numActiveShreds = 3;
        emit Notify(tr("Active shreds number too small, set to 3."));
    } else if ( numActiveShreds > NumShreds() ) {
        numActiveShreds = NumShreds();
        emit Notify(tr("Active shreds number too big. Set to %1.").
            arg(numActiveShreds));
    }
    emit Notify(tr("Active shreds number is %1x%1.").arg(numActiveShreds));
}

World::World(const QString world_name) :
        worldName(world_name),
        map(world_name),
        toResetDir(UP),
        craftManager(new CraftManager),
        initial_lighting()
{
    world = this;
    QSettings game_settings("freg.ini", QSettings::IniFormat);
    numShreds = game_settings.value("number_of_shreds", MIN_WORLD_SIZE).
        toLongLong();
    if ( 1 != numShreds%2 ) {
        ++numShreds;
        fprintf(stderr, "%s: Invalid number of shreds. Set to %d.\n",
            Q_FUNC_INFO, numShreds);
    }
    if ( numShreds < MIN_WORLD_SIZE ) {
        fprintf(stderr, "%s: Number of shreds to small: %d. Set to %d.\n",
            Q_FUNC_INFO, numShreds, MIN_WORLD_SIZE);
        numShreds = MIN_WORLD_SIZE;
    }
    SetNumActiveShreds(game_settings.value("number_of_active_shreds",
        numShreds).toUInt());
    game_settings.setValue("number_of_shreds", numShreds);
    game_settings.setValue("number_of_active_shreds", numActiveShreds);

    QDir::current().mkpath(worldName+"/texts");
    QSettings settings(worldName+"/settings.ini", QSettings::IniFormat);
    time = settings.value("time", END_OF_NIGHT).toLongLong();
    timeStep = settings.value("time_step", 0).toInt();
    spawnLongi = settings.value( "spawn_longitude",
        int(qrand() % MapSize()) ).toLongLong();
    spawnLati  = settings.value( "spawn_latitude",
        int(qrand() % MapSize()) ).toLongLong();
    settings.setValue("spawn_longitude", qlonglong(spawnLongi));
    settings.setValue("spawn_latitude",  qlonglong(spawnLati));
    longitude = settings.value("longitude", int(spawnLongi)).toLongLong();
    latitude  = settings.value("latitude",  int(spawnLati )).toLongLong();

    evernight = settings.value("evernight", false).toBool();
    settings.setValue("evernight", evernight); // save if not present

    shredStorage = new ShredStorage(numShreds+2, longitude, latitude);
    puts(qPrintable(tr("Loading world...")));
    LoadAllShreds();
    emit UpdatedAll();
}

World::~World() {
    Lock();
    quit();
    wait();
    Unlock();

    DeleteAllShreds();
    delete shredStorage;
    delete craftManager;

    QSettings settings(worldName+"/settings.ini", QSettings::IniFormat);
    settings.setValue("time", qlonglong(time));
    settings.setValue("time_step", timeStep);
    settings.setValue("longitude", qlonglong(longitude));
    settings.setValue("latitude", qlonglong(latitude));
}
