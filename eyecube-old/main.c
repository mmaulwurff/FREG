/*This file is part of Eyecube.
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

#include "header.h"
#include <pthread.h>
#include <ncurses.h>
//#include <locale.h>

extern short view;

WINDOW *world,
       *textwin,
       *pocketwin,
       *sound_window;
void tolog();

//parallel thread
void *mech(void *vptr_args) {
	tolog("mech start\n");
	void allmech();
	while (1) {
		sleep(1);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		//all mech and animal functions
		if (VIEW_MENU!=view) allmech();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
	tolog("mech finish\n");
}

void main() {
	tolog("main start\n");
	void      keytogame(), keytoinv(),
	          map(), notify(), sounds_print(),
	          loadgame(),
	          eraseanimals(),
	          *mech();
	int       ch;
	pthread_t mechthread;
	printf("\nEyecube\nCopyright (C) 2011 Alexander Kromm\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; see README for details.\n\n");
	(void)set_escdelay(10);
//	signal='w';
	(void)initscr();
	(void)start_color();
//	(void)cbreak();
	(void)raw();
	(void)noecho();
	(void)keypad(stdscr, TRUE);
//	setlocale(LC_ALL, "ru_Ru.utf8");
	curs_set(0); //make cursor invisible
	(void)init_pair(WHITE_BLUE,   COLOR_WHITE,  COLOR_BLUE  ); //player, sky
	(void)init_pair(BLACK_GREEN,  COLOR_BLACK,  COLOR_GREEN ); //grass, dwarf
	(void)init_pair(BLACK_WHITE,  COLOR_BLACK,  COLOR_WHITE ); //stone, skin
	(void)init_pair(RED_YELLOW,   COLOR_RED,    COLOR_YELLOW); //sun, fire1
	(void)init_pair(RED_WHITE,    COLOR_RED,    COLOR_WHITE ); //chiken
	(void)init_pair(WHITE_BLACK,  COLOR_WHITE,  COLOR_BLACK ); //?, heap
	(void)init_pair(YELLOW_RED,   COLOR_YELLOW, COLOR_RED   ); //fire2
	(void)init_pair(BLACK_RED,    COLOR_BLACK,  COLOR_RED   ); //pointer
	(void)init_pair(BLACK_YELLOW, COLOR_BLACK,  COLOR_YELLOW); //wood
	(void)init_pair(BLUE_YELLOW,  COLOR_BLUE,   COLOR_YELLOW); //clock
	(void)init_pair(WHITE_CYAN,   COLOR_WHITE,  COLOR_CYAN  ); //noon sky
	(void)init_pair(BLACK_BLUE,   COLOR_BLACK,  COLOR_BLUE  ); //raven
	(void)init_pair(BLACK_CYAN,   COLOR_BLACK,  COLOR_CYAN  ); //raven
	(void)init_pair(RED_BLUE,     COLOR_RED,    COLOR_BLUE  ); //bird
	(void)init_pair(RED_CYAN,     COLOR_RED,    COLOR_CYAN  ); //bird
	(void)init_pair(RED_BLACK,    COLOR_RED,    COLOR_BLACK ); //bird
	world=       newwin(23, 44, 0,  0);
	pocketwin=   newwin(1,  44, 23, 0);
	textwin=     newwin(5,  36, 24, 8);
	sound_window=newwin(5,   8, 24, 0);
	(void)refresh();
	loadgame();
	//start parallel thread
	(void)pthread_create(&mechthread, NULL, mech, NULL);
	map();
	sounds_print();
	notify("Game started.", 0);
	//this is the game itself
	while ((ch=getch())!=(int)'Q') {
		switch (view) {
			case VIEW_SURFACE: case VIEW_FLOOR: case VIEW_HEAD: case VIEW_SKY:
		       	case VIEW_FRONT: keytogame(ch);   break;
			case VIEW_MENU:  key_to_menu(ch); break;
			default: keytoinv(ch); break;
		}
//		sleep(1);
		usleep(10000);
	}
	//stop parallel thread
	eraseanimals(1);
	(void)pthread_cancel(mechthread);
	(void)delwin(world    );
	(void)delwin(textwin  );
	(void)delwin(pocketwin);
	(void)endwin();
	tolog("main finish\n");
}
