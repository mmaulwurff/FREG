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

//#include <iostream>
//using namespace std;

#include <unistd.h>
#include <ncurses.h>

#include "subs.h"

//TODO make variables
const int shred_width=20;
const int height=100;


class player {
	long x, y; //current shred coordinates. server do not need to know where exactly player is.
	//unsigned health : 7; //max 127, but 100 will be used for normal health and 120 for increased health
	//unsigned regen : 2; //regeneration
	
	public:
	player() {x=0; y=0;};
};

class world { //world without physics
	unsigned long time; //global game world time
	unsigned short blocks[shred_width*3][shred_width*3][height];
	
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
	enum {
		WHITE_BLUE=1,
		BLACK_GREEN,
		BLACK_WHITE,
		RED_YELLOW,
		RED_WHITE,
		WHITE_BLACK,
		YELLOW_RED,
		BLACK_RED,
		BLACK_YELLOW,
		BLUE_YELLOW,
		WHITE_CYAN,
		BLACK_BLUE,
		BLACK_CYAN,
		RED_BLUE,
		RED_CYAN,
		RED_BLACK,
	};
	enum {
		VIEW_SURFACE,
		VIEW_FLOOR,
		VIEW_HEAD,
		VIEW_SKY,
		VIEW_FRONT,
		VIEW_MENU,
		VIEW_INVENTORY,
		VIEW_CHEST,
		VIEW_WORKBENCH,
		VIEW_FURNACE
	};
	WINDOW * world, * pocketwin, * textwin, * sound_window;
	unsigned short view;
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
		world=newwin(23, 44, 0, 0);
		pocketwin=newwin(1, 44, 23, 0);
		textwin=newwin(5, 36, 24, 8);
		sound_window=newwin(5, 8, 24, 0);
		refresh();
	}
	void map() {};
	void notify() {};
	void refresh() {
		wrefresh(world);
	}
	~screen() {
		delwin(world);
		delwin(textwin);
		delwin(pocketwin);
		delwin(sound_window);
		endwin();
	}
};

int main(int argc, char *argv[]) {
	player mole;
	world earth;
	screen scr;
	int ch;
	while ((ch=getch())!='Q') {
//		proc_key(ch);
		usleep(10000);
	}
}
