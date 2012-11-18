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
#include <QMutex>
#include <QString>
#include "thread.h"
#include "header.h"

class Screen;
class Block;
class Dwarf;
class Active;
class Shred;

class World {
	unsigned short time_step;
	unsigned long time;
	Shred ** shreds;
	Block * normal_blocks[AIR];
	Dwarf * playerP;
	unsigned short spawnX, spawnY, spawnZ;
	unsigned long longitude, latitude;
	QString worldName;
	unsigned short numShreds;
	QMutex mutex;

	public:
	Block * BlockFromFile(FILE *,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void AddActive(Active *, unsigned short, unsigned short);
	void RemActive(Active *, unsigned short, unsigned short);
	Block * NewNormal(subs sub) { return normal_blocks[sub]; }
	Thread * thread;
	Screen * scr;

	private:
	Block * Block(const unsigned short,
	              const unsigned short,
	              const unsigned short) const;
	void SetBlock(class Block *,
		const unsigned short,
		const unsigned short,
		const unsigned short);

	short LightMap(const unsigned short,
	               const unsigned short,
	               const unsigned short) const;
	void SetLightMap(const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void PlusLightMap(const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short);

	void SaveAllShreds();
	void MakeSky() {
		unsigned short i, j;
		FILE * sky=fopen("sky.txt", "r");
		if ( NULL==sky ) {
			for (i=0; i<shred_width*3; ++i)
			for (j=0; j<shred_width*3; ++j)
				SetBlock(NewNormal( rand()%5 ? SKY : STAR ),
					i, j, height-1);
			return;
		}

		char c=fgetc(sky)-'0';
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
			if ( c ) {
				SetBlock(NewNormal(SKY), i, j, height-1);
				--c;
			} else {
				SetBlock(NewNormal(STAR), i, j, height-1);
				c=fgetc(sky)-'0';
			}
		fclose(sky);
	}
	dirs MakeDir(const unsigned short x_center, const unsigned short y_center, const unsigned short x_target, const unsigned short y_target) const {
		//if (x_center==x_target && y_center==y_target) return HERE;
		if (abs(x_center-x_target)<=1 && abs(y_center-y_target)<=1) return HERE;
		float x=x_target-x_center,
		      y=y_target-y_center;
		if (y<=3*x && y<=-3*x) return NORTH;
		else if (y>-3*x && y<-x/3) return NORTH_EAST;
		else if (y>=-x/3 && y<=x/3) return EAST;
		else if (y>x/3 && y<3*x) return SOUTH_EAST;
		else if (y>=3*x && y>=-3*x) return SOUTH;
		else if (y<-3*x && y>-x/3) return SOUTH_WEST;
		else if (y<=-x/3 && y>=x/3) return WEST;
		else return NORTH_WEST;
	}
	double Distance(const unsigned short x_from, const unsigned short y_from, const unsigned short z_from,
	                const unsigned short x_to,   const unsigned short y_to,   const unsigned short z_to) const {
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
	void ReEnlighten(const unsigned short i, const unsigned short j, const unsigned short k) {
		if ( height-1==k || !InBounds(i, j, k) )
			return;

		short x, y, z;
		for (x=i-max_light_radius-1; x<=i+max_light_radius+1; ++x)
		for (y=j-max_light_radius-1; y<=j+max_light_radius+1; ++y)
		for (z=k-max_light_radius-1; z<=k+max_light_radius+1 && z<height-1; ++z)
			if ( InBounds(x, y, z) )
				SetLightMap(0, x, y, z);

		for (x=i-max_light_radius-max_light_radius-1; x<=i+max_light_radius+max_light_radius+1; ++x)
		for (y=j-max_light_radius-max_light_radius-1; y<=j+max_light_radius+max_light_radius+1; ++y) {
			if ( InBounds(x, y, 0) )
				SunShine(x, y);
			for (z=k-max_light_radius-max_light_radius-1; z<=k+max_light_radius+max_light_radius+1; ++z)
				Shine(x, y, z);
		}
	}

	void ReEnlightenAll() {
		unsigned short i, j, k;

		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
		for (k=0; k<height-1; ++k)
			SetLightMap(0, i, j, k);

		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j) {
			SunShine(i, j);
			for (k=1; k<height-1; ++k)
				Shine(i, j, k);
		}
	}
	
	void SafeEnlighten(const unsigned short i, const unsigned short j, const unsigned short k) {
		if ( InBounds(i, j, k) && k<height-1 )
			SetLightMap(10, i, j, k);
	}

	void SunShine(const unsigned short i, const unsigned short j) {
		if ( NIGHT==PartOfDay() )
			return;

		unsigned short k=height-2;
		do {
			SetLightMap(10, i, j, k);
			SafeEnlighten(i+1, j, k);
			SafeEnlighten(i-1, j, k);
			SafeEnlighten(i, j+1, k);
			SafeEnlighten(i, j-1, k);
		} while ( Transparent(i, j, k--) );
	}

	void Shine(
			const unsigned short,
			const unsigned short,
			const unsigned short);

	//information section
	public:
	int Focus(const unsigned short,
	          const unsigned short,
	          const unsigned short,
	          unsigned short &,
	          unsigned short &,
	          unsigned short &, const dirs) const;
	int Focus(const unsigned short,
	          const unsigned short,
	          const unsigned short,
	          unsigned short &,
	          unsigned short &,
	          unsigned short &) const;
	void PlayerFocus(unsigned short & i_target, unsigned short & j_target, unsigned short & k_target) const {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Focus(playerX, playerY, playerZ, i_target, j_target, k_target);
	}
	dirs TurnRight(const dirs dir) const {
		switch (dir) {
			case NORTH: return EAST;
			case EAST: return SOUTH;
			case SOUTH: return WEST;
			default: return NORTH;
		}
	}
	dirs TurnLeft(const dirs dir) const {
		switch (dir) {
			case NORTH: return WEST;
			case WEST: return SOUTH;
			case SOUTH: return EAST;
			default: return NORTH;
		}
	}
	void Examine() const;
	char CharNumber(const unsigned short, const unsigned short, const unsigned short) const;
	char CharNumberFront(const unsigned short, const unsigned short) const;

	//visibility section
	public:
	bool DirectlyVisible(const float, const float, const float, const unsigned short, const unsigned short, const unsigned short) const;
	bool Visible(const unsigned short, const unsigned short, const unsigned short,
	             const unsigned short, const unsigned short, const unsigned short) const;
	bool Visible(const unsigned short x_to, const unsigned short y_to, const unsigned short z_to) const {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		return Visible(playerX, playerY, playerZ, x_to, y_to, z_to);
	}

	//movement section
	public:
	void ReloadShreds(const dirs);	
	void PhysEvents();
	int  Move(
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const dirs,
			unsigned=2); //how much block fall/rise at one turn
	void Jump(
			const unsigned short,
			const unsigned short,
			const unsigned short);
	int PlayerMove(const dirs dir);
	int PlayerMove();
	void PlayerJump() {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Jump(playerX, playerY, playerZ);
	}

	//player specific functions section
	public:
	void SetPlayerDir(const dirs);
	dirs GetPlayerDir() const;
	void GetPlayerCoords(
			unsigned short &,
			unsigned short &,
			unsigned short &) const;
	void GetPlayerCoords(
			unsigned short &,
			unsigned short &) const;
	void GetPlayerZ(unsigned short &) const;
	Dwarf * GetPlayerP() const { return playerP; }

	//time section
	public:
	unsigned long GetTime() const { return time; }
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
	void Damage(
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned short=1,
			const damage_kinds=CRUSH,
			const bool=true);
	void Use(
			const unsigned short,
			const unsigned short,
			const unsigned short);
	bool Build(class Block *,
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void PlayerBuild(const unsigned short);
	void Inscribe(Dwarf * const);
	void PlayerInscribe() { Inscribe(playerP); }
	void Inscribe(
			const unsigned short,
			const unsigned short,
			const unsigned short);
	void Eat(class Block *, class Block *);
	void Eat(const unsigned short i,
	         const unsigned short j,
	         const unsigned short k,
	         const unsigned short i_food,
	         const unsigned short j_food,
	         const unsigned short k_food) {
		if ( !InBounds(i, j, k) || !InBounds(i_food, j_food, k_food) )
			return;
		Eat(Block(i, j, k), Block(i_food, j_food, k_food));
	}
	void PlayerEat(unsigned short);

	//inventory functions section
	private:
	void Exchange(
			const unsigned short i_from,
			const unsigned short j_from,
			const unsigned short k_from,
			const unsigned short i_to,
			const unsigned short j_to,
			const unsigned short k_to,
			const unsigned short n);
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
	void PlayerDrop(const unsigned short n) {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Drop(playerX, playerY, playerZ, n);
	}
	void Get(const unsigned short i, const unsigned short j, const unsigned short k, const unsigned short n) {
		unsigned short i_from, j_from, k_from;
		if ( !Focus(i, j, k, i_from, j_from, k_from) )
			Exchange(i_from, j_from, k_from, i, j, k, n);
	}
	void PlayerGet(const unsigned short n) {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Get(playerX, playerY, playerZ, n);
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
	void PlayerWield() {
		/*unsigned short i, j=0;
		playerP->RangeForWield(i, j);

		if ( i>=inventory_size ) {
			scr->Notify("Nothing to wield.");
			return;
		}

		char str[18];
		sprintf(str, "Wield what? (%c-%c)", i+'a', j+'a');
		scr->Notify(str);
		//Wield(playerP, getch()-'a');
		scr->Notify("");*/
	}

	//block information section
	public:
	bool InBounds(
			const unsigned short i,
			const unsigned short j,
			const unsigned short k) const
		{ return (i<shred_width*3 && j<shred_width*3 && k<height); }
	char * FullName(char * const,
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	subs Sub(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	kinds Kind(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	short Durability(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	int Transparent(
			const unsigned short i,
			const unsigned short j,
			const unsigned short k) const
	{
		return ( !InBounds(i, j, k) ) ?
			2 :
			TransparentNotSafe(i, j, k);
	}

	int TransparentNotSafe(
			const unsigned short,
			const unsigned short,
			const unsigned short k) const;

	int Movable(const class Block * const) const;
	double Weight(const class Block * const) const;
	double Weight(const unsigned short i, const unsigned short j, const unsigned short k) const
		{ return (!InBounds(i, j, k)) ? 1000 : Weight(Block(i, j, k)); }

	void * HasInventory(
			const unsigned short,
			const unsigned short,
			const unsigned short k) const;	

	void * ActiveBlock(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	bool GetNote(char * const,
			const unsigned short,
			const unsigned short,
			const unsigned short k) const;
	int Temperature(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;
	bool Equal(
			const class Block * const,
			const class Block * const) const;
	short Enlightened(const unsigned short i, const unsigned short j, const unsigned short k) const {
		if ( !InBounds(i, j, k) || NULL==Block(i, j, k) )
			return 0;
		if ( height-1==k )
			return 10;
		return LightMap(i, j, k);
	}
	char MakeSound(
			const unsigned short,
			const unsigned short,
		       	const unsigned short) const;

	private:
	float LightRadius(
			const unsigned short,
			const unsigned short,
			const unsigned short) const;

	bool UnderTheSky(const unsigned short i, const unsigned short j, unsigned short k) const {
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
	void mutex_lock()    { mutex.lock(); }
	bool mutex_trylock() { return !mutex.tryLock(); }
	void mutex_unlock()  { mutex.unlock(); }

	public:
	World(QString);
	World(QString,
			const unsigned long,
			const unsigned long,
			const unsigned short,
			const unsigned short,
			const unsigned short,
			const unsigned long,
			const unsigned short);
	~World();
};

#endif
