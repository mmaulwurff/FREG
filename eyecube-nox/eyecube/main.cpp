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

//#include <ncurses.h>
#include <stdio.h>

const unsigned short shred_width=10;
const unsigned short height=100;
enum subs {
	STONE,
	SOIL,
	PLAYER,
	OTHER_PLAYER
};

class Block {
	subs id;
	public:
	subs Id() { return id; }
	Block() {}
	void* operator new(size_t size) {
		return ::new Block;
	}
	void* operator new(size_t size, subs n) {
		Block * nblock=::new Block;
		nblock->id=n;
		return nblock;
	}
};

class World {
	Block *blocks[shred_width*3][shred_width*3][height];
	public:
	subs Id(int i, int j, int k) { return blocks[i][j][k]->Id(); }
	int Transparent(int i, int j, int k) {
		if (blocks[i][j][k]==NULL)
			return 1;
		else switch ( blocks[i][j][k]->Id() ) {
			default: return 0;
		}
	}
	World() {//generate new world
		//TODO: add load and save
		unsigned short i, j, k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j) {
			for (k=0; k<height/2; ++k)
				blocks[i][j][k]=new (STONE) Block;
			for ( ; k<height; ++k)
				blocks[i][j][k]=NULL;
		}
		blocks[shred_width*2][shred_width*2][(int)height/2]=new (SOIL) Block;
	}
	~World() {
		unsigned short i, j, k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
			for (k=0; k<height; ++k)
				if (blocks[i][j][k]!=NULL)
					delete blocks[i][j][k];
	}
};

class Screen {
	short centerX, centerY;
	World * const w; //connected world
	char CharName(int i, int j, int k) {
		switch ( w->Id(i, j, k) ) {
			case STONE: return '#';
			case SOIL:  return 's';
			default: return '?';
		}
	}
	public:
	Screen(World *wor) : w(wor), centerX(shred_width*1.5), centerY(shred_width*1.5) {}
	void print() {//real print() will be written with ncurses
		unsigned short i, j, k;
		for ( i=0; i<shred_width*3; ++i, printf("\n") )
		for ( j=0; j<shred_width*3; ++j )
			for (k=height-1; k>=0; --k)
				if ( !w->Transparent(i, j, k) ) {
					printf( "%c_", CharName(i, j, k) );
					break;
				}
	}
};

int main() {
	World earth;
	Screen screen(&earth);
	screen.print();
}
