	/* freg, Free-Roaming Elementary Game with open and interactive world
	*  Copyright (C) 2012-2013 Alexander 'mmaulwurff' Kromm
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
#include <QWriteLocker>
#include <memory>
#include "Active.h"
#include "Inventory.h"
#include "Shred.h"
#include "world.h"
#include "worldmap.h"
#include "BlockManager.h"
#include "ShredStorage.h"

const ushort MIN_WORLD_SIZE = 7;

World * world;

int World::ShredPos(const int x, const int y) const {
	return y*NumShreds() + x;
}

Shred * World::GetShred(ushort x, ushort y) const {
	return shreds[ShredPos(Shred::CoordOfShred(x),
		Shred::CoordOfShred(y))];
}

int World::TimeOfDay() const { return time % SECONDS_IN_DAY; }
long World::GetSpawnLongi() const { return spawnLongi; }
long World::GetSpawnLati()  const { return spawnLati; }
long World::Longitude() const { return longitude; }
long World::Latitude()  const { return latitude; }
long World::MapSize() const { return map->MapSize(); }
ulong World::Time() const { return time; }
ushort World::TimeStepsInSec() { return TIME_STEPS_IN_SEC; }
ushort World::MiniTime() const { return timeStep; }
ushort World::NumShreds() const { return numShreds; }
QString World::WorldName() const { return worldName; }

char World::TypeOfShred(const long longi, const long lati) {
	return map->TypeOfShred(longi, lati);
}

QByteArray * World::GetShredData(long longi, long lati) const {
	return shredStorage->GetShredData(longi, lati);
}

void World::SetShredData(QByteArray * const data,
		const long longi, const long lati)
{
	shredStorage->SetShredData(data, longi, lati);
}

ushort World::SunMoonX() const {
	return ( NIGHT == PartOfDay() ) ?
		TimeOfDay()*SHRED_WIDTH*NumShreds()/
			SECONDS_IN_NIGHT :
		(TimeOfDay()-SECONDS_IN_NIGHT)*SHRED_WIDTH*NumShreds()/
			SECONDS_IN_DAYLIGHT;
}

int World::PartOfDay() const {
	const ushort time_day = TimeOfDay();
	if ( time_day < END_OF_NIGHT )   return NIGHT;
	if ( time_day < END_OF_MORNING ) return MORNING;
	if ( time_day < END_OF_NOON )    return NOON;
	return EVENING;
}

QString World::TimeOfDayStr() const {
	return tr("Time is %1:%2.").
		arg(TimeOfDay()/60).
		arg(TimeOfDay()%60, 2, 10, QChar('0'));
}

void World::Drop(Block * const block_from,
		const ushort x_to, const ushort y_to, const ushort z_to,
		const ushort src, const ushort dest, const ushort num)
{
	Block * block_to=GetBlock(x_to, y_to, z_to);
	if ( AIR == block_to->Sub() ) {
		SetBlock((block_to=NewBlock(PILE, DIFFERENT)),
			x_to, y_to, z_to);
	} else if ( WATER == block_to->Sub() ) {
		Block * const pile = NewBlock(PILE, DIFFERENT);
		SetBlock(pile, x_to, y_to, z_to);
		pile->HasInventory()->Get(block_to);
		block_to = pile;
	}
	Exchange(block_from, block_to, src, dest, num);
	emit Updated(x_to, y_to, z_to);
}

void World::Get(Block * const block_to,
		const ushort x_from, const ushort y_from, const ushort z_from,
		const ushort src, const ushort dest, const ushort num)
{
	Block * const block_from = GetBlock(x_from, y_from, z_from);
	Inventory * const inv = block_from->HasInventory();
	if ( inv && inv->Access() )
	{
		Exchange(block_from, block_to, src, dest, num);
	}
}

bool World::InBounds(const ushort x, const ushort y) const {
	static const ushort max_xy = SHRED_WIDTH*NumShreds();
	return ( x<max_xy && y<max_xy );
}
bool World::InVertBounds(const ushort z) {
	return ( z < HEIGHT );
}
bool World::InBounds(const ushort x, const ushort y, const ushort z) const {
	return ( InBounds(x, y) && InVertBounds(z) );
}

void World::ReloadAllShreds(const long lati, const long longi,
	const ushort new_x, const ushort new_y, const ushort new_z)
{
	newLati  = lati;
	newLongi = longi;
	newX = new_x;
	newY = new_y;
	newZ = new_z;
	toResetDir = DOWN; // full reset
}

QReadWriteLock * World::GetLock() const { return rwLock; }
void World::WriteLock() { rwLock->lockForWrite(); }
void World::ReadLock()  { rwLock->lockForRead(); }
bool World::TryReadLock() { return rwLock->tryLockForRead(); }
void World::Unlock() { rwLock->unlock(); }

void World::run() {
	QTimer timer;
	connect(&timer, SIGNAL(timeout()),
		this, SLOT(PhysEvents()),
		Qt::DirectConnection);
	timer.start(1000/TimeStepsInSec());
	exec();
}

quint8 World::TurnRight(const quint8 dir) {
	switch ( dir ) {
	default:
		fprintf(stderr, "World::TurnRight:Unlisted dir: %d\n",
			(int)dir);
	// no break;
	case WEST:  return NORTH;
	case NORTH: return EAST;
	case EAST:  return SOUTH;
	case SOUTH: return WEST;
	case UP:
	case DOWN:  return dir;
	}
}
quint8 World::TurnLeft(const quint8 dir) {
	switch ( dir ) {
	default:
		fprintf(stderr, "TurnLeft:Unlisted dir: %d\n",
			(int)dir);
	// no break;
	case EAST:  return NORTH;
	case NORTH: return WEST;
	case WEST:  return SOUTH;
	case SOUTH: return EAST;
	case UP:
	case DOWN:  return dir;
	}
}

void World::MakeSun() {
	ifStar = ( STAR==GetBlock( (sun_moon_x=SunMoonX()),
		SHRED_WIDTH*NumShreds()/2, HEIGHT-1)->Sub() );
	PutBlock(Normal(SUN_MOON),
		sun_moon_x, SHRED_WIDTH*NumShreds()/2, HEIGHT-1);
}

Block * World::GetBlock(const ushort x, const ushort y, const ushort z) const {
	return GetShred(x, y)->
		GetBlock(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

void World::SetBlock(Block * const block,
		const ushort x, const ushort y, const ushort z)
{
	GetShred(x, y)->SetBlock(block,
		Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

void World::PutBlock(Block * const block,
		const ushort x, const ushort y, const ushort z)
{
	GetShred(x, y)->PutBlock(block,
		Shred::CoordInShred(x), Shred::CoordInShred(y), z);
}

Block * World::Normal(const quint8 sub) {
	return block_manager.NormalBlock(sub);
}
Block * World::NewBlock(const int kind, const int sub) {
	return block_manager.NewBlock(kind, sub);
}
void World::DeleteBlock(Block * const block) {
	Active * const active = block->ActiveBlock();
	if ( active ) {
		active->SetToDelete();
	} else {
		block_manager.DeleteBlock(block);
	}
}

quint8 World::Anti(const quint8 dir) {
	switch ( dir ) {
	case NORTH: return SOUTH;
	case EAST:  return WEST;
	case SOUTH: return NORTH;
	case WEST:  return EAST;
	case UP:    return DOWN;
	case DOWN:  return UP;
	default:
		fprintf(stderr, "World::Anti(int): unlisted dir: %d\n",
			(int)dir);
		return NOWHERE;
	}
}

Shred ** World::FindShred(const ushort x, const ushort y) const {
	return &shreds[ShredPos(x, y)];
}

void World::ReloadShreds(const int direction) {
	short x, y; // do not make unsigned, values <0 are needed for checks
	RemSun();
	switch ( direction ) {
	case NORTH:
		--longitude;
		for (x=0; x<NumShreds(); ++x) {
			const Shred * const shred=*FindShred(x, NumShreds()-1);
			Shred * const memory = shred->GetShredMemory();
			shred->~Shred();
			for (y=NumShreds()-1; y>0; --y) {
				( *FindShred(x, y) = *FindShred(x, y-1) )->
					ReloadToNorth();
			}
			*FindShred(x, 0) = new(memory)
				Shred(x, 0,
					longitude-NumShreds()/2,
					latitude -NumShreds()/2+x, memory);
		}
	break;
	case SOUTH:
		++longitude;
		for (x=0; x<NumShreds(); ++x) {
			const Shred * const shred = *FindShred(x, 0);
			Shred * const memory = shred->GetShredMemory();
			shred->~Shred();
			for (y=0; y<NumShreds()-1; ++y) {
				( *FindShred(x, y) = *FindShred(x, y+1) )->
					ReloadToSouth();
			}
			*FindShred(x, NumShreds()-1) = new(memory)
				Shred(x, NumShreds()-1,
					longitude+NumShreds()/2,
					latitude -NumShreds()/2+x, memory);
		}
	break;
	case EAST:
		++latitude;
		for (y=0; y<NumShreds(); ++y) {
			const Shred * const shred = *FindShred(0, y);
			Shred * const memory = shred->GetShredMemory();
			shred->~Shred();
			for (x=0; x<NumShreds()-1; ++x) {
				( *FindShred(x, y) = *FindShred(x+1, y) )->
					ReloadToEast();
			}
			*FindShred(NumShreds()-1, y) = new(memory)
				Shred(NumShreds()-1, y,
					longitude-NumShreds()/2+y,
					latitude +NumShreds()/2, memory);
		}
	break;
	case WEST:
		--latitude;
		for (y=0; y<NumShreds(); ++y) {
			const Shred * const shred=*FindShred(NumShreds()-1, y);
			Shred * const memory = shred->GetShredMemory();
			shred->~Shred();
			for (x=NumShreds()-1; x>0; --x) {
				( *FindShred(x, y) = *FindShred(x-1, y) )->
					ReloadToWest();
			}
			*FindShred(0, y) = new(memory)
				Shred(0, y,
					longitude-NumShreds()/2+y,
					latitude -NumShreds()/2, memory);
		}
	break;
	default: fprintf(stderr,
		"World::ReloadShreds(int): invalid direction: %d\n",
		direction);
	}
	shredStorage->Shift(direction, longitude, latitude);
	MakeSun();
	ReEnlightenMove(direction);
	emit Moved(direction);
} // void World::ReloadShreds(int direction)

void World::SetReloadShreds(const int direction) { toResetDir=direction; }

void World::PhysEvents() {
	const QWriteLocker writeLock(rwLock);
	switch ( toResetDir ) {
	case UP: break; // no reset
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
	default:
		ReloadShreds(toResetDir);
		toResetDir = UP; // set no reset
	}

	static const ushort start = NumShreds()/2-numActiveShreds/2;
	static const ushort end   = start+numActiveShreds;
	for (ushort i=start; i<end; ++i)
	for (ushort j=start; j<end; ++j) {
		shreds[ShredPos(i, j)]->PhysEventsFrequent();
	}

	if ( TimeStepsInSec() > timeStep ) {
		++timeStep;
		for (ushort i=start; i<end; ++i)
		for (ushort j=start; j<end; ++j) {
			shreds[i+j*NumShreds()]->DeleteDestroyedActives();
		}
		return;
	} // else:

	for (ushort i=start; i<end; ++i)
	for (ushort j=start; j<end; ++j) {
		shreds[ShredPos(i, j)]->PhysEventsRare();
	}
	for (ushort i=start; i<end; ++i)
	for (ushort j=start; j<end; ++j) {
		shreds[ShredPos(i, j)]->DeleteDestroyedActives();
	}
	timeStep = 0;
	++time;
	// sun/moon moving
	if ( sun_moon_x != SunMoonX() ) {
		const ushort y = SHRED_WIDTH*NumShreds()/2;
		PutBlock(Normal(ifStar ? STAR : SKY), sun_moon_x, y, HEIGHT-1);
		emit Updated(sun_moon_x, y, HEIGHT-1);
		sun_moon_x = SunMoonX();
		ifStar = ( STAR==GetBlock(sun_moon_x, y, HEIGHT-1)->Sub() );
		PutBlock(Normal(SUN_MOON), sun_moon_x, y, HEIGHT-1);
		emit Updated(sun_moon_x, y, HEIGHT-1);
	}
	switch ( TimeOfDay() ) {
	case END_OF_NIGHT:
		ReEnlightenTime();
		emit Notify(tr("It's morning now."));
	break;
	case END_OF_MORNING: emit Notify(tr("It's day now.")); break;
	case END_OF_NOON:    emit Notify(tr("It's evening now.")); break;
	case END_OF_EVENING:
		ReEnlightenTime();
		emit Notify(tr("It's night now."));
	break;
	}
	emit UpdatesEnded();
	// emit ExitReceived(); // close all after 1 turn
} // void World::PhysEvents()

bool World::DirectlyVisible(float x_from, float y_from, float z_from,
		const ushort x_to, const ushort y_to, const ushort z_to)
const {
	return ( x_from==x_to && y_from==y_to && z_from==z_to ) || (
		( x_to<x_from && y_to<y_from ) ||
		( x_to>x_from && y_to<y_from && y_to>(2*x_from-x_to-3) ) ||
		( x_to<x_from && y_to>y_from && y_to>(2*x_from-x_to-3) ) ) ?
			NegativeVisible(x_from, y_from, z_from,
				x_to, y_to, z_to) :
			PositiveVisible(x_from, y_from, z_from,
				x_to, y_to, z_to);
}

bool World::NegativeVisible(float x_from, float y_from, float z_from,
		short x_to, short y_to, const short z_to)
const {
	// this function is like World::PositiveVisible
	x_from = -x_from;
	y_from = -y_from;
	x_to = -x_to;
	y_to = -y_to;
	const ushort max = qMax(qAbs(x_to-(short)x_from),
		qMax(qAbs(z_to-(short)z_from), qAbs(y_to-(short)y_from)));
	const float x_step = (x_to-x_from)/max;
	const float y_step = (y_to-y_from)/max;
	const float z_step = (z_to-z_from)/max;
	for (ushort i=1; i<max; ++i) {
		if ( BLOCK_OPAQUE == GetBlock(
				-qRound(x_from+=x_step),
				-qRound(y_from+=y_step),
				 qRound(z_from+=z_step))->Transparent() )
		{
			return false;
		}
	}
	return true;
}

bool World::PositiveVisible(float x_from, float y_from, float z_from,
		const ushort x_to, const ushort y_to, const ushort z_to)
const {
	const ushort max = qMax(qAbs(x_to-(short)x_from),
		qMax(qAbs(z_to-(short)z_from), qAbs(y_to-(short)y_from)));
	const float x_step = (x_to-x_from)/max;
	const float y_step = (y_to-y_from)/max;
	const float z_step = (z_to-z_from)/max;
	for (ushort i=1; i<max; ++i) {
		if ( BLOCK_OPAQUE == GetBlock(
				qRound(x_from+=x_step),
				qRound(y_from+=y_step),
				qRound(z_from+=z_step))->Transparent() )
		{
		   	return false;
		}
	}
	return true;
}

bool World::Visible(
		const ushort x_from, const ushort y_from, const ushort z_from,
		const ushort x_to,   const ushort y_to,   const ushort z_to)
const {
	short temp;
	return (
		(DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
		(GetBlock(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to)->
				Transparent()
			&& DirectlyVisible(
				x_from,    y_from, z_from,
				x_to+temp, y_to,   z_to)) ||
		(GetBlock(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to)->
				Transparent()
			&& DirectlyVisible(
				x_from, y_from,    z_from,
				x_to,   y_to+temp, z_to)) ||
		(GetBlock(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1))->
				Transparent()
			&& DirectlyVisible(
				x_from, y_from, z_from,
				x_to,   y_to,   z_to+temp)) );
}

bool World::Move(const ushort x, const ushort y, const ushort z,
		const quint8 dir)
{
	ushort newx, newy, newz;
	if ( !Focus(x, y, z, newx, newy, newz, dir) &&
			CanMove(x, y, z, newx, newy, newz, dir) )
	{
		NoCheckMove(x, y, z, newx, newy, newz, dir);
		return true;
	} else {
		return false;
	}
}

bool World::CanMove(const ushort x, const ushort y, const ushort z,
		const ushort newx, const ushort newy, const ushort newz,
		const quint8 dir)
{
	bool move_flag;
	Block * const block = GetBlock(x, y, z);
	Block * block_to = GetBlock(newx, newy, newz);
	if ( ENVIRONMENT == block->PushResult(NOWHERE) ) {
		move_flag = (*block != *block_to) &&
				MOVABLE == block_to->PushResult(NOWHERE);
	} else {
		block_to->Push(dir, block);
		block_to = GetBlock(newx, newy, newz);
		switch ( block_to->PushResult(dir) ) {
		default:
		case NOT_MOVABLE: move_flag = false; break;
		case ENVIRONMENT: move_flag = true; break;
		case JUMP:
			if ( DOWN!=dir && UP!=dir ) {
				Jump(x, y, z, dir);
			}
			move_flag = false;
		break;
		case MOVE_UP:
			if ( DOWN!=dir && UP!=dir ) {
				Move(x, y, z, UP);
			}
			move_flag = false;
		break;
		case MOVABLE:
			move_flag = ( (block->Weight() > block_to->Weight()) &&
				Move(newx, newy, newz, dir) );
		break;
		}
	}
	Active * active;
	return ( move_flag &&
		(DOWN==dir || !block->Weight() ||
		!( (active=block->ActiveBlock()) &&
			active->IsFalling() &&
			AIR==GetBlock(x, y, z-1)->Sub() &&
			AIR==GetBlock(newx, newy, newz-1)->Sub() )) );
} // bool World::CanMove(ushort x, y, z, newx, newy, newz, quint8 dir)

void World::NoCheckMove(const ushort x, const ushort y, const ushort z,
		const ushort newx, const ushort newy, const ushort newz,
		const quint8 dir)
{
	Block * const block = GetBlock(x, y, z);
	Block * const block_to = GetBlock(newx, newy, newz);

	PutBlock(block_to, x, y, z);
	PutBlock(block, newx, newy, newz);

	if ( block_to->Transparent() != block->Transparent() ) {
		ReEnlighten(newx, newy, newz);
		ReEnlighten(x, y, z);
	}

	block_to->Move( Anti(dir) );
	block->Move(dir);

	Shred * shred = GetShred(x, y);
	shred->AddFalling(Shred::CoordInShred(x), Shred::CoordInShred(y), z+1);
	shred->AddFalling(Shred::CoordInShred(x), Shred::CoordInShred(y), z);
	shred = GetShred(newx, newy);
	shred->AddFalling(Shred::CoordInShred(newx), Shred::CoordInShred(newy),
		newz+1);
	shred->AddFalling(Shred::CoordInShred(newx), Shred::CoordInShred(newy),
		newz);
}

void World::Jump(const ushort x, const ushort y, const ushort z,
		const quint8 dir)
{
	if ( !(AIR==GetBlock(x, y, z-1)->Sub() &&
			GetBlock(x, y, z)->Weight()) &&
			Move(x, y, z, UP) )
	{
		Move(x, y, z+1, dir);
	}
}

bool World::Focus(const ushort x, const ushort y, const ushort z,
		ushort & x_to, ushort & y_to, ushort & z_to, const quint8 dir)
const {
	x_to = x;
	y_to = y;
	z_to = z;
	switch ( dir ) {
	case NORTH: --y_to; break;
	case SOUTH: ++y_to; break;
	case EAST:  ++x_to; break;
	case WEST:  --x_to; break;
	case DOWN:  --z_to; break;
	case UP:    ++z_to; break;
	default:
		fprintf(stderr, "World::Focus: unlisted dir: %d\n", dir);
		return true;
	}
	return !InBounds(x_to, y_to, z_to);
}

void World::Damage(const ushort x, const ushort y, const ushort z,
		const ushort dmg, const int dmg_kind)
{
	Block * temp = GetBlock(x, y, z);
	if ( temp==Normal(temp->Sub()) && AIR!=temp->Sub() ) {
		temp = NewBlock(temp->Kind(), temp->Sub());
	}
	temp->Damage(dmg, dmg_kind);
	if ( temp->GetDurability() > 0 ) {
		if ( block_manager.MakeId(BLOCK, STONE)==temp->GetId() &&
				temp->GetDurability()!=MAX_DURABILITY )
		{ // convert stone into ladder
			temp = NewBlock(LADDER, STONE);
			emit ReEnlighten(x, y, z);
		}
	}
	SetBlock(temp, x, y, z);
}

void World::DestroyAndReplace(const ushort x, const ushort y, const ushort z) {
	Block * const temp = GetBlock(x, y, z);
	if ( temp->GetDurability() > 0 ) {
		return;
	}
	Block * const dropped = temp->DropAfterDamage();
	Shred * const shred = GetShred(x, y);
	const ushort x_in_shred = Shred::CoordInShred(x);
	const ushort y_in_shred = Shred::CoordInShred(y);
	const int old_transparency = temp->Transparent();
	const uchar old_light = temp->LightRadius();
	Block * new_block;
	if ( PILE!=temp->Kind() && (temp->HasInventory() || dropped) ) {
		new_block = ( ( dropped && PILE==dropped->Kind() ) ?
			dropped : NewBlock(PILE, DIFFERENT) );
		Inventory * const inv = temp->HasInventory();
		Inventory * const new_pile_inv = new_block->HasInventory();
		if ( inv ) {
			new_pile_inv->GetAll(inv);
		}
		if ( dropped && PILE!=dropped->Kind() &&
				!new_pile_inv->Get(dropped) )
		{
			DeleteBlock(dropped);
		}
		shred->AddFalling(x_in_shred, y_in_shred, z);
	} else {
		new_block = Normal(AIR);
	}
	shred->SetBlock(new_block, x_in_shred, y_in_shred, z);
	shred->AddFalling(x_in_shred, y_in_shred, z+1);
	if ( old_transparency != INVISIBLE ) {
		ReEnlightenBlockRemove(x, y, z);
	}
	if ( old_light ) {
		RemoveFireLight(x, y, z);
	}
} // void World::DestroyAndReplace(ushort x, y, z)

bool World::Build(Block * block,
		const ushort x, const ushort y, const ushort z,
		const quint8 dir, Block * const who, const bool anyway)
{
	Block * const target_block = GetBlock(x, y, z);
	if ( ENVIRONMENT!=target_block->PushResult(NOWHERE) && !anyway ) {
		if ( who ) {
			who->ReceiveSignal(tr("Cannot build here."));
		}
		return false;
	} // else:
	block->Restore();
	block->SetDir(dir);
	const int old_transparency = target_block->Transparent();
	const int new_transparency = block->Transparent();
	const uchar block_light = block->LightRadius();
	SetBlock(block, x, y, z);
	if ( old_transparency != new_transparency ) {
		ReEnlightenBlockAdd(x, y, z);
	}
	if ( block_light ) {
		AddFireLight(x, y, z, block_light);
	}
	return true;
}

bool World::Inscribe(const ushort x, const ushort y, const ushort z) {
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
		const ushort src, const ushort dest, const ushort num)
{
	Inventory * const inv_from = block_from->HasInventory();
	if ( !inv_from ) {
		block_from->ReceiveSignal(tr("No inventory."));
		return;
	}
	if ( !inv_from->Number(src) ) {
		block_from->ReceiveSignal(tr("Nothing here."));
		block_to  ->ReceiveSignal(tr("Nothing here."));
		return;
	}
	Inventory * const inv_to = block_to->HasInventory();
	if ( !inv_to ) {
		block_from->ReceiveSignal(tr("No room there."));
		return;
	}
	if ( inv_from->Drop(src, dest, num, inv_to) ) {
		block_from->ReceiveSignal(tr("Your bag is lighter now."));
		block_to  ->ReceiveSignal(tr("Your bag is heavier now."));
	}
}

int World::Temperature(const ushort x, const ushort y, const ushort z) const {
	if ( HEIGHT-1 == z ) {
		return 0;
	}
	short temperature = GetBlock(x, y, z)->Temperature();
	if ( temperature ) {
		return temperature;
	}
	for (short i=x-1; i<=x+1; ++i)
	for (short j=y-1; j<=y+1; ++j)
	for (short k=z-1; k<=z+1; ++k) {
		if ( InBounds(i, j, k) ) {
			temperature += GetBlock(i, j, k)->Temperature();
		}
	}
	return temperature/2;
}

void World::RemSun() {
	PutBlock(Normal(ifStar ? STAR : SKY),
		sun_moon_x, SHRED_WIDTH*NumShreds()/2, HEIGHT-1);
}

void World::LoadAllShreds() {
	shreds = new Shred *[(ulong)NumShreds()*NumShreds()];
	shredMemoryPool = static_cast<Shred *>
		(operator new (sizeof(Shred)*NumShreds()*NumShreds()));
	for (long i=latitude -NumShreds()/2, x=0; x<NumShreds(); ++i, ++x)
	for (long j=longitude-NumShreds()/2, y=0; y<NumShreds(); ++j, ++y) {
		shreds[ShredPos(x, y)] = new(shredMemoryPool+ShredPos(x, y))
			Shred( x, y, j, i, (shredMemoryPool+ShredPos(x, y)) );
	}
	MakeSun();
	sunMoonFactor=( NIGHT==PartOfDay() ) ?
		MOON_LIGHT_FACTOR : SUN_LIGHT_FACTOR;
	ReEnlightenAll();
}

void World::DeleteAllShreds() {
	RemSun();
	for (ushort i=0; i<NumShreds()*NumShreds(); ++i) {
		shreds[i]->~Shred();
	}
	operator delete(shredMemoryPool);
	delete [] shreds;
}

void World::SetNumActiveShreds(const ushort num) {
	const QWriteLocker writeLocker(rwLock);
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

World::World(const QString & world_name) :
		timeStep(0),
		worldName(world_name),
		rwLock(new QReadWriteLock()),
		map(new WorldMap(&world_name)),
		toResetDir(UP)
{
	puts(qPrintable(tr("Loading world settings...")));
	world = this;
	QSettings game_settings("freg.ini", QSettings::IniFormat);
	numShreds = game_settings.value("number_of_shreds", MIN_WORLD_SIZE).
		toLongLong();
	if ( 1 != numShreds%2 ) {
		++numShreds;
		fprintf(stderr, "Invalid number of shreds. Set to %hu.\n",
			numShreds);
	}
	if ( numShreds < MIN_WORLD_SIZE ) {
		fprintf(stderr,
			"Number of shreds: to small: %hu. Set to %hu.\n",
			numShreds, MIN_WORLD_SIZE);
		numShreds = MIN_WORLD_SIZE;
	}
	SetNumActiveShreds(game_settings.value("number_of_active_shreds",
		numShreds).toUInt());
	game_settings.setValue("number_of_shreds", numShreds);
	game_settings.setValue("number_of_active_shreds", numActiveShreds);

	QDir::current().mkpath(worldName+"/texts");
	QSettings settings(worldName+"/settings.ini", QSettings::IniFormat);
	time = settings.value("time", END_OF_NIGHT).toLongLong();
	spawnLongi = settings.value( "spawn_longitude",
		int(qrand() % MapSize()) ).toLongLong();
	spawnLati  = settings.value( "spawn_latitude",
		int(qrand() % MapSize()) ).toLongLong();
	settings.setValue("spawn_longitude", qlonglong(spawnLongi));
	settings.setValue("spawn_latitude", qlonglong(spawnLati));
	longitude = settings.value("longitude", int(spawnLongi)).toLongLong();
	latitude  = settings.value("latitude",  int(spawnLati )).toLongLong();

	shredStorage = new ShredStorage(numShreds+2, longitude, latitude);
	puts(qPrintable(tr("Loading world...")));
	LoadAllShreds();
	emit UpdatedAll();
} // World::World(const QString & world_name)

World::~World() { CleanAll(); }

void World::CleanAll() {
	static bool cleaned = false;
	if ( cleaned ) {
		return;
	}
	cleaned = true;

	WriteLock();
	quit();
	wait();
	Unlock();

	DeleteAllShreds();
	delete map;
	delete shredStorage;
	delete rwLock;

	QSettings settings(worldName+"/settings.ini", QSettings::IniFormat);
	settings.setValue("time", qlonglong(time));
	settings.setValue("longitude", qlonglong(longitude));
	settings.setValue("latitude", qlonglong(latitude));
}
