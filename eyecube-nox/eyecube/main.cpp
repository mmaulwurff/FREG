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
	DWARF
};
enum dirs { NORTH, SOUTH, EAST, WEST, UP, DOWN };

class Block {
	protected:
	subs id;
	bool movable;
	public:
	bool Movable() { return movable; }
	subs Id() { return id; }
	void* operator new(size_t, subs n) {
		Block * nblock=::new Block;
		nblock->id=n;
		nblock->movable=false;
		return nblock;
	}
};

class Dwarf : private Block {
	int health;
	public:
	void *operator new(size_t) {
		Dwarf *ndwarf=::new Dwarf;
		ndwarf->id=DWARF;
		ndwarf->movable=true;
		ndwarf->health=1;
		return ndwarf;
	}
};

class World {
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	unsigned short playerX, playerY, playerZ;
	long longitude, latitude;
	void *CreateBlock(subs id) {
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
	}
	void Reload() {
		long i, j;
		unsigned short k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
			for (k=0; k<height; ++k)
				DeleteBlock(blocks[i][j][k]);
		for (i=longitude-1; i<=longitude+1; ++i)
		for (j=latitude-1;  j<=latitude+1;  ++j)
			LoadShred(longitude, latitude, (i-longitude+1)*shred_width, (j-latitude+1)*shred_width);
		blocks[playerX][playerY][playerZ] =(Block*)( playerP=(Dwarf*)CreateBlock(DWARF) );
		blocks[shred_width*2-5][shred_width*2-5][height/2]=(Block*)CreateBlock(DWARF);
	}
	public:
	int Move(int i, int j, int k, dirs dir) {
		if (blocks[i][j][k]==NULL)
			return 1;
		else if ( !blocks[i][j][k]->Movable() ||
				(!i && dir==WEST ) ||
				(!j && dir==NORTH) ||
				(!k && dir==DOWN ) ||
				(i==shred_width*3-1 && dir==EAST ) ||
				(j==shred_width*3-1 && dir==SOUTH) ||
				(k==height-1 && dir==UP) )
			return 0;
		else {
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
						--latitude;
						Reload();
					} else if (playerX==shred_width*2) {
						playerX-=shred_width;
						++latitude;
						Reload();
					} else if (playerY==shred_width-1) {
						playerY+=shred_width;
						--longitude;
						Reload();
					} else if (playerY==shred_width*2) {
						playerY-=shred_width;
						++longitude;
						Reload();
					}
				}
				Block *temp=blocks[i][j][k];
				blocks[i][j][k]=blocks[newi][newj][newk];
				blocks[newi][newj][newk]=temp;
				return 1;
			} else return 0;
		}
	}
	void Move(dirs dir) { Move(playerX, playerY, playerZ, dir); }
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
	}
	~World() {
		FILE * file=fopen("world.txt", "w");
		if ("file!=NULL") {
			fprintf(file, "longitude: %ld\nlatitude: %ld\nplayerX: %hd\nplayerY: %hd\nplayerZ: %hd\n",
					longitude, latitude, playerX, playerY, playerZ);
			fclose(file);
		}
		//save shreds
		unsigned short i, j, k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j)
			for (k=0; k<height; ++k)
				DeleteBlock(blocks[i][j][k]);
	}
};

class Screen {
	World * const w; //connected world
	char CharName(int i, int j, int k) {
		switch ( w->Id(i, j, k) ) {
			case STONE: return '#';
			case SOIL:  return 's';
			case DWARF: return '@';
			default: return '?';
		}
	}
	public:
	Screen(World *wor) : w(wor) {}
	void print() {//real print() will be written with ncurses
		unsigned short i, j, k;
		for ( j=0; j<shred_width*3; ++j, printf("\n") )
		for ( i=0; i<shred_width*3; ++i )
			for (k=height-1; k>=0; --k)
				if ( !w->Transparent(i, j, k) ) {
					printf( "%c", CharName(i, j, k) );
					break;
				}
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
