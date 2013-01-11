	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#ifndef WORLD_H
#define WORLD_H

#include <cmath>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QThread>
#include "header.h"

class Block;
class Dwarf;
class Inventory;
class Active;
class Shred;

class World : public QThread {
	Q_OBJECT

	unsigned long time;
	Shred ** shreds;
	Block * normal_blocks[AIR];
	ulong longitude, latitude; //center of active zone
	unsigned long spawnLongi, spawnLati;
	QString worldName;
	unsigned short numShreds; //size of active zone
	QReadWriteLock rwLock;

	bool cleaned;
	unsigned short sun_moon_x;
	bool if_star;

	unsigned long mapSize;

	protected:
	void run();

	public:
	ulong MapSize() const { return mapSize; }
	dirs TurnRight(const dirs) const;
	dirs TurnLeft(const dirs) const;
	unsigned long GetSpawnLongi() const { return spawnLongi; }
	unsigned long GetSpawnLati()  const { return spawnLati; }

	unsigned short NumShreds() const { return numShreds; }

	Block * NewNormal(const subs & sub) const {
		return normal_blocks[sub];
	}
	
	Block * GetBlock(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	Shred * GetShred(
			const unsigned short i,
			const unsigned short j) const
	{
		return shreds[j/shred_width*numShreds+
		              i/shred_width];
	}	
	void SetBlock(Block *,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);

	private:
	unsigned short SunMoonX() const {
		return ( NIGHT==PartOfDay() ) ?
			TimeOfDay()*shred_width*numShreds/
				seconds_in_night :
			(TimeOfDay()-seconds_in_night)*shred_width*numShreds/
				seconds_in_daylight;
	}
	short LightMap(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	void SetLightMap(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);
	void PlusLightMap(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);

	void MakeSky();
	int MakeDir(
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	double Distance(
			const unsigned short x_from,
			const unsigned short y_from,
			const unsigned short z_from,
	                const unsigned short x_to,
			const unsigned short y_to,
			const unsigned short z_to) const
	{
		return sqrt( float((x_from-x_to)*(x_from-x_to)+
		                   (y_from-y_to)*(y_from-y_to)+
		                   (z_from-z_to)*(z_from-z_to)) );
	}
	dirs Anti(const dirs dir) const {
		switch (dir) {
			case NORTH: return SOUTH;
			case NORTH_EAST: return SOUTH_WEST;
			case EAST: return WEST;
			case SOUTH_EAST: return NORTH_WEST;
			case SOUTH: return NORTH;
			case SOUTH_WEST: return NORTH_EAST;
			case WEST: return EAST;
			case NORTH_WEST: return SOUTH_EAST;
			case UP: return DOWN;
			case DOWN: return UP;
			default:
				fprintf(stderr, "World::Anti(dirs): unlisted dir: %d\n", (int)dir);
				return HERE;
		}
	}

	//lighting section
	private:
	void ReEnlighten(
			const ushort i,
			const ushort j,
			const ushort k);
	void ReEnlightenAll();
	void SafeEnlighten(
			const ushort i,
			const ushort j,
			const ushort k) {
		if ( InBounds(i, j, k) && k<height-1 )
			SetLightMap(10, i, j, k);
	}

	void SunShine(
			const ushort i,
			const ushort j)
	{
		if ( NIGHT==PartOfDay() )
			return;

		ushort k=height-2;
		do {
			SetLightMap(10, i, j, k);
			SafeEnlighten(i+1, j, k);
			SafeEnlighten(i-1, j, k);
			SafeEnlighten(i, j+1, k);
			SafeEnlighten(i, j-1, k);
		} while ( Transparent(i, j, k--) );
	}

	void Shine(
			const ushort,
			const ushort,
			const ushort);

	//information section
	public:
	QString WorldName(QString str) {
		return str=worldName;
	}
	//TODO: make one Focus
	int Focus(const unsigned short &,
	          const unsigned short &,
	          const unsigned short &,
	          unsigned short &,
	          unsigned short &,
	          unsigned short &,
		  const dirs &) const;
	int Focus(const unsigned short &,
	          const unsigned short &,
	          const unsigned short &,
	          unsigned short &,
	          unsigned short &,
	          unsigned short &) const;

	//visibility section
	public:
	bool DirectlyVisible(
			float,
			float,
			float,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	bool Visible(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;

	//movement section
	public:
	int  Move(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const dirs &,
			const unsigned=2); //how much block fall/rise at one turn
	void Jump(
			const unsigned short &,
			const unsigned short &,
			unsigned short);


	//time section
	public:
	times_of_day PartOfDay() const {
		unsigned short time_day=TimeOfDay();
		if (time_day<end_of_night)   return NIGHT;
		if (time_day<end_of_morning) return MORNING;
		if (time_day<end_of_noon)    return NOON;
		return EVENING;
	}
	int TimeOfDay() const { return time%seconds_in_day; }
	unsigned long Time() const { return time; }

	//interactions section
	public:
	bool Damage(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &,
			const unsigned short=1,
			const damage_kinds=CRUSH,
			const bool=true);
	bool Use(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);
	bool Build(Block *,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);
	bool Inscribe(Dwarf * const);
	void Inscribe(
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void Eat(Block *, Block *);
	void Eat(const unsigned short i,
	         const unsigned short j,
	         const unsigned short k,
	         const unsigned short i_food,
	         const unsigned short j_food,
	         const unsigned short k_food) {
		if ( !InBounds(i, j, k) || !InBounds(i_food, j_food, k_food) )
			return;
		Eat(GetBlock(i, j, k), GetBlock(i_food, j_food, k_food));
	}

	//inventory functions section
	private:
	int Exchange(
			const unsigned short & i_from,
			const unsigned short & j_from,
			const unsigned short & k_from,
			const unsigned short & i_to,
			const unsigned short & j_to,
			const unsigned short & k_to,
			const unsigned short & n);
	void ExchangeAll(
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	public:
	void Drop(const unsigned short i, const unsigned short j, const unsigned short k, const unsigned short n) {
		unsigned short i_to, j_to, k_to;
		if ( !Focus(i, j, k, i_to, j_to, k_to) )
			Exchange(i, j, k, i_to, j_to, k_to, n);
	}
	void Get(const unsigned short i, const unsigned short j, const unsigned short k, const unsigned short n) {
		unsigned short i_from, j_from, k_from;
		if ( !Focus(i, j, k, i_from, j_from, k_from) )
			Exchange(i_from, j_from, k_from, i, j, k, n);
	}
	void DropAll(const unsigned short i_from, const unsigned short j_from, const unsigned short k_from) {
		unsigned short i, j, k;
		if ( !Focus(i_from, j_from, k_from, i, j, k) )
			ExchangeAll(i_from, j_from, k_from, i, j, k);
	}
	void GetAll(const unsigned short i_to, const unsigned short j_to, const unsigned short k_to) {
		unsigned short i, j, k;
		if ( !Focus(i_to, j_to, k_to, i, j, k) )
			ExchangeAll(i, j, k, i_to, j_to, k_to);
	}
	void Wield(Dwarf * const, const unsigned short);

	//block information section
	public:
	bool InBounds(
			const unsigned short i,
			const unsigned short j,
			const unsigned short k) const
	{
		static const unsigned short max_x_y=shred_width*numShreds;
		return (i<max_x_y && j<max_x_y && k<height);
	}
	QString FullName(QString,
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	int Transparent(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Durability(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Kind(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Sub(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Movable(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	double Weight(
			const unsigned short & i,
			const unsigned short & j,
			const unsigned short & k) const;
	Inventory * HasInventory(
			const unsigned short,
			const unsigned short,
			const unsigned short k) const;	

	Active * ActiveBlock(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	bool GetNote(QString,
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	int Temperature(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &) const;
	bool Equal(const Block * const, const Block * const) const;
	short Enlightened(
			const unsigned short & i,
			const unsigned short & j,
			const unsigned short & k) const
	{
		return InBounds(i, j, k) ?
			(( height-1==k ) ?
				10 :
				LightMap(i, j, k)) :
			0;
	}
	char MakeSound(
			const unsigned short &,
			const unsigned short &,
		       	const unsigned short &) const;

	private:
	float LightRadius(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	bool UnderTheSky(
			const unsigned short & i,
			const unsigned short & j,
			unsigned short k) const
	{
		if ( !InBounds(i, j, k) )
			return false;

		for (++k; k<height-1; ++k)
			if ( !Transparent(i, j, k) )
				return false;
		return true;
	}

	private:
	friend class Active;

	public:
	void WriteLock() { rwLock.lockForWrite(); }
	void ReadLock() { rwLock.lockForRead(); }
	bool TryReadLock() { return rwLock.tryLockForRead(); }
	void Unlock() { rwLock.unlock(); }

	public:
	World(const QString, const unsigned short);
	~World();

	public slots:
	void CleanAll();
	void ReloadShreds(const int &);	
	void PhysEvents();

	signals:
	void Notify(QString) const;
	void GetString(QString &) const;
	void Updated(
			const unsigned short &,
			const unsigned short &,
			const unsigned short &);
	void UpdatedAll();
};

#endif
