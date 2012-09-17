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

#include <pthread.h>
#include <cmath>
#include "screen.h"

class Block;
class Dwarf;
class Active;

void *PhysThread(void *vptr_args);
class World {
	unsigned short time_step;
	unsigned long time;
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	unsigned short spawnX, spawnY, spawnZ;
	long longitude, latitude;
	pthread_t eventsThread;
	Active * activeList;
	char worldName[20];
	unsigned short worldSize;

	void LoadShred(const long, const long, const unsigned short, const unsigned short);
	void SaveShred(const long, const long, const unsigned short, const unsigned short);
	void ReloadShreds(const dirs);
	void LoadAllShreds();
	void SaveAllShreds();
	void MakeSky() {
		unsigned short i, j;
		FILE * sky=fopen("sky.txt", "r");
		if (NULL==sky) {
			for (i=0; i<shred_width*3; ++i)
			for (j=0; j<shred_width*3; ++j)
				blocks[i][j][height-1]=new Block( random()%5 ? SKY : STAR );
		} else {
			char c=fgetc(sky)-'0';
			for (i=0; i<shred_width*3; ++i)
			for (j=0; j<shred_width*3; ++j)
				if (c) {
					blocks[i][j][height-1]=new Block(SKY);
					--c;
				} else {
					blocks[i][j][height-1]=new Block(STAR);
					c=fgetc(sky)-'0';
				}
			fclose(sky);
		}
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
			blocks[i][j][height-1]->enlightened=1; //sky is always enlightened
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
		return sqrt( (x_from-x_to)*(x_from-x_to)+
		             (y_from-y_to)*(y_from-y_to)+
		             (z_from-z_to)*(z_from-z_to) );
	}
	void FileName(char * const str, const long longi, const long lati) const { sprintf(str, "shreds/%ld_%ld", longi, lati); }

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

	char TypeOfShred(const long longi, const long lati) const {
		FILE * map=fopen(worldName, "r");
		if (NULL==map)
			return '.';

		char find_start[3];
		do fgets(find_start, 3, map);
		while ('#'!=find_start[0] || '#'!=find_start[1]);

		fseek(map, (worldSize+1)*longi+lati-2, SEEK_CUR);
		char c=fgetc(map);
		fclose(map);
		return c;
	}

	//lighting section
	void ReEnlighten(const int i, const int j, const int k) {
		if ( Transparent(i, j, k) )
			return;

		short x, y, z;
		for (x=i-max_light_radius-1; x<=i+max_light_radius+1; ++x)
		for (y=j-max_light_radius-1; y<=j+max_light_radius+1; ++y)
		for (z=k-max_light_radius-1; z<=k+max_light_radius+1; ++z)
			if ( InBounds(x, y, z) && NULL!=blocks[x][y][z] && z<height-1)
				blocks[x][y][z]->enlightened=0;

		for (x=i-max_light_radius-max_light_radius-1; x<=i+max_light_radius+max_light_radius+1; ++x)
		for (y=j-max_light_radius-max_light_radius-1; y<=j+max_light_radius+max_light_radius+1; ++y)
		for (z=k-max_light_radius-max_light_radius-1; z<=k+max_light_radius+max_light_radius+1; ++z)
			Shine(x, y, z);

		for (x=i-max_light_radius-1; x<=i+max_light_radius+1; ++x)
		for (y=j-max_light_radius-1; y<=j+max_light_radius+1; ++y)
			if ( InBounds(x, y, 0) )
				SunReShine(x, y);
	}

	void DirectReEnlighten(const int i, const int j, const int k) {
		if ( NIGHT!=PartOfDay() && UnderTheSky(i, j, k) ) {
			blocks[i][j][k]->enlightened=1;
			return;
		}

		short x, y, z;
		for (x=i-max_light_radius; x<=i+max_light_radius; ++x)
		for (y=j-max_light_radius; y<=j+max_light_radius; ++y)
		for (z=k-max_light_radius; z<=k+max_light_radius; ++z)
			if ( InBounds(x, y, z) && !(x==i && y==j && z==k) &&
					( Distance(i, j, k, x, y, z)<=LightRadius(x, y, z) ) &&
					DirectlyVisible(i, j, k, x, y, z) ) {
				blocks[i][j][k]->enlightened=1;
				return;
			}
		blocks[i][j][k]->enlightened=0;
	}

	void ReEnlightenAll() {
		unsigned short i, j, k;

		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
		for (k=0; k<height-1; ++k)
			if (NULL!=blocks[i][j][k]) blocks[i][j][k]->enlightened=0;

		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
		for (k=0; k<height-1; ++k)
			Shine(i, j, k);

		if ( NIGHT!=PartOfDay() )
			for (i=0; i<shred_width*3; ++i)
			for (j=0; j<shred_width*3; ++j)
				SunShine(i, j);
	}

	void SafeEnlighten(const int i, const int j, const int k) {
		if ( InBounds(i, j, k) && NULL!=blocks[i][j][k])
			blocks[i][j][k]->enlightened=1;
	}

	void SunShine(const int i, const int j) {
		unsigned short k=height-2;
		do {
			if ( NULL!=blocks[i][j][k] )
				blocks[i][j][k]->enlightened=1;
			SafeEnlighten(i+1, j, k);
			SafeEnlighten(i-1, j, k);
			SafeEnlighten(i, j+1, k);
			SafeEnlighten(i, j-1, k);
		} while ( Transparent(i, j, k--) );
	}

	void SunReShine(const int i, const int j) { 
		if ( NIGHT==PartOfDay() )
			return;

		for (unsigned short k=height-2; Transparent(i, j, k); --k)
			if ( NULL!=blocks[i][j][k] )
				blocks[i][j][k]->enlightened=0;
		SunShine(i, j);
	}

	void Shine(const int i, const int j, const int k) {
		float light_radius;
		if ( !InBounds(i, j, k) || NULL==blocks[i][j][k] || 0==(light_radius=blocks[i][j][k]->LightRadius()) )
			return;

		for (short x=ceil(i-light_radius); x<=floor(i+light_radius); ++x)
		for (short y=ceil(j-light_radius); y<=floor(j+light_radius); ++y)
		for (short z=ceil(k-light_radius); z<=floor(k+light_radius); ++z)
			if (InBounds(x, y, z) &&
					NULL!=blocks[x][y][z] &&
					Distance(i, j, k, x, y, z)<=light_radius &&
					DirectlyVisible(i, j, k, x, y, z))
				blocks[x][y][z]->enlightened=1;
	}

	//shred generators section
	//this functions fill space between the lowest nullstone layer and sky. so use k from 1 to heigth-2.
	//unfilled blocks are air.

	void NormalUnderground(const unsigned short istart, const unsigned short jstart, const unsigned short depth=0) {
		for (unsigned short i=istart; i<istart+shred_width; ++i)
		for (unsigned short j=jstart; j<jstart+shred_width; ++j) {
			unsigned short k;
			for (k=1; k<height/2-6 && k<height/2-depth-1; ++k)
				blocks[i][j][k]=new Block;
			blocks[i][j][k++]=new Block((random()%2) ? STONE : SOIL);
			for (; k<height/2-depth; ++k)
				blocks[i][j][k]=new Block(SOIL);
		}
	}

	void PlantGrass(const unsigned short istart, const unsigned short jstart) {
		for (unsigned short i=istart; i<istart+shred_width; ++i)	
		for (unsigned short j=jstart; j<jstart+shred_width; ++j) {
			unsigned short k;
			for (k=height-2; NULL==blocks[i][j][k]; --k);
			if ( SOIL==Sub(i, j, k++) )
				blocks[i][j][k]=new Grass(this, i, j, k);
		}
	}

	void NullMountain(const unsigned short istart, const unsigned short jstart) {
		unsigned short i, j, k;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j) {
			for (k=1; k<height/2; ++k)
				if (i==istart+4 || i==istart+5 || j==jstart+4 || j==jstart+5)
					blocks[i][j][k]=new Block(NULLSTONE);
				else
					blocks[i][j][k]=new Block;

			for ( ; k<height-1; ++k)
				if (i==istart+4 || i==istart+5 || j==jstart+4 || j==jstart+5)
					blocks[i][j][k]=new Block(NULLSTONE);
		}
	}

	void Plain(const unsigned short istart, const unsigned short jstart) {
		NormalUnderground(istart, jstart);
		unsigned short i;

		//bush
		short rand=random()%2;
		for (i=0; i<rand; ++i) {
			short x=istart+random()%shred_width,
			      y=jstart+random()%shred_width;
			if (NULL!=blocks[x][y][height/2])
				delete blocks[x][y][height/2];
			blocks[x][y][height/2]=new Bush(this, x, y, height/2);
		}

		//rabbits
		rand=random()%2;
		for (i=0; i<rand; ++i) {
			short x=istart+random()%shred_width,
			      y=jstart+random()%shred_width;
			if (NULL!=blocks[x][y][height/2])
				delete blocks[x][y][height/2];
			blocks[x][y][height/2]=new Rabbit(this, x, y, height/2);
		}

		PlantGrass(istart, jstart);
	}

	void Forest(const unsigned short istart, const unsigned short jstart, const long longi, const long lati) {
		NormalUnderground(istart, jstart);
		long i, j;
		unsigned short number_of_trees=0;
		for (i=longi-1; i<=longi+1; ++i)
		for (j=lati-1;  j<=lati+1;  ++j)
			if ( 't'==TypeOfShred(i, j) )
				++number_of_trees;

		for (i=0; i<number_of_trees; ++i) {
			short x=istart+random()%(shred_width-2),
			      y=jstart+random()%(shred_width-2);
			Tree(x, y, height/2, 4+random()%5);
		}

		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j)
			if (NULL==blocks[i][j][height/2])
				blocks[i][j][height/2]=new Grass(this, i, j, height/2);
	}

	void Water(const unsigned short istart, const unsigned short jstart, const long longi, const long lati) {
		unsigned short depth=1;
		char map[3][3];
		for (long i=longi-1; i<=longi+1; ++i)
		for (long j=lati-1;  j<=lati+1;  ++j)
			if ( '~'==(map[i-longi+1][j-lati+1]=TypeOfShred(i, j)) )
				++depth;

		NormalUnderground(istart, jstart, depth);

		if ('~'!=map[1][0] && '~'!=map[0][1]) { //north-west rounding
			for (unsigned short i=istart; i<istart+shred_width/2; ++i)	
			for (unsigned short j=jstart; j<jstart+shred_width/2; ++j)
				for (unsigned short k=height/2-depth; k<height/2; ++k)
					if (((istart+4-i)*(istart+4-i)+
					     (jstart+4-j)*(jstart+4-j)+
					     (height/2-k)*(height/2-k)*16/depth/depth)>16)
						blocks[i][j][k]=new Block(SOIL);
		}
		if ('~'!=map[1][0] && '~'!=map[2][1]) { //south-west rounding
			for (unsigned short i=istart; i<istart+shred_width/2; ++i)	
			for (unsigned short j=jstart+shred_width/2; j<jstart+shred_width; ++j)
				for (unsigned short k=height/2-depth; k<height/2; ++k)
					if (((istart+4-i)*(istart+4-i)+
					     (jstart+5-j)*(jstart+5-j)+
					     (height/2-k)*(height/2-k)*16/depth/depth)>16)
						blocks[i][j][k]=new Block(SOIL);
		}
		if ('~'!=map[2][1] && '~'!=map[1][2]) { //south-east rounding
			for (unsigned short i=istart+shred_width/2; i<istart+shred_width; ++i)	
			for (unsigned short j=jstart+shred_width/2; j<jstart+shred_width; ++j)
				for (unsigned short k=height/2-depth; k<height/2; ++k)
					if (((istart+5-i)*(istart+5-i)+
					     (jstart+5-j)*(jstart+5-j)+
					     (height/2-k)*(height/2-k)*16/depth/depth)>16)
						blocks[i][j][k]=new Block(SOIL);
		}
		if ('~'!=map[1][2] && '~'!=map[0][1]) { //north-east rounding
			for (unsigned short i=istart+shred_width/2; i<istart+shred_width; ++i)	
			for (unsigned short j=jstart; j<jstart+shred_width/2; ++j)
				for (unsigned short k=height/2-depth; k<height/2; ++k)
					if (((istart+5-i)*(istart+5-i)+
					     (jstart+4-j)*(jstart+4-j)+
					     (height/2-k)*(height/2-k)*16/depth/depth)>16)
						blocks[i][j][k]=new Block(SOIL);
		}
		for (unsigned short i=istart; i<istart+shred_width; ++i)	
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
		//for (unsigned short k=height/2-depth; k<height/2; ++k)
			if (NULL==blocks[i][j][height/2-depth])
				blocks[i][j][height/2-depth]=new Liquid(this, i, j, height/2-depth);

		PlantGrass(istart, jstart);
	}

	void Hill(const unsigned short istart, const unsigned short jstart, const long longi, const long lati) {
		unsigned short hill_height=1;
		for (long i=longi-1; i<=longi+1; ++i)
		for (long j=lati-1;  j<=lati+1;  ++j)
			if ( '+'==TypeOfShred(i, j) )
				++hill_height;

		NormalUnderground(istart, jstart);

		for (unsigned short i=istart; i<istart+shred_width; ++i)	
		for (unsigned short j=jstart; j<jstart+shred_width; ++j)
		for (unsigned short k=height/2; k<height/2+hill_height; ++k)
			if (((istart+4.5-i)*(istart+4.5-i)+
			     (jstart+4.5-j)*(jstart+4.5-j)+
			     (height/2-0.5-k)*(height/2-0.5-k)*16/hill_height/hill_height)<=16)
				blocks[i][j][k]=new Block(SOIL);
		
		PlantGrass(istart, jstart);
	}

	//block combinations section (trees, buildings, etc)
	bool Tree(const unsigned short x, const unsigned short y, const unsigned short z, const unsigned short height) {
		if (0>x || shred_width*3<=x+2 ||
		    0>y || shred_width*3<=y+2 ||
		    0>z || ::height-1<=z+height ||
		    height<2) return false;
		unsigned short i, j, k;
		for (i=x; i<=x+2; ++i)
		for (j=y; j<=y+2; ++j)
		for (k=z; k<z+height; ++k)
			if (NULL!=blocks[i][j][k])
				return false;

		for (k=z; k<z+height-1; ++k) //trunk
			blocks[x+1][y+1][k]=new Block(WOOD);

		for (i=x; i<=x+2; ++i) //leaves
		for (j=y; j<=y+2; ++j)
		for (k=z+height/2; k<z+height; ++k)
			if (NULL==blocks[i][j][k])
				blocks[i][j][k]=new Block(GREENERY);
		
		return true;
	}

	public:
	pthread_mutex_t mutex;
	Screen * scr;
	struct {
		char ch;
		unsigned short lev;
		color_pairs col;
	} soundMap[9];
	private:
	char MakeSound(const int i, const int j, const int k) const { return (NULL==blocks[i][j][k]) ? ' ' : blocks[i][j][k]->MakeSound(); }

	private:
	char CharNumber(const int, const int, const int) const;
	char CharNumberFront(const int, const int) const;

	//information section
	public:
	int Focus(const unsigned short, const unsigned short, const unsigned short, unsigned short &, unsigned short &, unsigned short &, const dirs) const;
	int Focus(const unsigned short i, const unsigned short j, const unsigned short k,
			unsigned short & i_target, unsigned short & j_target, unsigned short & k_target) const {
		return Focus( i, j, k, i_target, j_target, k_target, blocks[i][j][k]->GetDir() );
	}
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

	//visibility section
	bool DirectlyVisible(const int, const int, const int, const int, const int, const int) const;
	bool Visible(const int, const int, const int, const int, const int, const int) const;
	bool Visible(const int x_to, const int y_to, const int z_to) const {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		return Visible(playerX, playerY, playerZ, x_to, y_to, z_to);
	}

	//movement section
	void PhysEvents();
	int  Move(const unsigned short, const unsigned short, const unsigned short, const dirs, unsigned=2); //last arg is how much block fall/rise at one turn
	void Jump(const int, const int, const int);
	int PlayerMove(const dirs dir) {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		scr->viewLeft=NORMAL;
		return Move( playerX, playerY, playerZ, dir );
	}
	int PlayerMove() { return PlayerMove( playerP->GetDir() ); } 
	void PlayerJump() {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Jump(playerX, playerY, playerZ);
	}

	//player specific functions section
	void SetPlayerDir(const dirs dir) { playerP->SetDir(dir); }
	dirs GetPlayerDir() const { return playerP->GetDir(); }
	void GetPlayerCoords(unsigned short & x, unsigned short & y, unsigned short & z) const { playerP->GetSelfXYZ(x, y, z); }
	void GetPlayerCoords(unsigned short & x, unsigned short & y) const { playerP->GetSelfXY(x, y); }
	void GetPlayerZ(unsigned short & z) const { playerP->GetSelfZ(z); }
	Dwarf * GetPlayerP() const { return playerP; }

	//time section
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
	void Damage(const int i, const int j, const int k) {
		if ( NULL==blocks[i][j][k] || 0<blocks[i][j][k]->Damage() )
			return;

		Block * temp=blocks[i][j][k];
		if (temp==scr->blockToPrintLeft)
			scr->viewLeft=NORMAL;
		if (temp==scr->blockToPrintRight)
			scr->viewRight=NORMAL;
		
		Block * dropped=temp->DropAfterDamage();
		if ( PILE!=temp->Kind() && (temp->HasInventory() || NULL!=dropped) ) {
			Pile * new_pile=new Pile(this, i, j, k);
			blocks[i][j][k]=new_pile;
			if ( temp->HasInventory() )
				new_pile->GetAll(temp);
			if ( !(new_pile->Get(dropped)) )
				delete dropped;
		} else
			blocks[i][j][k]=NULL;

		delete temp;

		ReEnlighten(i, j, k);
		SunReShine(i, j);
	}
	void Use(const int i, const int j, const int k) {
		if (NULL!=blocks[i][j][k])
			switch ( blocks[i][j][k]->Use() ) {
				case OPEN:
					if (INVENTORY!=scr->viewLeft) {
						scr->viewLeft=INVENTORY;
						scr->blockToPrintLeft=blocks[i][j][k];
					} else
						scr->viewLeft=NORMAL;
				break;
				default: scr->viewLeft=NORMAL;
			}
	}
	bool Build(Block * block, const int i, const int j, const int k) {
		if (NULL==blocks[i][j][k] && block->CanBeOut() ) {
			block->Restore();
			if ( block->ActiveBlock() )
				((Active *)block)->Register(this, i, j, k);
			blocks[i][j][k]=block;
			if ( block->Transparent() )
				DirectReEnlighten(i, j, k);
			else
				ReEnlighten(i, j, k);
			return true;
		}
		scr->Notify("You can not build.");
		return false;
	}
	void PlayerBuild(const int n) {
		Block * temp=playerP->Drop(n);
		unsigned short i, j, k;
		PlayerFocus(i, j, k);
		if ( !Build(temp, i, j, k) )
			playerP->Get(temp);
	}
	void Inscribe(Dwarf * const dwarf) {
		if (!dwarf->CarvingWeapon()) {
			scr->Notify("You need some tool for inscribing!\n");
			return;
		}
		unsigned short i, j, k;
		dwarf->GetSelfXYZ(i, j, k);
		unsigned short i_to, j_to, k_to;
		if ( !Focus(i, j, k, i_to, j_to, k_to) ) {
			char str[note_length];
			scr->GetString(str);
			Inscribe(i_to, j_to, k_to, str);
		}
	}
	void PlayerInscribe() { Inscribe(playerP); }
	void Inscribe(const int i, const int j, const int k, char * const str) { if (NULL!=blocks[i][j][k]) blocks[i][j][k]->Inscribe(str); }

	//inventory functions section
	private:
	void Exchange(const int i_from, const int j_from, const int k_from,
	              const int i_to,   const int j_to,   const int k_to, const int n) {
		if ( pthread_mutex_trylock(&mutex) ) return;

		Inventory * inv_from=(Inventory *)( HasInventory(i_from, j_from, k_from) );
		Inventory * inv_to=(Inventory *)( HasInventory(i_to, j_to, k_to) );

		if (NULL!=inv_from) {
			if (NULL!=inv_to) {
				Block * temp=inv_from->Drop(n);
				if ( NULL!=temp && !inv_to->Get(temp) ) {
					inv_from->Get(temp);
					scr->Notify("Not enough room\n");
				}
			} else if (NULL==blocks[i_to][j_to][k_to]) {
				Pile * newpile=new Pile(this, i_to, j_to, k_to);
				newpile->Get( inv_from->Drop(n) );
				blocks[i_to][j_to][k_to]=newpile;
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	void ExchangeAll(const int i_from, const int j_from, const int k_from,
	                 const int i_to,   const int j_to,   const int k_to) {
		if (NULL!=blocks[i_from][j_from][k_from] && NULL!=blocks[i_to][j_to][k_to]) {
			Inventory * to=(Inventory *)(blocks[i_to][j_to][k_to]->HasInventory());
			if (NULL!=to)
				to->GetAll(blocks[i_from][j_from][k_from]);
		}
	}
	public:
	void Drop(const unsigned short i, const unsigned short j, const unsigned short k, int n) {
		unsigned short i_to, j_to, k_to;
		if ( !Focus(i, j, k, i_to, j_to, k_to) )
			Exchange(i, j, k, i_to, j_to, k_to, n);
	}
	void PlayerDrop(const int n) {
		unsigned short playerX, playerY, playerZ;
		GetPlayerCoords(playerX, playerY, playerZ);
		Drop(playerX, playerY, playerZ, n);
	}
	void Get(const unsigned short i, const unsigned short j, const unsigned short k, int n) {
		unsigned short i_from, j_from, k_from;
		if ( !Focus(i, j, k, i_from, j_from, k_from) )
			Exchange(i_from, j_from, k_from, i, j, k, n);
	}
	void PlayerGet(const int n) {
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
	void Wield(Dwarf * const dwarf, const int n) {
		if (0>n || inventory_size<=n) return;
		Block * temp=dwarf->Drop(n);
		if (NULL==temp) return;
		if ( !dwarf->Wield(temp) )
			dwarf->Get(temp);
	}
	void PlayerWield() {
		unsigned short i, j;
		playerP->RangeForWield(i, j);
		if (i<inventory_size) {
			char str[18];
			sprintf(str, "Wield what? (%c-%c)", i+'a', j+'a');
			scr->Notify(str);
			Wield(playerP, getch()-'a');
			scr->Notify("");
		} else
			scr->Notify("Nothing to wield.");
	}

	//block information section	
	bool InBounds(const unsigned short i, const unsigned short j, const unsigned short k) const {
		return (i<shred_width*3 && j<shred_width*3 && k<height);
	}
	void FullName(char * str, const int i, const int j, const int k) const {
		if ( InBounds(i, j, k) )
			(NULL==blocks[i][j][k]) ? WriteName(str, "Air") : blocks[i][j][k]->FullName(str);
	}
	subs Sub(const int i, const int j, const int k) const    { return (!InBounds(i, j, k) || NULL==blocks[i][j][k]) ? AIR : blocks[i][j][k]->Sub(); }
	kinds Kind(const int i, const int j, const int k) const  { return (!InBounds(i, j, k) || NULL==blocks[i][j][k]) ? BLOCK : blocks[i][j][k]->Kind(); }
	int  Transparent(const int i, const int j, const int k) const {
		return (!InBounds(i, j, k) || NULL==blocks[i][j][k]) ? 2 : blocks[i][j][k]->Transparent();
	}
	int  Movable(const Block * const block) const            { return (NULL==block) ? ENVIRONMENT : block->Movable(); }
	double Weight(const Block * const block) const           { return (NULL==block) ? 0 : block->Weight(); }
	void * HasInventory(const int i, const int j, const int k) const {
		return (!InBounds(i, j, k) || NULL==blocks[i][j][k]) ? NULL : blocks[i][j][k]->HasInventory();
	}
	void * ActiveBlock(const int i, const int j, const int k) const {
		return (!InBounds(i, j, k) || NULL==blocks[i][j][k]) ? NULL : blocks[i][j][k]->ActiveBlock();
	}
	void GetNote(char * const str, const int i, const int j, const int k) const {
		char note[note_length];
		if (InBounds(i, j, k) && NULL!=blocks[i][j][k]) {
			blocks[i][j][k]->GetNote(note);
			if ('\0'!=note[0]) {
				strcat(str, "\n Inscription: \"");
				strcat(str, note);
				strcat(str, "\"");
			}
		}
	}
	void GetTemperature(char * const str, const int i, const int j, const int k) const {
		char temp_str[10];
		if (NULL!=blocks[i][j][k]) {
			sprintf(temp_str, "%d", Temperature(i, j, k));
			if ('\0'!=temp_str[0]) {
				strcat(str, "\n Temperature: ");
				strcat(str, temp_str);
			}
		}	
	}
	int Temperature(const int i_center, const int j_center, const int k_center) const {
		if (!InBounds(i_center, j_center, k_center) || NULL==blocks[i_center][j_center][k_center] || height-1==k_center)
			return 0;
		int temperature=blocks[i_center][j_center][k_center]->Temperature();
		if ( temperature )
			return temperature;

		for (int i=i_center-1; i<=i_center+1; ++i)
		for (int j=j_center-1; j<=j_center+1; ++j)
		for (int k=k_center-1; k<=k_center+1; ++k)
			if (InBounds(i, j, k) && NULL!=blocks[i][j][k])
				temperature+=blocks[i][j][k]->Temperature();
		return temperature/2;
	}
	bool Equal(const Block * const block1, const Block * const block2) const {
		if (NULL==block1 && NULL==block2) return true;
		if (NULL==block1 || NULL==block2) return false;
		return *block1==*block2;
	}
	bool Enlightened(const int i, const int j, const int k) const {
		if ( !InBounds(i, j, k) || NULL==blocks[i][j][k])
			return false;
		return blocks[i][j][k]->enlightened;
	}
	private:
	float LightRadius(const int i, const int j, const int k) const { return (NULL==blocks[i][j][k]) ? 0 : blocks[i][j][k]->LightRadius(); }
	bool UnderTheSky(const int i, const int j, int k) const {
		if ( !InBounds(i, j, k) ) return false;
		for (++k; k<height-1; ++k)
			if ( !Transparent(i, j, k) )
				return false;
		return true;
	}

	private:
	friend void Screen::PrintNormal(WINDOW *) const;
	friend void Screen::PrintFront(WINDOW *) const;
	friend void Screen::PrintInv(WINDOW *, Inventory *) const;
	friend class Active;

	friend void Grass::Act();
	friend int Dwarf::Move(dirs);

	public:
	World();
	~World();
};

#endif
