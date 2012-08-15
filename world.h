#ifndef WORLD_H
#define WORLD_H

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

#include <pthread.h>
#include <cmath>

class Screen;

void *PhysThread(void *vptr_args);
class World {
	unsigned long time;
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	unsigned short playerX, playerY, playerZ;
	long longitude, latitude;
	pthread_t eventsThread;
	pthread_mutex_t mutex;
	Active * activeList;
	void LoadShred(long, long, unsigned short, unsigned short);
	void SaveShred(long, long, unsigned short, unsigned short);
	void ReloadShreds(dirs);
	void LoadAllShreds();
	void SaveAllShreds();
	void MakeSky() {
		FILE * sky=fopen("sky.txt", "r");
		if (NULL==sky) {
			for (unsigned short i=0; i<shred_width*3; ++i)
			for (unsigned short j=0; j<shred_width*3; ++j)
				blocks[i][j][height-1]=new Block( random()%5 ? STAR : SKY );
		} else {
			char c=fgetc(sky)-'0';
			for (unsigned short i=0; i<shred_width*3; ++i)
			for (unsigned short j=0; j<shred_width*3; ++j)
				if (c) {
					blocks[i][j][height-1]=new Block(SKY);
					--c;
				} else {
					blocks[i][j][height-1]=new Block(STAR);
					c=fgetc(sky)-'0';
				}
			fclose(sky);
		}
	}
	dirs MakeDir(unsigned short x_center, unsigned short y_center, unsigned short x_target, unsigned short y_target) {
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
	double Distance(unsigned short x_from, unsigned short y_from, unsigned short z_from,
	                unsigned short x_to,   unsigned short y_to,   unsigned short z_to) {
		return sqrt( (x_from-x_to)*(x_from-x_to)+
		             (y_from-y_to)*(y_from-y_to)+
		             (z_from-z_to)*(z_from-z_to) );
	}
	void FileName(char * str, long longi, long lati) { sprintf(str, "shreds/%ld_%ld", longi, lati); }

	bool LegalXYZ(int i, int j, int k) { return (i>=0 && i<shred_width*3 && j>=0 && j<shred_width*3 && k>0 && k<height-1); }

	public:
	Screen * scr;
	struct {
		char ch;
		unsigned short lev;
		color_pairs col;
	} soundMap[9];

	void PhysEvents();
	char CharNumber(int, int, int);
	char CharNumberFront(int, int);

	bool DirectlyVisible(int, int, int, int, int, int);
	bool Visible(int, int, int, int, int, int);
	bool Visible(int x_to, int y_to, int z_to) { return Visible(playerX, playerY, playerZ, x_to, y_to, z_to); }

	int  Move(int, int, int, dirs);
	void Jump(int, int, int);
	void Focus(int, int, int, int &, int &, int &);

	void PlayerFocus(int & i_target, int & j_target, int & k_target) { Focus(playerX, playerY, playerZ, i_target, j_target, k_target); }
	void SetPlayerDir(dirs dir) { playerP->SetDir(dir); }
	dirs GetPlayerDir() { return playerP->GetDir(); }
	void GetPlayerCoords(short & x, short & y, short & z) { x=playerX; y=playerY; z=playerZ; }
	void GetPlayerCoords(short & x, short & y) { x=playerX; y=playerY; }
	Dwarf * GetPlayerP() { return playerP; }

	unsigned long GetTime() { return time; }
	times_of_day PartOfDay() {
		unsigned short time_day=TimeOfDay();
		if (time_day<end_of_night)   return NIGHT;
		if (time_day<end_of_morning) return MORNING;
		if (time_day<end_of_noon)    return NOON;
		return EVENING;
	}
	int TimeOfDay() { return time%seconds_in_day; }
	unsigned long Time() { return time; }

	int Use(int i, int j, int k) { if (NULL!=blocks[i][j][k]) blocks[i][j][k]->Use(); }
	void Exchange(int i_from, int j_from, int k_from,
	              int i_to,   int j_to,   int k_to, int n) {
		if ( pthread_mutex_trylock(&mutex) ) return;
		Block * inv_from=blocks[i_from][j_from][k_from];
		Block * inv_to=blocks[i_to][j_to][k_to];
		if ( HasInventory(i_from, j_from, k_from))
			if ( HasInventory(i_to, j_to, k_to) ) {
				Block * temp=inv_from->Drop(n);
				if ( NULL!=temp && !inv_to->Get(temp) ) {
					inv_from->Get(temp);
					scr->Notify("Not enough room\n");
				}
			} else if (NULL==blocks[i_to][j_to][k_to]) {
				blocks[i_to][j_to][k_to]=new Pile(this, i_to, j_to, k_to);
				blocks[i_to][j_to][k_to]->Get( inv_from->Drop(n) );
			}
		pthread_mutex_unlock(&mutex);
	}
	void Drop(int i, int j, int k, int n) {
		int i_to, j_to, k_to;
		Focus(i, j, k, i_to, j_to, k_to);
		Exchange(i, j, k, i_to, j_to, k_to, n);
	}
	void PlayerDrop(int n) { Drop(playerX, playerY, playerZ, n); }
	void Get(int i, int j, int k, int n) {
		int i_from, j_from, k_from;
		Focus(i, j, k, i_from, j_from, k_from);
		Exchange(i_from, j_from, k_from, i, j, k, n);

	}
	void PlayerGet(int n) { Get(playerX, playerY, playerZ, n); }

	void FullName(char * str, int i, int j, int k) { (NULL==blocks[i][j][k]) ? WriteName(str, "Air") : blocks[i][j][k]->FullName(str); }
	subs Sub(int i, int j, int k)          { return (NULL==blocks[i][j][k]) ? AIR : blocks[i][j][k]->Sub(); }
	kinds Kind(int i, int j, int k)        { return (NULL==blocks[i][j][k]) ? BLOCK : blocks[i][j][k]->Kind(); }
	int  Transparent(int i, int j, int k)  { return (NULL==blocks[i][j][k]) ? 2 : blocks[i][j][k]->Transparent(); }
	int  Movable(Block * block)            { return (NULL==block) ? ENVIRONMENT : block->Movable(); }
	double Weight(Block * block)           { return (NULL==block) ? 0 : block->Weight(); }
	bool HasInventory(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? false : blocks[i][j][k]->HasInventory(); }

	int  PlayerMove(dirs dir)              { return Move( playerX, playerY, playerZ, dir ); }
	int  PlayerMove()                      { return Move( playerX, playerY, playerZ, playerP->GetDir() ); }
	void PlayerJump() {
		playerP->SetWeight(0);
		if ( PlayerMove(UP) ) PlayerMove();
		playerP->SetWeight();
		PlayerMove(DOWN);
	}
	void Wield(Dwarf * dwarf, int n) {
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

	void Inscribe(Dwarf * dwarf) {
		if (!dwarf->CarvingWeapon()) {
			scr->Notify("You need some tool for inscibing!\n");
			return;
		}
		unsigned short i, j, k;
		dwarf->GetSelfXYZ(i, j, k);
		int i_to, j_to, k_to;
		Focus(i, j, k, i_to, j_to, k_to);
		char str[note_length];
		scr->GetString(str);
		Inscribe(i_to, j_to, k_to, str);
	}
	void PlayerInscribe() { Inscribe(playerP); }
	void Inscribe(int i, int j, int k, char * str) { if (NULL!=blocks[i][j][k]) blocks[i][j][k]->Inscribe(str); }
	

	void GetNote(char * str, int i, int j, int k) {
		char note[note_length];
		if (NULL!=blocks[i][j][k]) {
			blocks[i][j][k]->GetNote(note);
			if ('\0'!=note[0]) {
				strcat(str, "\n Inscription: \"");
				strcat(str, note);
				strcat(str, "\"");
			}
		}
	}

	char MakeSound(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? ' ' : blocks[i][j][k]->MakeSound(); }

	private:
	float LightRadius(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? 0 : blocks[i][j][k]->LightRadius(); }
	bool UnderTheSky(int i, int j, int k) {
		if ( !LegalXYZ(i, j, k) ) return false;
		for (++k; k<height-1; ++k)
			if ( !Transparent(i, j, k) )
				return false;
		return true;
	}
	public:
	bool Enlightened(int i, int j, int k) {
		if ( height-1==k || ( NIGHT!=PartOfDay() && (
				LightRadius(i, j, k) || UnderTheSky(i, j, k) ||
				UnderTheSky(i-1, j, k) || UnderTheSky(i, j-1, k) ||
				UnderTheSky(i+1, j, k) || UnderTheSky(i, j+1, k)) ) )
			return true;

		unsigned short const x_start=(i-max_light_radius>0) ? i-max_light_radius : 0;
		unsigned short const y_start=(j-max_light_radius>0) ? j-max_light_radius : 0;
		unsigned short const z_start=(k-max_light_radius>0) ? k-max_light_radius : 0;
		unsigned short const x_end=(i+max_light_radius<shred_width*3) ? i+max_light_radius : shred_width*3-1;
		unsigned short const y_end=(j+max_light_radius<shred_width*3) ? j+max_light_radius : shred_width*3-1;
		unsigned short const z_end=(k+max_light_radius<height-1) ? k+max_light_radius : height-2;

		for (unsigned short x=x_start; x<=x_end; ++x)
		for (unsigned short y=y_start; y<=y_end; ++y)
		for (unsigned short z=z_start; z<=z_end; ++z)
			if (( LightRadius(x, y, z) > Distance(i, j, k, x, y, z) ) &&
					DirectlyVisible(x, y, z, i, j, k))
				return true;
		return false;
	}

	friend void Screen::PrintNormal(WINDOW *);
	friend void Screen::PrintFront(WINDOW *);
	friend void Screen::PrintInv(WINDOW *, Inventory *);
	friend class Active;

	World();
	~World();
};

#endif
