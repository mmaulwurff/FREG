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

#ifndef HEADER_H
#define HEADER_H

#include <cstdio>
#include <string.h>
#include <cstdlib>
#include <curses.h>

#ifdef _WIN32
	#include <windows.h>
	void usleep(unsigned int n) { Sleep(n/1000); }
#endif
#ifndef _WIN32
	#include <unistd.h>
#endif

/*internationalization (planned)
#include <libintl.h>

#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
*/

const unsigned short shred_width=10;
const unsigned short height=100;

const unsigned short full_name_length=20;
const unsigned short note_length=144;

const unsigned short inventory_size=26;
const unsigned short max_stack_size=9; //num_str in Screen::PrintInv must be big enough

const unsigned short time_steps_in_sec=10;
const unsigned short seconds_in_hour=60;
const unsigned short seconds_in_day=24*seconds_in_hour;
const unsigned short end_of_night  = 6*seconds_in_hour;
const unsigned short end_of_morning=12*seconds_in_hour;
const unsigned short end_of_noon   =18*seconds_in_hour;
const unsigned short end_of_evening= 0*seconds_in_hour;

const unsigned short max_light_radius=5;

const unsigned short max_durability=100;
const unsigned short max_breath=100;

inline char * WriteName(char * str, const char * name) { strncpy(str, name, full_name_length); return str; }

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
        WHITE_WHITE
};
enum dirs { HERE, NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST, UP, DOWN };

enum { NOT_MOVABLE, MOVABLE, ENVIRONMENT };

enum window_views { NORMAL, FRONT, INVENTORY };

enum times_of_day { MORNING, NOON, EVENING, NIGHT };

enum damage_kinds { MINE, DIG, CUT, THRUST, CRUSH, HEAT, FREEZE, MELT, ELECTRO, HUNGER, BREATH };

enum kinds {//kind of atom
	BLOCK,
	BELL,
	CHEST,
	PILE,
	DWARF,
	ANIMAL,
	PICK,
	TELEGRAPH,
	LIQUID,
	GRASS,
	BUSH,
	RABBIT,
	ACTIVE
};
enum subs {//substance block is made from
	STONE,
	MOSS_STONE,
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
	IRON,
	WATER,
	GREENERY,
	SAND,
	HAZELNUT,
	ROSE
};

enum before_move_return { NOTHING, DESTROY };

enum usage_types { NO, OPEN, INNER_ACTION };

#endif
