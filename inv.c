#include "header.h"

extern struct item {
	short what,
	      num;
} inv[][3];
extern short view,
             cloth[];
extern WINDOW *world,
              *pocketwin;

void map(),
     pocketshow();

short cur[5];

void mark(x, y, wind) 
short x, y;
WINDOW *wind; {
	(void)mvwprintw(wind, y, x-1, ">");
	(void)mvwprintw(wind, y, x+2, "<");
}

void keytoinv(key)
int key; {
	short pocketflag=0;
	switch (key) {
		case 'u': view=0; pocketflag=1; break; //surface view
		case 'f': view=1; pocketflag=1; break; //floor view
		case 'h': view=2; pocketflag=1; break; //head viw
		case 'k': view=3; pocketflag=1; break; //sky view
		case 'r': view=4; pocketflag=1; break; //front view
	}
	map();
	if (pocketflag) pocketshow();
}

void invview() {
	short i, j;
	//left arm
	wattrset(world, COLOR_PAIR(3));
	(void)mvwprintw(world, 4, 5, "ww");
	//right arm
	if (inv[cloth[4]][2].what) (void)mvwprintw(world, 4, 1, "%c%d",
			getname(cloth[4], 2, HEAVEN+3), inv[cloth[4]][2].num);
	else (void)mvwprintw(world, 4, 1, "ww");
	//shoulders
	if (cloth[1]) {
		char name=getname(1, 0, HEAVEN+2);
		(void)mvwprintw(world, 3, 1, "%c%c  %c%c", name, name, name, name);
	} else {
		wattrset(world, COLOR_PAIR(1));
		(void)mvwprintw(world, 3, 1, "      ");
	}
	for (i=0; i<=3; ++i)
		if (cloth[i]) {
			char name=getname(i, 0, HEAVEN+2);
			(void)mvwprintw(world, 2+i, 3, "%c%c", name, name);
		} else switch (i) {
			case 0: //head
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 2, 3, "''");
				break;
			case 1:	case 2: //body & legs
				wattrset(world, COLOR_PAIR(1));
				(void)mvwprintw(world, 2+i, 3, "  ");
				break;
			case 3: //feet
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 5, 3, "db");
				break;
		}
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+19+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(j, i, HEAVEN+3), inv[j][i].num);
/*	switch (view) {
		default: //5 - normal
			for (i=0; i<=1)
	}*/
	(void)wclear(pocketwin);
	(void)wrefresh(pocketwin);
}
