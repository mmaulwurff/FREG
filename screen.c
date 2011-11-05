/*Eyecube, sandbox game.
* Copyright (C) 2011 Alexander Kromm, see README file for details.
*/

#include "header.h"
#include <ncurses.h>
#include <locale.h>

extern WINDOW      *world, *textwin, *pocketwin, *sound_window;
extern short       xp, yp, zp, pl, eye[], earth[][3*WIDTH][HEAVEN+1],
                   cur[], radar_dist;
extern unsigned    time;
extern char        view;
extern struct item inv[][3], cloth[], radar[], *craft;
extern struct something *animalstart;
extern struct for_sky sky[][39];

void tolog();

//global notify system.
void notify(not, noc)
char not[];
short noc; {
	tolog("notify start\n");
//:	setlocale(LC_ALL, "ru_RU.utf8");
	(void)wclear(textwin);
	wstandend(textwin);
	mvwaddstr(textwin, 1, 1, not);
	if (noc) wprintw(textwin, "%d", noc);
//	(void)mvwprintw(textwin, 2, 1, "x: %d", xp);
//	(void)mvwprintw(textwin, 3, 1, "y: %d", yp);
//	(void)mvwprintw(textwin, 4, 1, "z: %d", zp);
	(void)box(textwin, 0, 0);
	(void)wrefresh(textwin);
	tolog("notify finish\n");
}

//this prints pockets contents
void pocketshow() {
	tolog("pocketshow start\n");
	void  mark();
	char  getname();
	short x;
	(void)wclear(pocketwin);
	for (x=0; x<=9; ++x) (void)mvwprintw(pocketwin, 0, 7+x*3, "%c%d",
		getname(inv[x][2].what, pocketwin), inv[x][2].num);
	mark(7+cloth[4].num*3, 0, pocketwin, 'e');
	(void)wrefresh(pocketwin);
	tolog("pocketshow finish\n");
}

//this marks chosen item in inventory and pockets
void mark(x, y, wind, c)
short  x, y;
WINDOW *wind;
char   c; {
	if (c=='e') { //empty
		wattrset(wind, COLOR_PAIR(BLACK_GREEN));
		(void)mvwprintw(wind, y, x-1, " ");
		(void)mvwprintw(wind, y, x+2, " ");
	} else {      //full
		wattrset(wind, COLOR_PAIR(BLACK_RED));
		(void)mvwprintw(wind, y, x-1, "!");
		(void)mvwprintw(wind, y, x+2, "!");
	}
}

// this prints visible world
void map() {
	tolog("map start\n");
	void invview(), surf(), frontview();
	short i,
	      number=NUMBER_OF_USEFUL;
	(void)wclear(world);
	wstandend(world);
	(void)box(world, 0, 0);
	if (!pl) {
		(void)mvwprintw(world, 22, 21, "^^");
		(void)mvwprintw(world, 20,  0, ">" );
		(void)mvwprintw(world, 20, 43, "<" );
	}
	for (i=0; i<30 && number; ++i) {
		switch (inv[i%10][i/10].what) {
			case CLOCK:
				(void)mvwprintw(world, 22, 38, "%2d:%2d",
					time/60, time%60);
				--number;
			break;
			case COMPASS:
				if (eye[0]==0)
					if (-1==eye[1]*(('k'==view) ? (-1) : 1))
						(void)mvwprintw(world, 0, 1, "^north^");
					else
						(void)mvwprintw(world, 0, 1, "^south^");
				else if (-1==eye[0]*(('k'==view) ? (-1) : 1))
						(void)mvwprintw(world, 0, 1, "^west^");
				else
						(void)mvwprintw(world, 0, 1, "^east^");
				--number;
			break;
			default: break;	
		}
	}
	if (30==i)
		if (time<6*60) (void)mvwprintw(world, 22, 37, "night");
		else if (time<12*60) (void)mvwprintw(world, 22, 36, "morning");
		else if (time<18*60) (void)mvwprintw(world, 22, 40, "day");
		else (void)mvwprintw(world, 22, 36, "evening");
	switch (view) {
		case 'u': (void)mvwprintw(world, 22, 1, "surface"); surf(); break;
		case 'f': (void)mvwprintw(world, 22, 1, "floor"  ); surf(); break;
		case 'h': (void)mvwprintw(world, 22, 1, "head"   ); surf(); break;
		case 'k':
			(void)mvwprintw(world, 22,  1, "sky");
			(void)mvwprintw(world,  0, 21, "vv" );
			(void)mvwprintw(world,  2,  0, ">"  );
			(void)mvwprintw(world,  2, 43, "<"  );
			surf();
		break;
		case 'r':
			(void)mvwprintw(world, 22,  1, "front");
			(void)mvwprintw(world, 22, 21, "^^"   );
			(void)mvwprintw(world, 20,  0, ">"    );
			(void)mvwprintw(world, 20, 43, "<"    );
			frontview();
		break;
		case 'c': //no break
		case 'i': (void)mvwprintw(world, 22, 1, "inventory"); invview(); break;
		default : (void)mvwprintw(world, 22, 1, "another"  ); invview(); break;
	}
	(void)wrefresh(world);
	tolog("map finish\n");
}

//prints sounds
void sounds_print() {
	tolog("sounds_print start\n");
	register short i;
	for (i=0; i<9; ++i) {
		radar[i].what=' ';
		radar[i].num=0;
	}
	sounds(animalstart);
	for (i=0; i<9; ++i) {
		if (radar[i].num<0) radar[i].num='-';
		else if (radar[i].num>9) radar[i].num='+';
		else if (0==radar[i].num) radar[i].num+=' ';
		else radar[i].num+='0';
	}
	if (' '==radar[4].what) radar[4].what='.';
	(void)wclear(sound_window);
	wmove(sound_window, 1, 1);
/* 0 1 2
 * 3 4 5
 * 6 7 8
 * this looks ugly, but should work fast:*/
	if (eye[0]==0)
		if (eye[1]==-1)
			(void)wprintw(sound_window, //north
			"%c%c%c%c%c%c\n %c%c%c%c%c%c\n %c%c%c%c%c%c",
			radar[0].what, radar[0].num, radar[1].what, radar[1].num,
			radar[2].what, radar[2].num,
			radar[3].what, radar[3].num, radar[4].what, radar[4].num,
			radar[5].what, radar[5].num,
			radar[6].what, radar[6].num, radar[7].what, radar[7].num,
			radar[8].what, radar[8].num);
		else
			(void)wprintw(sound_window, //south
			"%c%c%c%c%c%c\n %c%c%c%c%c%c\n %c%c%c%c%c%c",
			radar[8].what, radar[8].num, radar[7].what, radar[7].num,
			radar[6].what, radar[6].num,
			radar[5].what, radar[5].num, radar[4].what, radar[4].num,
			radar[3].what, radar[3].num,
			radar[2].what, radar[2].num, radar[1].what, radar[1].num,
			radar[0].what, radar[0].num);
	else if (eye[0]==-1)
			(void)wprintw(sound_window, //west
			"%c%c%c%c%c%c\n %c%c%c%c%c%c\n %c%c%c%c%c%c",
			radar[6].what, radar[6].num, radar[3].what, radar[3].num,
			radar[0].what, radar[0].num,
			radar[7].what, radar[7].num, radar[4].what, radar[4].num,
			radar[1].what, radar[1].num,
			radar[8].what, radar[8].num, radar[5].what, radar[5].num,
			radar[2].what, radar[2].num);
	else
			(void)wprintw(sound_window, //east
			"%c%c%c%c%c%c\n %c%c%c%c%c%c\n %c%c%c%c%c%c",
			radar[2].what, radar[2].num, radar[5].what, radar[5].num,
			radar[8].what, radar[8].num,
			radar[1].what, radar[1].num, radar[4].what, radar[4].num,
			radar[7].what, radar[7].num,
			radar[0].what, radar[0].num, radar[3].what, radar[3].num,
			radar[6].what, radar[6].num);
	(void)box(sound_window, 0, 0);
	(void)mvwprintw(sound_window, 0, 1, "sounds");
	mvwprintw(sound_window, 4, 2, (NEAR==radar_dist) ? "near" : "far");
	(void)wrefresh(sound_window);
	tolog("sounds_print finish\n");
}

//this prints world in front view
void frontview() {
	tolog("frontview start\n");
	void in_frontview();
	if (eye[0]==0)
		if (eye[1]==-1) in_frontview(xp-11,  1, yp-1, yp-21, -1, 1); //north
		else            in_frontview(xp+11, -1, yp+1, yp+21,  1, 1); //south
	else if (eye[0]==-1)    in_frontview(yp+11, -1, xp-1, xp-21, -1, 0); //west
	else                    in_frontview(yp-11,  1, xp+1, xp+21,  1, 0); //east
	tolog("frontview finish\n");
}

//this function is used inside frontview()
void in_frontview(xsave, xplus, zsave, zend, zplus, flag)
short xsave, xplus,
      zsave, zend,
      zplus, flag; {
	int   visible2x3();
	char  getname();
	register short x, y;
	short save, p, z,
	      xnew, ynew;
	p=(flag) ? yp : xp;
	for (x=1; x<=21; ++x)
	for (y=1; y<=21; ++y) {
		ynew=zp+20-y;
		if (flag) {
			xnew=xsave+x*xplus;
			z=zsave;
			while (z!=zend && earth[xnew][z][ynew]==0) z+=zplus;
			save=z;
//			p=yip;
		} else {
			z=xsave+x*xplus;
			xnew=zsave;
			while (xnew!=zend && earth[xnew][z][ynew]==0) xnew+=zplus;
			save=xnew;
//k			p=xp;
		}
		if (visible2x3(xp, yp, zp+1, xnew, z, ynew) &&
				illuminated(xnew, z, ynew)) {
			if (save!=zend) {
				//TODO: this can be without 'names'
				char name=getname(earth[xnew][z][ynew], world), name2;
				if (abs(save-p)==1)      name2=name;
				else if (abs(save-p)<11) name2=abs(save-p)-1+'0';
				else                     name2='+';
				(void)mvwprintw(world, y, 2*x-1, "%c%c", name, name2);
			} else if (earth[xnew][z][ynew]!=0) {
					wattrset(world, COLOR_PAIR(WHITE_BLACK));
					(void)mvwprintw(world, y, 2*x-1, "??");
			} else {
				wattrset(world, COLOR_PAIR(WHITE_BLUE));
				(void)mvwprintw(world, y, 2*x-1, ". ");
			}
		}
	}
}

//surface view
void surf() {
	tolog("surf start\n");
	void  in_surf();
	short skycor=('k'==view) ? (-1) : 1;
	if (eye[0]==0)
		if (eye[1]==-1) //north
			in_surf(xp-11,        yp-20*skycor,  1,  skycor,  0,       0);
		else            //south
			in_surf(xp+11,        yp+20*skycor, -1, -skycor,  0,       0);
	else if (eye[0]==-1)    //west
			in_surf(xp-20*skycor, yp+11,         0,  0,       skycor, -1);
	else                    //east
			in_surf(xp+20*skycor, yp-11,         0,  0,      -skycor,  1);
	tolog("surf finish\n");
}

//this function is inside surf function
void in_surf(xcor, ycor, xarr, yarr, xrarr, yrarr)
short xcor,  ycor,
      xarr,  yarr,
      xrarr, yrarr; {
	int   visible2(), visible2x3(),
	      property();
	char  getname(), get_sky_name();
	register short x, y, z;
	short st, en, plus,
	      newx, newy;
	switch (view) {
		case  'f': st=zp;     en=0;      plus=-1; break; //floor
		case  'h': st=zp+1;   en=0;      plus=-1; break; //head
		case  'k': st=zp+2;   en=HEAVEN; plus= 1; break; //sky
		default  : st=HEAVEN; en=0;      plus=-1; break; //surface 
	}
	for (y=1; y<=21; ++y)
	for (x=1; x<=21; ++x) {
		newx=xcor+xarr*x+xrarr*y+(('k'==view) ? eye[0]*18 : 0);
		newy=ycor+yarr*y+yrarr*x+(('k'==view) ? eye[1]*18 : 0);
		for (z=st; z!=en && property(earth[newx][newy][z], 't'); z+=plus);
		if (z==HEAVEN) {
			if (visible2(xp, yp, zp, newx, newy, z)) //print sky
				if (sky[newx-xp+19][newy-yp+19].birds)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
					get_sky_name(sky[newx-xp+19][newy-yp+19].birds));
				else if (sky[newx-xp+19][newy-yp+19].clouds)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
					get_sky_name(sky[newx-xp+19][newy-yp+19].clouds));
				else if (sky[newx-xp+19][newy-yp+19].sun)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
					get_sky_name(sky[newx-xp+19][newy-yp+19].sun));
				else (void)mvwprintw(world, y, 2*x-1, "%c ",
					get_sky_name(sky[newx-xp+19][newy-yp+19].sky));
		} else if (visible2x3(xp, yp, zp+1, newx, newy, z) && 
				illuminated(newx, newy, z))
			if (z-zp>=10) (void)mvwprintw(world, y, 2*x-1,
				"%c%c", getname(earth[newx][newy][z], world), '+');
			else if (z-zp>-1) (void)mvwprintw(world, y, 2*x-1,
				"%c%c", getname(earth[newx][newy][z], world), z-zp+1+'0');
			else if (z-zp==-2) (void)mvwprintw(world, y, 2*x-1,
				"%c%c", getname(earth[newx][newy][z], world), '-');
			else if (z-zp<-2) (void)mvwprintw(world, y, 2*x-1,
				"%c%c", getname(earth[newx][newy][z], world), '!');
			else {
				char name=getname(earth[newx][newy][z], world);
				(void)mvwprintw(world, y, 2*x-1, "%c%c", name, name);
			}
	}
	//show player or not
	if (pl && view!='k') {
		for (z=HEAVEN; z!=zp && earth[xp][yp][z]==0; --z);
		if (z==zp) {
			wattrset(world, COLOR_PAIR(WHITE_BLUE));
			(void)mvwprintw(world, 20, 21, "  ");
		}
	}
}

//this prints inventory
void invview() {
	tolog("invview start\n");
	void  mark();
	char  getname();
	short i, j;
	//left arm
	wattrset(world, COLOR_PAIR(BLACK_WHITE));
	(void)mvwprintw(world, 4, 5, "U");
	//right arm
	if (inv[cloth[4].num][2].what) (void)mvwprintw(world, 4, 1, "%c%d",
			getname(inv[cloth[4].num][2].what, world),
			        inv[cloth[4].num][2].num);
	else (void)mvwprintw(world, 4, 2, "U");
	//shoulders
	if (cloth[1].what) (void)getname(cloth[1].what, world);
	else wattrset(world, COLOR_PAIR(WHITE_BLUE));
	(void)mvwprintw(world, 3, 2, "    ");
	for (i=0; i<=3; ++i)
		if (cloth[i].what)
			(void)mvwprintw(world, 2+i, 3, "%c%d",
				getname(cloth[i].what, world), cloth[i].num);
		else switch (i) {
			case 0: //head
				wattrset(world, COLOR_PAIR(BLACK_WHITE));
				(void)mvwprintw(world, 2, 3, "''");
			break;
			case 1:	case 2: //body & legs
				wattrset(world, COLOR_PAIR(WHITE_BLUE));
				(void)mvwprintw(world, 2+i, 3, "  ");
			break;
			case 3: //feet
				wattrset(world, COLOR_PAIR(BLACK_WHITE));
				(void)mvwprintw(world, 5, 3, "db");
			break;
		}
	//backpack
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+18+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(inv[j][i].what, world), inv[j][i].num);
	wstandend(world);
	//chest
	if ('c'==view) {
		(void)mvwprintw(world, 16, 1, "Chest");
		(void)mvwprintw(world, 15, 6, "|");
		(void)mvwprintw(world, 16, 6, "|");
		(void)mvwprintw(world, 17, 6, "|");
		struct something *open_chest(), *chest=open_chest();
		for (j=0; j<=9; ++j)
		for (i=0; i<=2; ++i)
			(void)mvwprintw(world, i+15, j*3+7, "%c%d",
				getname(chest->arr[3+j+i*10], world),
				        chest->arr[33+j+i*10]);
	}
	//workbench
	(void)mvwprintw(world, 10, 9, "Workbench");
	if (view!='w')
		for (i=0; i<=1; ++i)
		for (j=0; j<=1; ++j)
			(void)mvwprintw(world, 11+i, 10+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	else
		for (i=0; i<=2; ++i)
		for (j=0; j<=2; ++j)
			(void)mvwprintw(world, 11+i, 7+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	(void)mvwprintw(world, 12, 17, "%c%d", getname(craft[0].what, world),
	                                               craft[0].num);
	//cursor (i, j are now coordinates)
	switch (cur[0]) {
		case 1: //chest
			i=cur[2]+((cur[2]>2) ? 12 : 18+((cur[2]==2) ? 1 : 0));
			j=cur[1]*3+7;
		break;
		case 2: //player
			i=cur[2]+2;
			j=3;
		break;
		case 3: //workbench
			i=(cur[1]!=3) ? cur[2]+11 : 12;
			j=(cur[1]!=3) ? (cur[1]*3+((view!='w') ? 10 : 7)) : 17;
		break;
		//0 is functional field
	}
	if (cur[3]!=0) {
		(void)mvwprintw(world, i, j, "%c%d",
			getname(cur[3], world), cur[4]);
		mark(j, i, world, 'f');
	} else  mark(j, i, world, 'e');
	(void)wclear(pocketwin);
	(void)wrefresh(pocketwin);
	tolog("invview finish\n");
}

//return char name for IDs, also sets colors
char getname(block, pwin)
short  block;
WINDOW *pwin; {
//	tolog("getname start\n");
	switch (block) {
		case GRASS:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(BLACK_GREEN));
			return '|';
		break;
		case STONE:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 's';
		break;
		case 3: //player
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(WHITE_BLUE));
			return 'p';
		break;
		case CHICKEN:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(RED_WHITE));
			return 'c';
		break;
		case FIRE:
			if (time%2) {
				if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(RED_YELLOW));
				return 'F';
			} else {
				if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(YELLOW_RED));
				return 'f';
			}
		break;
		case STEEL_HELMET:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'h';
		break;
		case CHEST:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(BLACK_YELLOW));
			return 'c';
		break;
		case HEAP:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(WHITE_BLACK));
			return 'h';
		break;
		case CLOCK:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(BLUE_YELLOW));
			return 'c';
		break;
		case COMPASS:
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(RED_BLACK));
			return 'c';
		break;
		case AIR: if (NULL!=pwin) wstandend(pwin); return ' '; break;
		default: return '?'; break;
	}
}

char get_sky_name(id)
short id; {
switch (id) {
	case BLUE_SKY:    wattrset(world, COLOR_PAIR(WHITE_BLUE));   return ' '; break;
	case CYAN_SKY:    wattrset(world, COLOR_PAIR(WHITE_CYAN));   return ' '; break;
	case BLACK_SKY:   wattrset(world, COLOR_PAIR(WHITE_BLACK));  return ' '; break;
	case BLUE_STAR:   wattrset(world, COLOR_PAIR(WHITE_BLUE));   return '.'; break;
	case BLACK_STAR:  wattrset(world, COLOR_PAIR(WHITE_BLACK));  return '.'; break;
	case SUN:         wattrset(world, COLOR_PAIR(RED_YELLOW));   return ' '; break;
	case MOON:        wattrset(world, COLOR_PAIR(BLACK_WHITE));  return ' '; break;
	case CLOUD:       wattrset(world, COLOR_PAIR(BLACK_WHITE));  return 'c'; break;
	case BLUE_RAVEN:  wattrset(world, COLOR_PAIR(BLACK_BLUE));   return 'r'; break;
	case CYAN_RAVEN:  wattrset(world, COLOR_PAIR(BLACK_CYAN));   return 'r'; break;
	case BLACK_RAVEN: wattrset(world, COLOR_PAIR(WHITE_BLACK));  return ' '; break;
	case CLOUD_RAVEN: wattrset(world, COLOR_PAIR(BLACK_WHITE));  return 'r'; break;
	case SUN_RAVEN:   wattrset(world, COLOR_PAIR(BLACK_YELLOW)); return 'r'; break;
	case BLUE_BIRD:   wattrset(world, COLOR_PAIR(RED_BLUE));     return 'b'; break;
	case CYAN_BIRD:   wattrset(world, COLOR_PAIR(RED_CYAN));     return 'b'; break;
	case BLACK_BIRD:  wattrset(world, COLOR_PAIR(RED_BLACK));    return 'b'; break;
	case CLOUD_BIRD:  wattrset(world, COLOR_PAIR(RED_WHITE));    return 'b'; break;
	case SUN_BIRD:    wattrset(world, COLOR_PAIR(RED_YELLOW));   return 'b'; break;
	default:          wattrset(world, COLOR_PAIR(WHITE_BLACK));  return '?'; break;
}}
