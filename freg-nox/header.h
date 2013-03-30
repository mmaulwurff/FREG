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
	//#define usleep(n) { Sleep(n/1000); }
#else
	#include <unistd.h>
#endif

const unsigned short shred_width=16;
const unsigned short height=128;

const unsigned short note_length=144;

const unsigned short seconds_in_hour=60;
const unsigned short seconds_in_day=24*seconds_in_hour;
const unsigned short end_of_night  = 6*seconds_in_hour;
const unsigned short end_of_morning=12*seconds_in_hour;
const unsigned short end_of_noon   =18*seconds_in_hour;
const unsigned short end_of_evening= 0*seconds_in_hour;
const unsigned short seconds_in_night=end_of_night;
const unsigned short seconds_in_daylight=seconds_in_day-end_of_night;

const unsigned char max_light_radius=15;

const unsigned short max_durability=100;
const unsigned short max_breath=60;

enum dirs {
	UP,
	DOWN,
	NORTH,
	SOUTH,
	EAST,
	WEST,
	NORTH_EAST,
	SOUTH_EAST,
	SOUTH_WEST,
	NORTH_WEST,
	HERE
}; //enum dirs

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
	EATEN,
	TIME,
	NO_HARM
}; //enum damage_kinds

enum kinds {//kind of atom
	//do not change order, or rewrite craft recipes.
	//add new kinds to bottom.
	BLOCK, ///<0
	BELL, ///<1
	CHEST, ///<2
	PILE, ///<3
	DWARF, ///<4
	ANIMAL, ///<5
	PICK, ///<6
	TELEGRAPH, ///<7
	LIQUID, ///<8
	GRASS, ///<9
	BUSH, ///<10
	RABBIT, ///<11
	ACTIVE, ///<12
	CLOCK, ///<13
	PLATE, ///<14
	WORKBENCH, ///<15
	WEAPON, ///<16
	LADDER, ///<17
	DOOR ///<18
}; //enum kinds
enum subs {//substance block is made from
	//do not change order, or rewrite craft recipes.
	//add new substances right before air.
	STONE,      ///<0
	MOSS_STONE, ///<1
	NULLSTONE,  ///<2
	SKY,        ///<3
	STAR,       ///<4
	SUN_MOON,   ///<5
	SOIL,       ///<6
	H_MEAT,     ///<7 (hominid meat)
	A_MEAT,     ///<8 (animal meat)
	GLASS,      ///<9
	WOOD,       ///<10
	DIFFERENT,  ///<11
	IRON,       ///<12
	WATER,      ///<13
	GREENERY,   ///<14
	SAND,       ///<15
	HAZELNUT,   ///<16
	ROSE,       ///<17
	AIR //keep it last in this list
}; //enum subs

enum before_move_return { NOTHING, DESTROY };
enum before_push_action {
	NO_ACTION,
	MOVE_UP,
	JUMP,
	DAMAGE
}; //enum before_push_action

enum usage_types { NO, OPEN, INNER_ACTION };

enum transparency {
	BLOCK_OPAQUE,
	BLOCK_TRANSPARENT,
	INVISIBLE,
	NONSTANDARD=6,
	UNDEF
}; //enum transparency

#endif
