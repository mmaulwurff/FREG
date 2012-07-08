	/*
	*This file is part of Eyecube.
	*
	*Eyecube is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*Eyecube is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with Eyecube. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <curses.h>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <cmath>
#include <string.h>

int abs(int number) { return (number<0) ? (-number) : number; }

const unsigned short shred_width=10;
const unsigned short height=100;
const unsigned short full_name_length=20;
const unsigned short inventory_size=26;
const unsigned short max_stack_num=9; //num_str in Screen::PrintInv must be big enough
void WriteName(char * str, const char * name) { strncpy(str, name, full_name_length); }
enum color_pairs { //do not change colors order!
        BLACK_BLACK=1,
        BLACK_RED,
        BLACK_GREEN,
        BLACK_YELLOW,
        BLACK_BLUE,
        BLACK_MAGENTA,
        BLACK_CYAN,
        BLACK_WHITE,
        //
        RED_BLACK,
        RED_RED,
        RED_GREEN,
        RED_YELLOW,
        RED_BLUE,
        RED_MAGENTA,
        RED_CYAN,
        RED_WHITE,
        //
        GREEN_BLACK,
        GREEN_RED,
        GREEN_GREEN,
        GREEN_YELLOW,
        GREEN_BLUE,
        GREEN_MAGENTA,
        GREEN_CYAN,
        GREEN_WHITE,
        //
        YELLOW_BLACK,
        YELLOW_RED,
        YELLOW_GREEN,
        YELLOW_YELLOW,
        YELLOW_BLUE,
        YELLOW_MAGENTA,
        YELLOW_CYAN,
        YELLOW_WHITE,
        //
        BLUE_BLACK,
        BLUE_RED,
        BLUE_GREEN,
        BLUE_YELLOW,
        BLUE_BLUE,
        BLUE_MAGENTA,
        BLUE_CYAN,
        BLUE_WHITE,
        //
	MAGENTA_BLACK,
        MAGENTA_RED,
        MAGENTA_GREEN,
        MAGENTA_YELLOW,
        MAGENTA_BLUE,
        MAGENTA_MAGENTA,
        MAGENTA_CYAN,
        MAGENTA_WHITE,
        //
        CYAN_BLACK,
        CYAN_RED,
        CYAN_GREEN,
        CYAN_YELLOW,
        CYAN_BLUE,
        CYAN_MAGENTA,
        CYAN_CYAN,
        CYAN_WHITE,
        //
        WHITE_BLACK,
        WHITE_RED,
        WHITE_GREEN,
        WHITE_YELLOW,
        WHITE_BLUE,
        WHITE_MAGENTA,
        WHITE_CYAN,
        WHITE_WHITE,
};
enum special_views { NONE, INVENTORY };
enum dirs { NORTH, SOUTH, EAST, WEST, UP, DOWN };
enum { NOT_MOVABLE, MOVABLE,  GAS };
enum subs {
	AIR, //though there is no air block.
	STONE,
	NULLSTONE,
	SOIL,
	DWARF,
	GLASS
};

class World;
class Block { //blocks without special physics and attributes
	protected:
	const subs id;
	double weight;
	int shown_weight;
	dirs direction;
	public:
	void SetWeight(double m) { 
		weight=shown_weight;
		shown_weight=m;
	}
	void SetWeight() { shown_weight=weight; }
	double Weight() { return shown_weight; }
	dirs GetDir() { return direction; }
	void SetDir(dirs dir) { direction=dir; }
	subs Id() { return id; }
	virtual bool Stackable() { return  true; }
	virtual int Movable() { return NOT_MOVABLE; }
	virtual int Transparent() {
		switch (id) {
			case GLASS: return 1;
			default: return 0; //0 - totally invisible blocks, 1 - block is visible, but light can pass through it, 2 - invisible
		}
	}
	virtual void FullName(char * str) {
		switch (id) {
			case STONE: WriteName(str, "Stone"); break;
			case NULLSTONE: WriteName(str, "Nullstone"); break;
			case SOIL: WriteName(str, "Soil"); break;
			case GLASS: WriteName(str, "Glass"); break;
			default: WriteName(str, "Some unknown thing");
		}
	}
	Block(subs n) : id(n), shown_weight(1), weight(1), direction(NORTH) {}
};

class Animal : public Block {
	protected:
	int health;
	public:
	virtual int Movable() { return MOVABLE; }
	virtual bool Stackable() { return false; }
	virtual void FullName(char * str) {
		switch (id) {
			default: WriteName(str, "Some animal");
		}
	}
	Animal(subs n) : Block::Block(n) {}
};

class Dwarf : public Animal {
	struct {
		Block * block;
		unsigned short number;
	} inventory[inventory_size];
	Block * &onHead;
	Block * &onBody;
	Block * &onFeet;
	Block * &inRightHand;
	Block * &inLeftHand;
	public:
	Dwarf() : Animal::Animal(DWARF), onHead(inventory[0].block), onBody(inventory[1].block), onFeet(inventory[2].block),
			inRightHand(inventory[3].block), inLeftHand(inventory[4].block) {
		unsigned short i;
		for (i=0; i<inventory_size; ++i) {
			inventory[i].block=new Block(STONE);
			inventory[i].number=2;
		}
	}
	~Dwarf() {
		unsigned short i;
		for (i=0; i<inventory_size; ++i)
			delete inventory[i].block;
	}
	void FullName(char * str, int i) { (NULL==inventory[i].block) ? WriteName(str, "") : inventory[i].block->FullName(str); }
	void NumStr(char * str, int i) {
		if (1==inventory[i].number)
			strcpy(str, "");
		else
			sprintf(str, "(%hdx) ", inventory[i].number);
	}
	double GetInvWeight(int i) { return (NULL==inventory[i].block) ? 0 : inventory[i].block->Weight()*inventory[i].number; }
	subs GetInvId(int i) { return (NULL==inventory[i].block) ? AIR : inventory[i].block->Id(); }
	virtual void FullName(char * str) {
		switch (id) {
			default: WriteName(str, "Dwarf");
		}
	}
};

class Screen {
	World * const w; //connected world
	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin;
	char CharName(unsigned short i, unsigned short j, unsigned short k);
	void PrintInv();
	public:
	special_views view;
	Screen(World *wor);
	~Screen();
	int Color(subs);
	int Color(unsigned short i, unsigned short j, unsigned short k);
	void Print();
	void Notify(char *);
	void InvOnOff() {
		if (NONE==view) {
			view=INVENTORY;
			wclear(rightWin);
		} else
			view=NONE;
	}
};

void *PhysThread(void *vptr_args);
class World {
	unsigned long time;
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	unsigned short playerX, playerY, playerZ;
	long longitude, latitude;
	pthread_t eventsThread;
	pthread_mutex_t mutex;
	void LoadShred(long, long, unsigned short, unsigned short);
	void SaveShred(long, long, unsigned short, unsigned short);
	void ReloadShreds(dirs);
	public:
	Screen * scr;
	void PhysEvents();
	char CharNumber(int, int, int);
	char CharNumberFront(int, int);
	bool DirectlyVisible(int, int, int, int, int, int);
	bool Visible(int, int, int, int, int, int);
	bool Visible(int x_to, int y_to, int z_to) { return Visible(playerX, playerY, playerZ, x_to, y_to, z_to); }
	int  Move(int, int, int, dirs);
	void Jump(int, int, int);
	void Focus(int i, int j, int k, int & i_target, int & j_target, int & k_target) {
		i_target=i;
		j_target=j;
		k_target=k;
		switch ( blocks[i][j][k]->GetDir() ) {
			case NORTH: --j_target; break;
			case SOUTH: ++j_target; break;
			case EAST:  ++i_target; break;
			case WEST:  --i_target; break;
			case DOWN:  --k_target; break;
			case UP:    ++k_target; break;
		}
	}
	void PlayerFocus(int & i_target, int & j_target, int & k_target) { Focus(playerX, playerY, playerZ, i_target, j_target, k_target); }
	void SetPlayerDir(dirs dir) { playerP->SetDir(dir); }
	dirs GetPlayerDir() { return playerP->GetDir(); }
	void GetPlayerCoords(short * const x, short * const y, short * const z) { *x=playerX; *y=playerY; *z=playerZ; }
	Dwarf * GetPlayerP() { return playerP; }
	void FullName(char * str, int i, int j, int k) { (NULL==blocks[i][j][k]) ? WriteName(str, "Air") : blocks[i][j][k]->FullName(str); }
	subs Id(int i, int j, int k)          { return (NULL==blocks[i][j][k]) ? AIR : blocks[i][j][k]->Id(); }
	int  Transparent(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? 2 : blocks[i][j][k]->Transparent(); }
	int  Movable(Block * block)           { return (NULL==block) ? GAS : block->Movable(); }
	double Weight(Block * block)          { return (NULL==block) ? 0 : block->Weight(); }
	int  PlayerMove(dirs dir)             { return Move( playerX, playerY, playerZ, dir ); }
	int  PlayerMove()                     { return Move( playerX, playerY, playerZ, playerP->GetDir() ); }
	void PlayerJump() {
		playerP->SetWeight(0);
		if ( PlayerMove(UP) ) PlayerMove();
		playerP->SetWeight();
		PlayerMove(DOWN);
	}
	unsigned long Time() { return time; }
	friend void Screen::Print();
	World();
	~World();
};
void World::LoadShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
	unsigned short i, j, k;
	for (i=istart; i<istart+shred_width; ++i)
	for (j=jstart; j<jstart+shred_width; ++j) {
		blocks[i][j][0]=new Block(NULLSTONE);
		for (k=1; k<height/2; ++k)
			blocks[i][j][k]=new Block(STONE);
		for ( ; k<height; ++k)
			blocks[i][j][k]=NULL;
	}
	for (i=istart+1; i<istart+7; ++i)
		blocks[i][jstart+1][height/2]=new Block(GLASS);
	for (i=istart+1; i<istart+7; ++i) {
		delete blocks[i][jstart+2][height/2-1];
		blocks[i][jstart+2][height/2-1]=NULL;
	}
	for (k=height/2; k>0; --k) {
		delete blocks [istart+3][jstart+3][k];
		blocks[istart+3][jstart+3][k]=NULL;
	}
}
void World::SaveShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
	unsigned short i, j, k;
	for (i=istart; i<istart+shred_width; ++i)
	for (j=jstart; j<jstart+shred_width; ++j)
		for (k=0; k<height; ++k)
			delete blocks[i][j][k];
}
void World::ReloadShreds(dirs direction) { //ReloadShreds is called from Move, so there is no need to use mutex in this function
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		SaveShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	switch (direction) {
		case NORTH: --longitude; break;
		case SOUTH: ++longitude; break;
		case EAST:  ++latitude;  break;
		case WEST:  --latitude;  break;
	}
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		LoadShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	blocks[playerX][playerY][playerZ] =(Block*)( playerP=new Dwarf );
	blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block *)new Dwarf;
}
void World::PhysEvents() {
	++time;
	unsigned short i, j, k;
	if (NULL!=scr) scr->Print();
}
char World::CharNumber(int i, int j, int k) {
	if (i==playerX && j==playerY && k==playerZ)
		switch ( playerP->GetDir() ) {
			case NORTH: return '^';
			case SOUTH: return 'v';
			case EAST:  return '>';
			case WEST:  return '<';
			case DOWN:  return 'x';
			case UP:    return '.';
		}	
	if ( UP==GetPlayerDir() ) {
		if (k > playerZ && k < playerZ+10) return k-playerZ+'0';
	} else {
		if (k==playerZ) return ' ';
		return playerZ-k+'0';
	}
	return '+';
}
char World::CharNumberFront(int i, int j) {
	unsigned short ret;
	if ( NORTH==playerP->GetDir() || SOUTH==playerP->GetDir() ) {
		if ( (ret=abs(playerY-j))<10 ) return ret+'0';
	} else
		if ( (ret=abs(playerX-i))<10 ) return ret+'0';
	return '+';
}
bool World::DirectlyVisible(int x_from, int y_from, int z_from,
                            int x_to,   int y_to,   int z_to) {
	if (x_from==x_to && y_from==y_to && z_from==z_to) return true;
	unsigned short max=(abs(z_to-z_from) > abs(y_to-y_from)) ? abs(z_to-z_from) : abs(y_to-y_from);
	if (abs(x_to-x_from) > max) max=abs(x_to-x_from);
	float x_step=(float)(x_to-x_from)/max,
	      y_step=(float)(y_to-y_from)/max,
	      z_step=(float)(z_to-z_from)/max;
	unsigned short i;
	for (i=1; i<max; ++i)
		if ( !Transparent(nearbyint(x_from+i*x_step),
		                  nearbyint(y_from+i*y_step),
		                  nearbyint(z_from+i*z_step)))
		   	return false;
	return true;
}
bool World::Visible(int x_from, int y_from, int z_from,
                    int x_to,   int y_to,   int z_to) {
	short temp;
	if ((DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
		(Transparent(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to) && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to)) ||
		(Transparent(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to)) ||
		(Transparent(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1)) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp)))
			return true;
	return false;
}
int World::Move(int i, int j, int k, dirs dir) {
	pthread_mutex_lock(&mutex);
	if ( Movable(blocks[i][j][k])==NOT_MOVABLE ||
			(!i && dir==WEST ) ||
			(!j && dir==NORTH) ||
			(!k && dir==DOWN ) ||
			(i==shred_width*3-1 && dir==EAST ) ||
			(j==shred_width*3-1 && dir==SOUTH) ||
			(k==height-1 && dir==UP) ) {
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	int newi=i, newj=j, newk=k;
	switch (dir) {
		case NORTH: --newj; break;
		case SOUTH: ++newj; break;
		case EAST:  ++newi; break;
		case WEST:  --newi; break;
		case UP:    ++newk; break;
		case DOWN:  --newk; break;
	}
	int numberMoves=0;
	if (GAS==Movable(blocks[newi][newj][newk]) || (numberMoves=Move(newi, newj, newk, dir)) ) {
		Block *temp=blocks[i][j][k];
		blocks[i][j][k]=blocks[newi][newj][newk];
		blocks[newi][newj][newk]=temp;
		int weight;
		if ( weight=Weight(blocks[i][j][k]) )
			if (weight>0) Move(i, j, k, DOWN);
			else        Move(i, j, k, UP);
		if ( weight=Weight(blocks[newi][newj][newk]) )
			if (weight>0) newk-=Move(newi, newj, newk, DOWN);
			else        newk+=Move(newi, newj, newk, UP);
		if (blocks[newi][newj][newk]==(Block*)playerP) {
			playerX=newi;
			playerY=newj;
			playerZ=newk;
			if (playerX==shred_width-1) {
				playerX+=shred_width;
				ReloadShreds(WEST);
			} else if (playerX==shred_width*2) {
				playerX-=shred_width;
				ReloadShreds(EAST);
			} else if (playerY==shred_width-1) {
				playerY+=shred_width;
				ReloadShreds(NORTH);
			} else if (playerY==shred_width*2) {
				playerY-=shred_width;
				ReloadShreds(SOUTH);
			}
		}
		pthread_mutex_unlock(&mutex);
		return numberMoves+1;
	}
	pthread_mutex_unlock(&mutex);
	return 0;
}
void World::Jump(int i, int j, int k) {
	if ( NULL!=blocks[i][j][k] && blocks[i][j][k]->Movable() ) {
		blocks[i][j][k]->SetWeight(0);
		if ( Move(i, j, k, UP) ) {
			++k;
			dirs dir;
			if (Move( i, j, k, dir=blocks[i][j][k]->GetDir() ));
				switch (dir) {
					case NORTH: --j; break;
					case SOUTH: ++j; break;
					case EAST:  ++i; break;
					case WEST:  --i; break;
					case UP:    ++k; break;
					case DOWN:  --k; break;
				}
		}
		blocks[i][j][k]->SetWeight();
		Move(i, j, k, DOWN);
	}
}
World::World() {
	//TODO: add load and save
	FILE * file=fopen("world.txt", "r");
	if (file==NULL) {
		longitude=0;
		latitude=0;
		playerX=shred_width*2-7;
		playerY=shred_width*2-7;
		playerZ=height/2;
	} else {
		fscanf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\n playerY: %hd\n playerZ: %hd\n",
				&longitude, &latitude, &playerX, &playerY, &playerZ);
		fclose(file);
	}
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		LoadShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
	blocks[playerX][playerY][playerZ] =(Block*)( playerP=new Dwarf );
	blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block*)new Dwarf;
	time=0;
	scr=NULL;
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mutex_attr);
	pthread_create(&eventsThread, NULL, PhysThread, this);
}
World::~World() {
	pthread_cancel(eventsThread);
	pthread_mutex_destroy(&mutex);
	FILE * file=fopen("world.txt", "w");
	if ("file!=NULL") {
		fprintf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\nplayerY: %hd\nplayerZ: %hd\n",
				longitude, latitude, playerX, playerY, playerZ);
		fclose(file);
	}
	long i, j;
	for (i=longitude-1; i<=longitude+1; ++i)
	for (j=latitude-1;  j<=latitude+1;  ++j)
		SaveShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
}
void *PhysThread(void *vptr_args) {
	while (1) {
		((World*)vptr_args)->PhysEvents();
		sleep(1);
	}
}

char Screen::CharName(unsigned short i, unsigned short j, unsigned short k) {
	switch ( w->Id(i, j, k) ) {
		case NULLSTONE:
		case STONE: return '#';
		case SOIL:  return 's';
		case DWARF: return '@';
		case GLASS: return 'g';
		default: return '?';
	}
}
void Screen::Print() {
	if (pthread_mutex_trylock(&(w->mutex)))
		return;
	//left window
	wmove(leftWin, 1, 1);
	short i, j, k;
	for ( j=0; j<shred_width*3; ++j, waddstr(leftWin, "\n_") )
	for ( i=0; i<shred_width*3; ++i )
		for (k=w->playerZ; k>=0; --k) //bottom is made from undestructable stone, loop will find what to print everytime
			if (w->Transparent(i, j, k) < 2) {
				if ( w->Visible(i, j, k) ) {
					wattrset(leftWin, Color(i, j, k));
					wprintw( leftWin, "%c%c", CharName(i, j, k), w->CharNumber(i, j, k) );
				} else {
					wattrset(leftWin, COLOR_PAIR(BLACK_BLACK));
					wprintw(leftWin, "  ");
				}
				break;
			}
	wstandend(leftWin);
	box(leftWin, 0, 0);
	if ( UP==w->GetPlayerDir() )
		mvwaddstr(leftWin, 0, 1, "Sky View");
	else 
		mvwaddstr(leftWin, 0, 1, "Normal View");
	wrefresh(leftWin);
	//right window
	if (INVENTORY==view) {
		PrintInv();
		return;
	} else if ( UP==w->GetPlayerDir() || DOWN==w->GetPlayerDir() ) {
		wclear(rightWin);
		wrefresh(rightWin);
		return;
	}
	short pX, pY, pZ,
	      x_step, z_step,
	      x_end, z_end,
	      * x, * z;
	unsigned short x_start, z_start,
	               k_start,
	               arrow_Y, arrow_X;
	w->GetPlayerCoords(&pX, &pY, &pZ);
	switch ( w->GetPlayerDir() ) {
		case NORTH:
			x=&i;
			x_step=1;
			x_start=0;
			x_end=shred_width*3;
			z=&j;
			z_step=-1;
			z_start=pY-1;
			z_end=-1;
			arrow_X=pX*2+1;
		break;
		case SOUTH:
			x=&i;
			x_step=-1;
			x_start=shred_width*3-1;
			x_end=-1;
			z=&j;
			z_step=1;
			z_start=pY+1;
			z_end=shred_width*3;
			arrow_X=(shred_width*3-pX)*2-1;
		break;
		case WEST:
			x=&j;
			x_step=-1;
			x_start=shred_width*3-1;
			x_end=-1;
			z=&i;
			z_step=-1;
			z_start=pX-1;
			z_end=-1;
			arrow_X=(shred_width*3-pY)*2-1;
		break;
		case EAST:
			x=&j;
			x_step=1;
			x_start=0;
			x_end=shred_width*3;
			z=&i;
			z_step=1;
			z_start=pX+1;
			z_end=shred_width*3;
			arrow_X=pY*2+1;
		break;
	}
	if (pZ+shred_width*1.5>=height) {
		k_start=height-1;
		arrow_Y=height-pZ+1;
	} else if (pZ-shred_width*1.5<0) {
		k_start=shred_width*3-1;
		arrow_Y=shred_width*3-pZ;
	} else {
		k_start=pZ+shred_width*1.5;
		arrow_Y=shred_width*1.5+1;
	}
	wmove(rightWin, 1, 1);
	for (k=k_start; k_start-k<shred_width*3; --k, waddstr(rightWin, "\n_"))
		for (*x=x_start; *x!=x_end; *x+=x_step) {
			for (*z=z_start; *z!=z_end; *z+=z_step)
				if (w->Transparent(i, j, k) < 2) {
					if ( w->Visible(i, j, k) ) {
						wattrset(rightWin, Color(i, j, k));
						wprintw( rightWin, "%c%c", CharName(i, j, k), w->CharNumberFront(i, j) );
					} else {
						wattrset(rightWin, COLOR_PAIR(BLACK_BLACK));
						wprintw(rightWin, "  ");
					}
					break;
				}
			if (*z==z_end) { //print background decorations
				*z-=z_step;
				if (w->Visible(i, j, k)) {
				       	wattrset(rightWin, COLOR_PAIR(WHITE_BLUE));
					wprintw(rightWin, " .");
				} else {
					wattrset(rightWin, COLOR_PAIR(BLACK_BLACK));
					waddstr(rightWin, "  ");
				}
			}
		}
	wstandend(rightWin);
	box(rightWin, 0, 0);
	mvwaddstr(rightWin, 0, 1, "Front View");
	mvwprintw(rightWin, 0,               arrow_X,           "vv");
	mvwprintw(rightWin, shred_width*3+1, arrow_X,           "^^");
	mvwprintw(rightWin, arrow_Y,         0,                 ">");
	mvwprintw(rightWin, arrow_Y,         shred_width*3*2+1, "<");
	wrefresh(rightWin);
	pthread_mutex_unlock(&(w->mutex));
}
void Screen::PrintInv() {
	//wclear(rightWin);
	Dwarf * player=w->GetPlayerP();
	unsigned short i;
	double sum_weight=0, temp_weight;
	char str[full_name_length],
	     num_str[6];
	mvwaddstr(rightWin, 1, 50, "Weight");
	mvwaddstr(rightWin, 2, 4, "On head:");
	mvwaddstr(rightWin, 3, 4, "On body:");
	mvwaddstr(rightWin, 4, 4, "On feet:");
	mvwaddstr(rightWin, 5, 4, "In right hand:");
	mvwaddstr(rightWin, 6, 4, "In left hand:");
	for (i=0; i<inventory_size; ++i) {
		player->FullName(str, i);
		player->NumStr(num_str, i);
		mvwprintw(rightWin, 2+i, 20, "%c) %s", 'a'+i, num_str);
		wattrset( rightWin, Color(player->GetInvId(i)) );
		wprintw(rightWin, "%s", str);
		wstandend(rightWin);
		if ('\0'!=str[0]) {
			mvwprintw(rightWin, 2+i, 50, "%2.1f kg", temp_weight=player->GetInvWeight(i));
			sum_weight+=temp_weight;
		}
	}
	mvwprintw(rightWin, 2+i, 43, "Sum:%6.1f kg", sum_weight);
	box(rightWin, 0, 0);
	mvwprintw(rightWin, 0, 1, "Inventory");
	wrefresh(rightWin);
}
void Screen::Notify(char * str) {
	mvwprintw(notifyWin, 1, 1, "%s", str);
	box(notifyWin, 0, 0);
	wrefresh(notifyWin);
}
int Screen::Color(subs sub) {
	switch (sub) {
		case DWARF:     return COLOR_PAIR(WHITE_BLUE);
		case GLASS:     return COLOR_PAIR(BLUE_WHITE);
		case NULLSTONE: return COLOR_PAIR(WHITE_BLACK);
		default:        return COLOR_PAIR(BLACK_WHITE);
	}
}
inline int Screen::Color(unsigned short i, unsigned short j, unsigned short k) { return Color( w->Id(i, j, k) ); }
Screen::Screen(World *wor) : w(wor), view(NONE) {
	set_escdelay(10);
	initscr();
	start_color();
	raw();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	//all available color pairs (maybe some of them will not be used)
	short i, colors[]={ //do not change colors order!
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_BLUE,
		COLOR_MAGENTA,
		COLOR_CYAN,
		COLOR_WHITE
	};
	for (i=BLACK_BLACK; i<=WHITE_WHITE; ++i)
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	leftWin  =newwin(shred_width*3+2, shred_width*2*3+2, 0, 0);
	rightWin =newwin(shred_width*3+2, shred_width*2*3+2, 0, shred_width*2*3+2);
	notifyWin=newwin(5, (shred_width*2*3+2)*2, shred_width*3+2, 0);
	w->scr=this;
}
Screen::~Screen() {
	delwin(leftWin);
	delwin(rightWin);
	delwin(notifyWin);
	endwin();
	w->scr=NULL;
}

int main() {
	World earth;
	Screen screen(&earth);
	int c;
	int print_flag=1; //print only if needed, needed nearly everytime
	while ((c=getch())!='Q') {
		switch (c) {
			case ',': earth.PlayerMove(NORTH); break;
			case 'o': earth.PlayerMove(SOUTH); break;
			case 'e': earth.PlayerMove(EAST ); break;
			case 'a': earth.PlayerMove(WEST ); break;
			case ' ': earth.PlayerJump(); break;
			case KEY_LEFT:  earth.SetPlayerDir(WEST);  break;
			case KEY_RIGHT: earth.SetPlayerDir(EAST);  break;
			case KEY_DOWN:  earth.SetPlayerDir(SOUTH); break;
			case KEY_UP:    earth.SetPlayerDir(NORTH); break;
			case 'v':       earth.SetPlayerDir(DOWN);  break;
			case '^':       earth.SetPlayerDir(UP);    break;
			case 'i': screen.InvOnOff(); break;
			case '?': {
				int i, j, k;
				earth.PlayerFocus(i, j, k);
				char str[full_name_length];
				earth.FullName(str, i, j, k);
				screen.Notify(str);
			} break;
		}
		if (print_flag) screen.Print();
	}
}
