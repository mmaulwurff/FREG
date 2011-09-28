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

//#include "header.h"
#include <pthread.h>
#include <ncurses.h>

extern char view;

char signal='w';
WINDOW *world,
       *textwin,
       *pocketwin;

//parallel thread
void *mech(void *vptr_args) {
	void allmech();
	while (1) {
		sleep(1);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		//all mech and animal functions
		allmech();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

void main() {
	void keytogame(),
	     keytoinv(),
	     eraseanimals(),
	     loadgame(),
	     map(),
	     notify(),
	     *mech();
	int  ch;
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
	curs_set(0); //make cursor invisible
	(void)init_pair(1, COLOR_WHITE,  COLOR_BLUE  );  //player, sky
	(void)init_pair(2, COLOR_BLACK,  COLOR_GREEN );  //grass, dwarf
	(void)init_pair(3, COLOR_BLACK,  COLOR_WHITE );  //stone, skin
	(void)init_pair(4, COLOR_RED,    COLOR_YELLOW);  //sun, fire1
	(void)init_pair(5, COLOR_RED,    COLOR_WHITE );  //chiken
	(void)init_pair(6, COLOR_WHITE,  COLOR_BLACK );  //?, heap
	(void)init_pair(7, COLOR_YELLOW, COLOR_RED   );  //fire2
	(void)init_pair(8, COLOR_BLACK,  COLOR_RED   );  //pointer
	(void)init_pair(9, COLOR_BLACK,  COLOR_YELLOW);  //wood
	world=    newwin(24, 44, 0,  0);
	pocketwin=newwin(1,  44, 24, 0);
	textwin=  newwin(6,  44, 25, 0);
	(void)refresh();
	loadgame();
	map();
	notify("Game started.", 0);
	//this is the game itself
	while ((ch=getch())!=(int)'Q')
		if (view=='u' || view=='f' || view=='h' || view=='k' || view=='r')
			keytogame(ch);
		else keytoinv(ch);
	//stop parallel thread
	eraseanimals();
	(void)pthread_cancel(mechthread);
	(void)delwin(world    );
	(void)delwin(textwin  );
	(void)delwin(pocketwin);
	(void)endwin();
}
