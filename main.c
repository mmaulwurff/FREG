/*This file is part of Eyecube.
*
* Eyecube is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Eyecube is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Eyecube. If not, see <http://www.gnu.org/licenses/>.
*/

#include "header.h"
#include <pthread.h>
#include <ncurses.h>
//#include <locale.h>

extern char view;

char signal='w';
WINDOW *world,
       *textwin,
       *pocketwin,
       *sound_window;
void tolog();

//parallel thread
void *mech(void *vptr_args) {
	void allmech();
	while (1) {
		sleep(1);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		//all mech and animal functions
		if ('m'!=view) allmech();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

void main() {
	tolog("main start\n");
	void keytogame(),
	     keytoinv(),
	     eraseanimals(),
	     loadgame(),
	     map(),
	     sounds_print(),
	     notify(),
	     *mech();
	int       ch;
	pthread_t mechthread;
	(void)set_escdelay(10);
//	signal='w';
	//start parallel thread
	(void)pthread_create(&mechthread, NULL, mech, NULL);
	(void)initscr();
	(void)start_color();
	(void)cbreak();
	(void)noecho();
	(void)keypad(stdscr, TRUE);
//	setlocale(LC_ALL, "ru_Ru.utf8");
	curs_set(0); //make cursor invisible
	(void)init_pair(1,  COLOR_WHITE,  COLOR_BLUE  );  //player, sky
	(void)init_pair(2,  COLOR_BLACK,  COLOR_GREEN );  //grass, dwarf
	(void)init_pair(3,  COLOR_BLACK,  COLOR_WHITE );  //stone, skin
	(void)init_pair(4,  COLOR_RED,    COLOR_YELLOW);  //sun, fire1
	(void)init_pair(5,  COLOR_RED,    COLOR_WHITE );  //chiken
	(void)init_pair(6,  COLOR_WHITE,  COLOR_BLACK );  //?, heap
	(void)init_pair(7,  COLOR_YELLOW, COLOR_RED   );  //fire2
	(void)init_pair(8,  COLOR_BLACK,  COLOR_RED   );  //pointer
	(void)init_pair(9,  COLOR_BLACK,  COLOR_YELLOW);  //wood
	(void)init_pair(10, COLOR_BLUE,   COLOR_YELLOW);  //clock
	world=       newwin(23, 44, 0,  0);
	pocketwin=   newwin(1,  44, 23, 0);
	textwin=     newwin(5,  36, 24, 8);
	sound_window=newwin(5,   8, 24, 0);
	(void)refresh();
	loadgame();
	map();
	sounds_print();
	notify("Game started.", 0);
	//this is the game itself
	while ((ch=getch())!=(int)'Q')
		switch (view) {
			case 'u': case 'f': case 'h': case 'k': case 'r':
			keytogame(ch);
			break;
			case 'm': key_to_menu(ch); break;
			default : keytoinv(ch); break;
		}
	//stop parallel thread
	eraseanimals();
	(void)pthread_cancel(mechthread);
	(void)delwin(world    );
	(void)delwin(textwin  );
	(void)delwin(pocketwin);
	(void)endwin();
	tolog("main finish\n");
}
