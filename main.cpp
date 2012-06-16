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

#include <ncurses.h>
#include <cstdio>
#include <cmath>

int abs(int number) { return (number<0) ? (-number) : number; }

const unsigned short shred_width=10;
const unsigned short height=100;
enum subs {
	AIR, //though there is no air block.
	STONE,
	SOIL,
	DWARF
};
enum color_pairs {
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
enum dirs { NORTH, SOUTH, EAST, WEST, UP, DOWN };

class Block {
	protected:
	subs id;
	public:
	virtual bool Movable() { return false; }
	virtual bool Transparent() { return false; } //totally invisible blocks
	virtual bool HalfTransparent() { return false; } //example: glass is not transparent, but halftransparent (blocks are visible through it).
	subs Id() { return id; }
	void* operator new(size_t, subs n) {
		Block * nblock=::new Block;
		nblock->id=n;
		return nblock;
	}
};

class Dwarf : private Block {
	int health;
	public:
	virtual bool Movable() { return true; }
	void *operator new(size_t) {
		Dwarf *ndwarf=::new Dwarf;
		ndwarf->id=DWARF;
		ndwarf->health=1;
		return ndwarf;
	}
};

class World {
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	dirs playerDir;
	unsigned short playerX, playerY, playerZ;
	long longitude, latitude;
	void * CreateBlock(subs id) {
		switch (id) {
			case DWARF: return new Dwarf;
			default: return new (id) Block;
		}
	}
	void DeleteBlock(Block * delblock) {
		if (delblock!=NULL)
			switch (delblock->Id()) {
				case DWARF: delete (Dwarf *)delblock; break;
				default: delete delblock; break;
			}
	}
	void LoadShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
		unsigned short i, j, k;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j) {
			for (k=0; k<height/2; ++k)
				blocks[i][j][k]=(Block*)CreateBlock(STONE);
			for ( ; k<height; ++k)
				blocks[i][j][k]=NULL;
		}
		for (i=istart+1; i<istart+7; ++i)
		for (k=height/2; k<height/2+1; ++k)
			blocks[i][jstart+1][k]=(Block*)CreateBlock(STONE);
	}
	void SaveShred(long longi, long lati, unsigned short istart, unsigned short jstart) {
		unsigned short i, j, k;
		for (i=istart; i<istart+shred_width; ++i)
		for (j=jstart; j<jstart+shred_width; ++j)
			for (k=0; k<height; ++k)
				DeleteBlock(blocks[i][j][k]);
	}
	void ReloadShreds(enum dirs direction) {
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
		blocks[playerX][playerY][playerZ] =(Block*)( playerP=(Dwarf*)CreateBlock(DWARF) );
		blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block*)CreateBlock(DWARF);
	}
	public:
	char CharNumber(unsigned short i, unsigned short j, unsigned short k) {
		if (i==playerX && j==playerY && k==playerZ)
			switch (playerDir) {
				case NORTH: return '^';
				case SOUTH: return 'v';
				case EAST:  return '>';
				case WEST:  return '<';
				case DOWN:  return 'x';
				case UP:    return '.';
			}
		if (k < playerZ-2) return '!';
		if (k== playerZ-2) return '_';
		if (k== playerZ-1) return ' '; //floor
		if (k > playerZ-1 && k < playerZ+9) return k-playerZ+1+'0';
		return '+';
	}
	bool DirectlyVisible(unsigned short x_from, unsigned short y_from, unsigned short z_from,
	                     unsigned short x_to,   unsigned short y_to,   unsigned short z_to) {
		if (x_from==x_to && y_from==y_to && z_from==z_to) return true;
		unsigned short max=(abs(z_to-z_from) > abs(y_to-y_from)) ? abs(z_to-z_from) : abs(y_to-y_from);
		if (abs(x_to-x_from) > max) max=abs(x_to-x_from);
		float x_step=(float)(x_to-x_from)/max,
		      y_step=(float)(y_to-y_from)/max,
		      z_step=(float)(z_to-z_from)/max;
		unsigned short i;
		for (i=1; i<max; ++i)
			if ( !HalfTransparent(nearbyint(x_from+i*x_step),
			                      nearbyint(y_from+i*y_step),
			                      nearbyint(z_from+i*z_step)))
			   	return false;
		return true;
	}
	bool Visible(unsigned short x_from, unsigned short y_from, unsigned short z_from,
	             unsigned short x_to,   unsigned short y_to,   unsigned short z_to) {
		short temp;
		if ((DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to)) ||
			(Transparent(x_to+(temp=(x_to>x_from) ? (-1) : 1), y_to, z_to) && DirectlyVisible(x_from, y_from, z_from, x_to+temp, y_to, z_to)) ||
			(Transparent(x_to, y_to+(temp=(y_to>y_from) ? (-1) : 1), z_to) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to+temp, z_to)) ||
			(Transparent(x_to, y_to, z_to+(temp=(z_to>z_from) ? (-1) : 1)) && DirectlyVisible(x_from, y_from, z_from, x_to, y_to, z_to+temp)))
				return true;
		return false;
	}
	bool Visible(unsigned short x_to, unsigned short y_to, unsigned short z_to) {
		return Visible(playerX, playerY, playerZ, x_to, y_to, z_to);
	}
	int Move(int i, int j, int k, dirs dir) {
		if (blocks[i][j][k]==NULL)
			return 1;
		if ( !blocks[i][j][k]->Movable() ||
				(!i && dir==WEST ) ||
				(!j && dir==NORTH) ||
				(!k && dir==DOWN ) ||
				(i==shred_width*3-1 && dir==EAST ) ||
				(j==shred_width*3-1 && dir==SOUTH) ||
				(k==height-1 && dir==UP) )
			return 0;
		int newi=i, newj=j, newk=k;
		switch (dir) {
			case NORTH: --newj; break;
			case SOUTH: ++newj; break;
			case EAST:  ++newi; break;
			case WEST:  --newi; break;
			case UP:    ++newk; break;
			case DOWN:  --newk; break;
		}
		if ( Move(newi, newj, newk, dir) ) {
			if (blocks[i][j][k]==(Block*)playerP) {
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
			Block *temp=blocks[i][j][k];
			blocks[i][j][k]=blocks[newi][newj][newk];
			blocks[newi][newj][newk]=temp;
			return 1;
		}
		return 0;
	}
	void Move(dirs dir) { Move(playerX, playerY, playerZ, dir); }
	subs Id(int i, int j, int k) {
		return (NULL==blocks[i][j][k]) ? AIR : blocks[i][j][k]->Id();
	}
	bool Transparent(unsigned short i, unsigned short j, unsigned short k) {
		return (NULL==blocks[i][j][k]) ? true : blocks[i][j][k]->Transparent();
	}
	bool HalfTransparent(unsigned short i, unsigned short j, unsigned short k) {
		return (NULL==blocks[i][j][k]) ? true : blocks[i][j][k]->HalfTransparent();
	}
	World() {//generate new world
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
		blocks[playerX][playerY][playerZ] =(Block*)( playerP=(Dwarf*)CreateBlock(DWARF) );
		blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block*)CreateBlock(DWARF);
		playerDir=NORTH;
	}
	~World() {
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
};

class Screen {
	World * const w; //connected world
	WINDOW * leftWin;
	char CharName(unsigned short i, unsigned short j, unsigned short k) {
		switch ( w->Id(i, j, k) ) {
			case STONE: return '#';
			case SOIL:  return 's';
			case DWARF: return '@';
			default: return '?';
		}
	}
	int Color(unsigned short i, unsigned short j, unsigned short k) {
		switch ( w->Id(i, j, k) ) {
			case DWARF: return COLOR_PAIR(WHITE_BLUE);
			default:    return COLOR_PAIR(BLACK_WHITE);
		}
	}
	public:
	Screen(World *wor) : w(wor) {
		set_escdelay(10);
		initscr();
		start_color();
		raw();
		noecho();
		keypad(stdscr, TRUE);
		curs_set(0);
		init_pair(BLACK_BLACK,  COLOR_BLACK,  COLOR_BLACK );
		init_pair(BLACK_RED,    COLOR_BLACK,  COLOR_RED   );
		init_pair(BLACK_GREEN,  COLOR_BLACK,  COLOR_GREEN );
		init_pair(BLACK_YELLOW, COLOR_BLACK,  COLOR_YELLOW);
		init_pair(BLACK_BLUE,   COLOR_BLACK,  COLOR_BLUE  );
		init_pair(BLACK_CYAN,   COLOR_BLACK,  COLOR_CYAN  );
		init_pair(BLACK_WHITE,  COLOR_BLACK,  COLOR_WHITE );
		//
		init_pair(RED_BLACK,    COLOR_RED,    COLOR_BLACK );
		init_pair(RED_YELLOW,   COLOR_RED,    COLOR_YELLOW);
		init_pair(RED_BLUE,     COLOR_RED,    COLOR_BLUE  );
		init_pair(RED_CYAN,     COLOR_RED,    COLOR_CYAN  );
		init_pair(RED_WHITE,    COLOR_RED,    COLOR_WHITE );
		//
		init_pair(GREEN_BLACK,  COLOR_GREEN,  COLOR_BLACK );
		init_pair(GREEN_CYAN,   COLOR_GREEN,  COLOR_CYAN  );
		//
		init_pair(YELLOW_RED,   COLOR_YELLOW, COLOR_RED   );
		//
		init_pair(BLUE_BLACK,   COLOR_BLUE,   COLOR_BLACK );
		init_pair(BLUE_YELLOW,  COLOR_BLUE,   COLOR_YELLOW);
		init_pair(BLUE_CYAN,    COLOR_BLUE,   COLOR_CYAN  );
		//
		init_pair(WHITE_BLACK,  COLOR_WHITE,  COLOR_BLACK );
		init_pair(WHITE_BLUE,   COLOR_WHITE,  COLOR_BLUE  );
		init_pair(WHITE_CYAN,   COLOR_WHITE,  COLOR_CYAN  );
		leftWin=newwin(shred_width*3+2, shred_width*2*3+2, 0, 0);
		refresh();
	}
	void print() {
		wmove(leftWin, 1, 1);
		unsigned short i, j, k;
		for ( j=0; j<shred_width*3; ++j, wprintw(leftWin, "\n_") )
		for ( i=0; i<shred_width*3; ++i )
			for (k=height-1; k>=0; --k) //bottom is made from undestructable stone, loop will find what to print everytime
				if ( !w->Transparent(i, j, k) ) {
					if ( w->Visible(i, j, k) ) {
						wattrset(leftWin, Color(i, j, k));
						wprintw( leftWin, "%c%c", CharName(i, j, k), w->CharNumber(i, j, k) );
						break;
					}
					wattrset(leftWin, COLOR_PAIR(BLACK_BLACK));
					wprintw(leftWin, "  ");
					break;
				}
		wstandend(leftWin);
		box(leftWin, 0, 0);
		wrefresh(leftWin);
	}
};

int main() {
	World earth;
	Screen screen(&earth);
	char c;
	while ((c=getchar())!='Q') {
		switch (c) {
			case ',': earth.Move(NORTH); screen.print(); break;
			case 'o': earth.Move(SOUTH); screen.print(); break;
			case 'e': earth.Move(EAST ); screen.print(); break;
			case 'a': earth.Move(WEST ); screen.print(); break;
		}
	}
}
