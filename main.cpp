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

#include <cstdio>
#include <string.h>
#include <cstdlib>
#include <curses.h>

const unsigned short shred_width=10;
const unsigned short height=100;

const unsigned short full_name_length=20;
const unsigned short note_length=144;

const unsigned short inventory_size=26;
const unsigned short max_stack_size=9; //num_str in Screen::PrintInv must be big enough

const unsigned short seconds_in_hour=60;
const unsigned short seconds_in_day=24*seconds_in_hour;
const unsigned short end_of_night  = 6*seconds_in_hour;
const unsigned short end_of_morning=12*seconds_in_hour;
const unsigned short end_of_noon   =18*seconds_in_hour;

const unsigned short max_light_radius=5;

const unsigned short max_durability=10;

inline void WriteName(char * str, const char * name) { strncpy(str, name, full_name_length); }

enum color_pairs { //do not change colors order! //foreground_background
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
enum dirs { HERE, NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST, UP, DOWN };

enum { NOT_MOVABLE, MOVABLE, ENVIRONMENT };

enum window_views { NORMAL, FRONT, INVENTORY };

enum times_of_day { MORNING, NOON, EVENING, NIGHT };

enum kinds {//kind of atom
	BLOCK,
	BELL,
	CHEST,
	PILE,
	DWARF,
	ANIMAL,
	PICK,
	TELEGRAPH
};
enum subs {//substance block is made from
	STONE,
	NULLSTONE,
	AIR, //though there is no air block.
	SKY,
	STAR,
	SUN_MOON,
	SOIL,
	H_MEAT, //hominid meat
	A_MEAT, //animal meat
	GLASS,
	WOOD,
	DIFFERENT,
	IRON
};

enum usage_types { NO, OPEN, INNER_ACTION };

class Block;
class World;
void BlockFromFile(FILE *, Block * &, World * =NULL, unsigned short=0, unsigned short=0, unsigned short=0);

#include "blocks.h"

void BlockFromFile(FILE * in, Block * & block, World * world,
		unsigned short i, unsigned short j, unsigned short k) {
	char str[300];
	fgets(str, 300, in);
	int kind;
	sscanf(str, "%d", &kind);
	switch(kind) {
		case BLOCK:     block=new Block(str); break;
		case TELEGRAPH: block=new Telegraph(str); break;
		case PICK:      block=new Pick(str); break;
		case DWARF:     block=new Dwarf(world, i, j, k, str, in); break;
		case CHEST:     block=new Chest(str, in); break;
		case PILE:      block=new Pile(world, i, j, k, str, in); break;
		default:        block=NULL;
	}
}

#include "screen.h"

#include "world.h"

#include "world-func.h"

#include "blocks-func.h"

#include "screen-func.h"

int main() {
	World earth;
	Screen screen(&earth);
	int c;
	int print_flag=1; //print only if needed, needed nearly everytime
	while ( 'Q'!=(c=getch()) ) {
		switch (c) {
			case KEY_UP:    earth.PlayerMove(NORTH); break;
			case KEY_DOWN:  earth.PlayerMove(SOUTH); break;
			case KEY_RIGHT: earth.PlayerMove(EAST ); break;
			case KEY_LEFT:  earth.PlayerMove(WEST ); break;
			case ' ': earth.PlayerJump(); break;
			case 'w': earth.SetPlayerDir(WEST);  break;
			case 'e': earth.SetPlayerDir(EAST);  break;
			case 's': earth.SetPlayerDir(SOUTH); break;
			case 'n': earth.SetPlayerDir(NORTH); break;
			case 'v': earth.SetPlayerDir(DOWN);  break;
			case '^': earth.SetPlayerDir(UP);    break;
			case 'i':
				if (INVENTORY!=screen.viewRight) {
					screen.viewRight=INVENTORY;
					screen.blockToPrintRight=(Block *)(earth.GetPlayerP());
				} else
					screen.viewRight=FRONT;
			break;
			case '\n': {
				int i, j, k;
				earth.PlayerFocus(i, j, k);
				earth.Use(i, j, k);
			} break;
			case '?': {
				int i, j, k;
				earth.PlayerFocus(i, j, k);
				char str[full_name_length+note_length+20];
				earth.FullName(str, i, j, k);
				earth.GetNote(str, i, j, k);
				screen.Notify(str);
			} break;
			case 'd': earth.PlayerDrop(getch()-'a'); break;
			case 'g': earth.PlayerGet(getch()-'a'); break;
			case 'W': earth.PlayerWield(); break;
			case 'I': earth.PlayerInscribe(); break;
			case KEY_BACKSPACE: {
				int i, j, k;
				earth.PlayerFocus(i, j, k);
				earth.Damage(i, j, k);
			} break;
			case KEY_HOME: earth.PlayerBuild(getch()-'a'); break;
			default: screen.Notify("What?\n");
		}
		if (print_flag) screen.Print();
	}
}
