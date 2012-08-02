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

#include <cstdio>
#include <string.h>
#include <cstdlib>

const unsigned short shred_width=10;
const unsigned short height=100;
const unsigned short full_name_length=20;
const unsigned short inventory_size=26;
const unsigned short max_stack_num=9; //num_str in Screen::PrintInv must be big enough
const unsigned short seconds_in_hour=60;
const unsigned short seconds_in_day=24*seconds_in_hour;
const unsigned short end_of_night  = 6*seconds_in_hour;
const unsigned short end_of_morning=12*seconds_in_hour;
const unsigned short end_of_noon   =18*seconds_in_hour;

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
enum special_views { NONE, INVENTORY };
enum dirs { HERE, NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST, UP, DOWN };
enum { NOT_MOVABLE, MOVABLE,  GAS };
enum times_of_day { MORNING, NOON, EVENING, NIGHT };
enum kinds {//kind of atom
	BLOCK,
	BELL,
	CHEST,
	DWARF,
	ANIMAL
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
	WOOD
};

#include "blocks.h"

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
			case 'v':       screen.UpDownView(DOWN); break; //earth.SetPlayerDir(DOWN);  break;
			case '^':       screen.UpDownView(UP); break; //earth.SetPlayerDir(UP);    break;
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
