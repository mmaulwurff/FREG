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
#include <QtGlobal>

#ifdef Q_OS_WIN32
	#include <windows.h>
	#define usleep(n) { Sleep(n/1000); }
#else
	#include <unistd.h>
#endif

const unsigned short shred_width=16;
const unsigned short height=100;

const unsigned short note_length=144;

const unsigned short inventory_size=26;

//num_str in Screen::PrintInv must be big enough
const unsigned short max_stack_size=9;

const unsigned short time_steps_in_sec=10;
const unsigned short seconds_in_hour=60;
const unsigned short seconds_in_day=24*seconds_in_hour;
const unsigned short end_of_night  = 6*seconds_in_hour;
const unsigned short end_of_morning=12*seconds_in_hour;
const unsigned short end_of_noon   =18*seconds_in_hour;
const unsigned short end_of_evening= 0*seconds_in_hour;
const unsigned short seconds_in_night=end_of_night;
const unsigned short seconds_in_daylight=seconds_in_day-end_of_night;

const unsigned char max_light_radius=10;

const unsigned short max_durability=100;
const unsigned short max_breath=60;

enum dirs {
	HERE,
	NORTH,
	NORTH_EAST,
	EAST,
	SOUTH_EAST,
	SOUTH,
	SOUTH_WEST,
	WEST,
	NORTH_WEST,
	UP,
	DOWN
};

enum { NOT_MOVABLE, MOVABLE, ENVIRONMENT };

enum times_of_day { MORNING, NOON, EVENING, NIGHT };

enum damage_kinds {
	MINE,
	DIG,
	CUT,
	THRUST,
	CRUSH,
	HEAT,
	FREEZE,
	MELT,
	ELECTRO,
	HUNGER,
	BREATH,
	EATEN
};

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
	STONE, //keep it first in this list
	MOSS_STONE,
	NULLSTONE,
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
	ROSE,
	AIR //keep it last in this list
};

enum before_move_return { NOTHING, DESTROY };

enum usage_types { NO, OPEN, INNER_ACTION };

enum props {
	KIND,
	SUB,
	DURABILITY,
	TRANSPARENCY,
	MOVABILITY
};

enum actions {
	MOVE,
	TURN_RIGHT,
	TURN_LEFT,
	TURN,
	JUMP,
	OPEN_INVENTORY,
	USE,
	EXAMINE,
	DROP,
	GET,
	WIELD,
	INSCRIBE,
	EAT,
	DAMAGE,
	BUILD,
	CRAFT,
	TAKEOFF
};

//for debugging reasons:
#define eprint(n) { fputs(n"\n", stderr); }

#endif
