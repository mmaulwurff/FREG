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
enum weathers {
	CLEAR,
	RAIN,
	SNOW,
	HAIL,
	FROGS
};

const int shred_width=20;//TODO make variables
const int height=100;
const int screen_width=21;
const char skymap[screen_width][screen_width+1]={
	"   ..  .. .      ..  ",
	"       .        .    ",
	"   .     .   .   .   ",
	" ..     .   .    .  .",
	"     . . .     . ..  ",
	" .. .  .    .    .   ",
	" .              .    ",
	"   .          .      ",
	"     .   ..    .     ",
	".        .      ..   ",
	" .       .       .   ",
	"                     ",
	" .      .   .     .  ",
	"       ...  .        ",
	"      .     .   .    ",
	".  ....       ..   ..",
	"          .        . ",
	"   .    ..  . .    ..",
	"  .      .           ",
	"              .     .",
	".      .   ...  .   ."
};
const int prec_prob=20;

int random_prob(short prob) {
	FILE *file=fopen("/dev/urandom", "rb");
	char c=fgetc(file);
	fclose(file);
	return (128+c<prob*2.55) ? 1 : 0;
}

class player {
	long planet_x, planet_y; //current shred coordinates
	//unsigned health : 7; //max 127, but 100 will be used for normal health and 120 for increased health
	//unsigned regen : 2; //regeneration
	public:
	unsigned short x, y, z; 
	player() {
		planet_x=0;
		planet_y=0;
		x=30;
		y=30;
		z=height/2;
	};
};

class world { //world without physics
	enum subs {
		AIR,
		STONE,
		SOIL
	} blocks[shred_width*3][shred_width*3][height];
	//there isn't the only way for light from one point to another
	//this finds one of them, and it is not the straight line.
	//it works fast
	int visible3(short x, short y, short z,
	             short xtarget, short ytarget, short ztarget) {
		if (x==xtarget && y==ytarget && z==ztarget) return 1;
		else if (!transparent(x, y, z)) return 0;
		else {
			if (x!=xtarget) x+=(xtarget>x) ? 1 : -1;
			if (y!=ytarget) y+=(ytarget>y) ? 1 : -1;
			if (z!=ztarget) z+=(ztarget>z) ? 1 : -1;
			return visible3(x, y, z, xtarget, ytarget, ztarget);
		}
	};
	public:
	unsigned long time; //global game world time
	weathers weather;
	int visible2x3(short x1, short y1, short z1, short x2, short y2, short z2) {
		short savecor;
		if (visible3(x1, y1, z1, x2, y2, z2)) return 1;
		else if ((x2!=x1 && transparent(x2+(savecor=(x2>x1) ? (-1) : 1), y2, z2)
			&& visible3(x1, y1, z1, x2+savecor, y2, z2)) ||
			(y2!=y1 && transparent(x2, y2+(savecor=(y2>y1) ? (-1) : 1), z2)
			&& visible3(x1, y1, z1, x2, y2+savecor, z2)) ||
			(z2!=z1 && transparent(x2, y2, z2+(savecor=(z2>z1) ? (-1) : 1))
			&& visible3(x1, y1, z1, x2, y2, z2+savecor))) return 1;
		else return 0;
	};
	inline int name(short x, short y, short z) {
		switch(blocks[x][y][z]) {
			case SOIL : return '|';
			case STONE: return '#';
			default   : return '?';
		}
	};
	inline int transparent(short x, short y, short z) {
		switch(blocks[x][y][z]) {
			case AIR: return 1;
			default : return 0;
		}
	};
	inline color_pairs color(short x, short y, short z) {
		switch (blocks[x][y][z]) {
			case SOIL:  return BLACK_GREEN;
			case STONE: return BLACK_WHITE;
			default:    return WHITE_BLACK;
		}
	};
	world() {
		//TODO connect, get map, rewrite all costructor
		time=1*60;
		weather=FROGS;
		unsigned short i, j, k;
		for (i=0; i<shred_width*3; ++i)
		for (j=0; j<shred_width*3; ++j) {
			for (k=0; k<height/4; ++k) blocks[i][j][k]=STONE;
			for (   ; k<height/2; ++k) blocks[i][j][k]=SOIL;
			for (   ; k<height;   ++k) blocks[i][j][k]=AIR;
		}
		blocks[shred_width+10][shred_width+7][height/2]=STONE;
		blocks[shred_width+15][shred_width+9][height/2]=STONE;
		blocks[shred_width+9][shred_width+9][height/2]=STONE;
		blocks[shred_width+15][shred_width+17][height/2]=STONE;
	};
	void proc_key(int ch) {};
};

class screen {
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
	} right_view;
	enum left_views {
		VIEW_SKY,
		VIEW_EARTH,
		VIEW_MIDDLE
	} left_view;
	WINDOW * world_left_win, * world_right_win, * pocket_win, * text_win, * sound_win;
	void print_env(WINDOW * print_win, const player & P, world & W) {
		short x, y, z,
		      * i, * j, * k,
		      idir, jdir, kdir,
		      kbound, coor;
		if (world_left_win==print_win) {//set variables
			i=&(x=P.x-10);
			j=&(y=P.y-10);
			idir=1;
			jdir=1;
			coor=P.z;
			if (VIEW_SKY==left_view) {
				k=&(z=P.z+1);
				kdir=1;
				kbound=height;
			} else {
				k=&(z=height-1);
				kdir=-1;
				kbound=-1;
			}
		} else {
			j=&(z=P.z+19);
			jdir=-1;
			switch (right_view) {
				case VIEW_NORTH:
					i=&(x=P.x-10);
					idir=1;
					coor=P.y;
					k=&(y=P.y-1);
					kdir=-1;
					kbound=-1;
				break;
				case VIEW_SOUTH:
					i=&(x=P.x+10);
					idir=-1;
					coor=P.y;
					k=&(y=P.y+1);
					kdir=1;
					kbound=shred_width*3;
				break;
				case VIEW_WEST:
					i=&(y=P.y+10);
					idir=-1;
					coor=P.x;
					k=&(x=P.x-1);
					kdir=-1;
					kbound=-1;
				break;
				case VIEW_EAST:
					i=&(y=P.y-10);
					idir=1;
					coor=P.x;
					k=&(x=P.x+1);
					kdir=1;
					kbound=shred_width*3;
				break;
			}
		}
		unsigned short scrx, scry, ksave=*k, jsave=*j;
		for (scrx=1;           scrx<=screen_width*2-1; scrx+=2, *i+=idir)
		for (scry=1, *j=jsave; scry<=screen_width;   ++scry,    *j+=jdir) {
			for (*k=ksave ; *k!=kbound; *k+=kdir)
				if ( !W.transparent(x, y, z) ) {
					if ( W.visible2x3(P.x, P.y, P.z, x, y, z) ) {
						char ch;
						//set second character
						if (world_left_win==print_win) {
							if (*k<coor-2) ch='!';
							else if (*k==coor-2) ch='_';
							else if (*k==coor-1) ch=' ';
							else if (*k<coor+9)
								ch='0'+*k-coor+1;
							else ch='+';
						} else {
							if (*k==coor+kdir) ch=' ';
							else if (kdir*(*k-coor)<11)
								ch='0'+kdir*(*k-coor)-1;
							else ch='+';
						}
						wattrset(print_win,
							COLOR_PAIR(W.color(x, y, z)));
						mvwprintw(print_win, scry, scrx, "%c%c",
							W.name(x, y, z), ch);
					} else {
						wstandend(print_win);
						mvwprintw(print_win, scry, scrx, "  ");
					}
					break;
				}
			if (*k==kbound)
				if (world_left_win==print_win) {
					if (W.weather!=CLEAR && random_prob(prec_prob))
						switch (W.weather) {
							case RAIN:
				wattrset(print_win, (W.time%(24*60)<6*60) ?
					COLOR_PAIR(BLUE_BLACK) : COLOR_PAIR(BLUE_CYAN));
				mvwprintw(print_win, scry, scrx, ", ");
							break;
							case SNOW:
				wattrset(print_win, (W.time%(24*60)<6*60) ?
					COLOR_PAIR(WHITE_BLACK) : COLOR_PAIR(WHITE_CYAN));
				mvwprintw(print_win, scry, scrx, "* ");
							break;
							case HAIL:
				wattrset(print_win, (W.time%(24*60)<6*60) ?
					COLOR_PAIR(WHITE_BLACK) : COLOR_PAIR(WHITE_CYAN));
				mvwprintw(print_win, scry, scrx, "o ");
							break;
							case FROGS:
				wattrset(print_win, (W.time%(24*60)<6*60) ?
					COLOR_PAIR(GREEN_BLACK) : COLOR_PAIR(GREEN_CYAN));
				mvwprintw(print_win, scry, scrx, "f ");
						}
					else if (W.time%(24*60)<6*60) {
						wattrset(print_win,
							COLOR_PAIR(WHITE_BLACK));
						mvwprintw(print_win, scry, scrx, "%c ",
							skymap[scry-1][scrx/2-1]);
					} else {
						wattrset(print_win,
							COLOR_PAIR(WHITE_CYAN));
						mvwprintw(print_win, scry, scrx, "  ");
					}
				} else {
					wattrset(print_win, COLOR_PAIR(BLACK_WHITE));
					mvwprintw(print_win, scry, scrx, "  ");
				}
				//print sun/moon
		}
		if (world_left_win==print_win)//print player
			if (left_view==VIEW_SKY) {
				wstandend(print_win);
				mvwprintw(print_win, screen_width/2+1, 0, ">");
				mvwprintw(print_win, screen_width/2+1,
					screen_width*2+1, "<");
				mvwprintw(print_win, screen_width+1, screen_width, "^^");
				mvwprintw(print_win, 0, screen_width, "vv");
			} else {
				char ch;
				switch (right_view) {
					case VIEW_NORTH: ch='^'; break;
					case VIEW_SOUTH: ch='v'; break;
					case VIEW_WEST:  ch='<'; break;
					case VIEW_EAST:  ch='>'; break;
					default: ch=' '; break;
				}
				wattrset(print_win, COLOR_PAIR(WHITE_BLUE));
				mvwprintw(print_win, screen_width/2+1, screen_width,
					"@%c", ch);
			}
		else {//print player locating arrows
			wstandend(print_win);
			mvwprintw(print_win, screen_width-1,            0, ">");
			mvwprintw(print_win, screen_width-1, screen_width*2+1, "<");
			mvwprintw(print_win, screen_width+1,   screen_width, "^^");
		}
	};
	public:
	screen() {
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
		world_left_win =newwin(23, 44,  0,  0);
		world_right_win=newwin(23, 44,  0, 44);
		pocket_win     =newwin( 1, 44, 23,  0);
		text_win       =newwin( 5, 36, 24,  8);
		sound_win      =newwin( 5,  8, 24,  0);
		right_view=VIEW_EAST;
		left_view=VIEW_EARTH;
		refresh();
	}
	void map(const player & P, world & W) {
		wclear(world_left_win);
		wclear(world_right_win);
		//
		wstandend(world_left_win);
		wstandend(world_right_win);
		//
		box(world_left_win,  0, 0);
		box(world_right_win, 0, 0);
		//
		print_env(world_left_win,  P, W);
		print_env(world_right_win, P, W);
		//
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

int main(int argc, char *argv[]) {
	player mole;
	world earth;
	screen scr;
	scr.map(mole, earth);
	int ch;
	while ((ch=getch())!='Q') {
		//proc_key(ch);
		scr.notify("hollo");
		scr.map(mole, earth);
		usleep(10000);
	}
}
