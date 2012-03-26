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
#include <unistd.h>

#define DEBUG 1

enum subs {
	AIR,
	STONE,
	SOIL
};
enum right_views {
	VIEW_NORTH,
	VIEW_EAST,
	VIEW_SOUTH,
	VIEW_WEST,
	VIEW_MENU,
	VIEW_INVENTORY,
	VIEW_CHEST,
	VIEW_WORKBENCH,
	VIEW_FURNACE
};
enum left_views {
	VIEW_SKY,
	VIEW_EARTH,
	VIEW_MIDDLE
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

const int shred_width=20;//TODO make variables
const int height=100;
const int screen_width=21;

inline int name(subs sub) {
	switch(sub) {
		case SOIL : return '|';
		case STONE: return '#';
		default   : return '?';
	}
};
inline int visible(subs sub) {
	switch(sub) {
		case AIR: return 0;
		default : return 1;
	}
};
inline color_pairs color(subs sub) {
	switch (sub) {
		case SOIL:  return BLACK_GREEN;
		case STONE: return BLACK_WHITE;
		default:    return WHITE_BLACK;
	}
};

class player;
class screen;

class world { //world without physics
	unsigned long time; //global game world time
	subs blocks[shred_width*3][shred_width*3][height];
	friend void print_env(WINDOW *, const player &, const world &, const screen &);
	public:
	world() {
		//TODO connect, get map, rewrite all costructor
		time=0;
		unsigned short i, j, k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j) {
			for (k=0; k<height/4; ++k) blocks[i][j][k]=STONE;
			for (   ; k<height/2; ++k) blocks[i][j][k]=SOIL;
			for (   ; k<height;   ++k) blocks[i][j][k]=AIR;
		}
	};
	void proc_key(int ch) {};
};

class screen {
	right_views right_view;
	left_views left_view;
	WINDOW * world_left_win, * world_right_win, * pocket_win, * text_win, * sound_win;
	friend void print_env(WINDOW *, const player &, const world &, const screen & S);
	public:
	screen() {
		set_escdelay(10);
		initscr();
		start_color();
		raw();
		noecho();
		keypad(stdscr, TRUE);
		curs_set(0);
		init_pair(WHITE_BLUE,   COLOR_WHITE,  COLOR_BLUE  ); //player, sky
		init_pair(BLACK_GREEN,  COLOR_BLACK,  COLOR_GREEN ); //grass, dwarf
		init_pair(BLACK_WHITE,  COLOR_BLACK,  COLOR_WHITE ); //stone, skin
		init_pair(RED_YELLOW,   COLOR_RED,    COLOR_YELLOW); //sun, fire1
		init_pair(RED_WHITE,    COLOR_RED,    COLOR_WHITE ); //chiken
		init_pair(WHITE_BLACK,  COLOR_WHITE,  COLOR_BLACK ); //?, heap
		init_pair(YELLOW_RED,   COLOR_YELLOW, COLOR_RED   ); //fire2
		init_pair(BLACK_RED,    COLOR_BLACK,  COLOR_RED   ); //pointer
		init_pair(BLACK_YELLOW, COLOR_BLACK,  COLOR_YELLOW); //wood
		init_pair(BLUE_YELLOW,  COLOR_BLUE,   COLOR_YELLOW); //clock
		init_pair(WHITE_CYAN,   COLOR_WHITE,  COLOR_CYAN  ); //noon sky
		init_pair(BLACK_BLUE,   COLOR_BLACK,  COLOR_BLUE  ); //raven
		init_pair(BLACK_CYAN,   COLOR_BLACK,  COLOR_CYAN  ); //raven
		init_pair(RED_BLUE,     COLOR_RED,    COLOR_BLUE  ); //bird
		init_pair(RED_CYAN,     COLOR_RED,    COLOR_CYAN  ); //bird
		init_pair(RED_BLACK,    COLOR_RED,    COLOR_BLACK ); //bird
		world_left_win =newwin(23, 44,  0,  0);
		world_right_win=newwin(23, 44,  0, 44);
		pocket_win     =newwin( 1, 44, 23,  0);
		text_win       =newwin( 5, 36, 24,  8);
		sound_win      =newwin( 5,  8, 24,  0);
		right_view=VIEW_NORTH;
		left_view=VIEW_EARTH;
		refresh();
	}
	void map(const player & P, const world & W, const screen & S) {
		wclear(world_left_win);
		wclear(world_right_win);
		wstandend(world_left_win);
		wstandend(world_right_win);
		box(world_left_win, 0, 0);
		box(world_right_win, 0, 0);
		print_env(world_left_win, P, W, S);
		wrefresh(world_left_win);
		wrefresh(world_right_win);
	}
	void notify(const char * n) {
		wclear(text_win);
		wstandend(text_win);
		mvwaddstr(text_win, 1, 1, n);
		box(text_win, 0, 0);
		wrefresh(text_win);
	};
	~screen() {
		delwin(world_left_win);
		delwin(world_right_win);
		delwin(text_win);
		delwin(pocket_win);
		delwin(sound_win);
		endwin();
	}
};

class player {
	long planet_x, planet_y; //current shred coordinates
	unsigned short x, y, z; 
	//unsigned health : 7; //max 127, but 100 will be used for normal health and 120 for increased health
	//unsigned regen : 2; //regeneration
	friend void print_env(WINDOW *, const player &, const world &, const screen &);
	public:
	player() {
		planet_x=0;
		planet_y=0;
		x=30;
		y=30;
		z=height/2;
	};
};
void print_env(WINDOW * print_win, const player & P, const world & W, const screen & S) {
	unsigned short x, y, * i, * j;
	short idir, jdir, kdir, kbound, z, *k;
	if (print_win==S.world_left_win) {
		i=&(x=P.x-10);
		j=&(y=P.y-10);
		idir=1;
		jdir=1;
		if (VIEW_SKY==S.left_view) {
			k=&(z=0);
			kdir=1;
			kbound=height;
		} else {
			k=&(z=height-1);
			kdir=-1;
			kbound=-1;
		}
	}/* else {
		switch (view) {
			case VIEW_NORTH:
				i
			break;
		}
	}*/
	unsigned short scrx, scry, ksave=*k, jsave=*j;
	for (scrx=1;           scrx<=screen_width*2-1; scrx+=2, *i+=idir)
	for (scry=1, *j=jsave; scry<=screen_width;   ++scry,    *j+=jdir) {
		for (*k=ksave ; *k!=kbound; *k+=kdir)
			if ( visible(W.blocks[x][y][z]) ) {
				wattrset(print_win, COLOR_PAIR(color(W.blocks[x][y][z])));
				mvwprintw(print_win, scry, scrx, "%c ",
					name(W.blocks[x][y][z]));
				break;
			}
		//print background
	}
}

int main(int argc, char *argv[]) {
	player mole;
	world earth;
	screen scr;
	int ch;
	while ((ch=getch())!='Q') {
		//proc_key(ch);
		char hel[]="hello";
		scr.notify(hel);
		scr.map(mole, earth, scr);
		//usleep(10000);
	}
}
