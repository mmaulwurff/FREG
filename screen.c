/*Eyecube, sandbox game.
* Copyright (C) 2011 Alexander Kromm, see README file for details.
*/

#include "header.h"
#include <ncurses.h>
#include <locale.h>

extern WINDOW      *world, *textwin, *pocketwin, *sound_window;
extern short       xp, yp, zp, pl, eye[], earth[][3*WIDTH][HEAVEN+1],
                   cur[], radar_dist, view, *opened;
extern unsigned    time;
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
	if (VIEW_SURFACE==view || VIEW_FLOOR==view || VIEW_HEAD==view || VIEW_SKY==view ||
			VIEW_FRONT==view) {
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
						if (-1==eye[1]*((VIEW_SKY==view) ?
								(-1) : 1))
							(void)mvwprintw(world, 0, 1,
									"^north^");
						else
							(void)mvwprintw(world, 0, 1,
									"^south^");
					else if (-1==eye[0]*((VIEW_SKY==view) ?
								(-1) : 1))
							(void)mvwprintw(world, 0, 1,
									"^west^");
					else
							(void)mvwprintw(world, 0, 1,
									"^east^");
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
	}
	switch (view) {
		case VIEW_SURFACE:
			(void)mvwprintw(world, 22, 1, "surface");
			surf();
		break;
		case VIEW_FLOOR: (void)mvwprintw(world, 22, 1, "floor"  ); surf(); break;
		case VIEW_HEAD:  (void)mvwprintw(world, 22, 1, "head"   ); surf(); break;
		case VIEW_SKY:
			(void)mvwprintw(world, 22,  1, "sky");
			(void)mvwprintw(world,  0, 21, "vv" );
			(void)mvwprintw(world,  2,  0, ">"  );
			(void)mvwprintw(world,  2, 43, "<"  );
			surf();
		break;
		case VIEW_FRONT:
			(void)mvwprintw(world, 22,  1, "front");
			(void)mvwprintw(world, 22, 21, "^^"   );
			(void)mvwprintw(world, 20,  0, ">"    );
			(void)mvwprintw(world, 20, 43, "<"    );
			frontview();
		break;
		case VIEW_CHEST: case VIEW_INVENTORY:
			(void)mvwprintw(world, 22, 1, "inventory");
			invview();
		break;
		case VIEW_WORKBENCH:
			(void)mvwprintw(world, 22, 1, "workbench");
			invview();
		break;
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
				if (abs(save-p)==1)      name2=' ';
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
	short skycor=(VIEW_SKY==view) ? (-1) : 1;
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
	int   visible3(), visible2x3(),
	      transparent();
	char  getname(), get_sky_name();
	register short x, y, z;
	short st, en, plus,
	      newx, newy,
	      sky_x, sky_y;
	switch (view) {
		case VIEW_FLOOR: st=zp;     en=0;      plus=-1; break;
		case VIEW_HEAD:  st=zp+1;   en=0;      plus=-1; break;
		case VIEW_SKY:   st=zp+2;   en=HEAVEN; plus= 1; break;
		default:         st=HEAVEN; en=0;      plus=-1; break;
	}
	for (y=1; y<=21; ++y)
	for (x=1; x<=21; ++x) {
		newx=xcor+xarr*x+xrarr*y+((VIEW_SKY==view) ? eye[0]*18 : 0);
		newy=ycor+yarr*y+yrarr*x+((VIEW_SKY==view) ? eye[1]*18 : 0);
		for (z=st; z!=en && transparent(earth[newx][newy][z]); z+=plus);
		if (z==HEAVEN) {
			sky_x=newx-xp+19;
			sky_y=newy-yp+19;
			if (visible3(xp, yp, zp, newx, newy, z)) //print sky
				if (sky[sky_x][sky_y].birds)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
						get_sky_name(sky[sky_x][sky_y].birds));
				else if (sky[sky_x][sky_y].clouds)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
						get_sky_name(sky[sky_x][sky_y].clouds));
				else if (sky[sky_x][sky_y].sun)
					(void)mvwprintw(world, y, 2*x-1, "%c ",
						get_sky_name(sky[sky_x][sky_y].sun));
				else (void)mvwprintw(world, y, 2*x-1, "%c ",
						get_sky_name(sky[sky_x][sky_y].sky));
		} else if (visible2x3(xp, yp, zp+1, newx, newy, z) && 
				illuminated(newx, newy, z))
			if (z-zp>=10) (void)mvwprintw(world, y, 2*x-1,
				"%c+", getname(earth[newx][newy][z], world));
			else if (z-zp>-1) (void)mvwprintw(world, y, 2*x-1,
				"%c%d", getname(earth[newx][newy][z], world), z-zp+1);
			else if (z-zp==-2) (void)mvwprintw(world, y, 2*x-1,
				"%c-", getname(earth[newx][newy][z], world));
			else if (z-zp<-2) (void)mvwprintw(world, y, 2*x-1,
				"%c!", getname(earth[newx][newy][z], world));
			else (void)mvwprintw(world, y, 2*x-1,
				"%c ", getname(earth[newx][newy][z], world));
	}
	//show player or not
	if (pl && VIEW_SKY!=view) {
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
	char  getname(), *real_name();
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
	wstandend(world);
	(void)mvwprintw(world, 2, 8, "%s on head", real_name(cloth[0].what));
	(void)mvwprintw(world, 3, 8, "%s on body", real_name(cloth[1].what));
	(void)mvwprintw(world, 4, 8, "%s on legs", real_name(cloth[2].what));
	(void)mvwprintw(world, 5, 8, "%s on feet", real_name(cloth[3].what));
	//backpack
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+18+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(inv[j][i].what, world), inv[j][i].num);
	wstandend(world);
	if (VIEW_CHEST==view) {
		(void)mvwprintw(world, 15, 4, "+");
		(void)mvwprintw(world, 16, 4, "+");
		(void)mvwprintw(world, 17, 4, "+");
		for (j=0; j<=9; ++j)
		for (i=0; i<=2; ++i)
			(void)mvwprintw(world, i+15, j*3+7, "%c%d",
				getname(opened[3+j+i*10], world), opened[33+j+i*10]);
	}
	wstandend(world);
	(void)mvwprintw(world, 10, 9, "Crafting");
	if (VIEW_WORKBENCH!=view) {
		(void)mvwprintw(world, 11, 10, "%c%d",
			getname(craft[1].what, world), craft[1].num);
		(void)mvwprintw(world, 12, 10, "%c%d",
			getname(craft[2].what, world), craft[2].num);
		(void)mvwprintw(world, 12, 17, "%c%d",
			getname(craft[0].what, world), craft[0].num);
	} else {
		for (i=0; i<=2; ++i)
		for (j=0; j<=2; ++j)
			(void)mvwprintw(world, 11+i, 7+j*3, "%c%d",
				getname(opened[i+3*j+3], world), opened[i+3*j+3+10]);
		(void)mvwprintw(world, 12, 17, "%c%d",
			getname(opened[12], world), opened[22]);
	}
	//cursor (i, j are now coordinates)
	switch (cur[0]) {
//		case 0: furnace
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
			j=(cur[1]!=3) ? (cur[1]*3+((VIEW_WORKBENCH!=view) ? 10 : 7)) : 17;
		break;
		//0 is functional field
	}
	if (cur[3]!=0) {
		(void)mvwprintw(world, i, j, "%c%d",
			getname(cur[3], world), cur[4]);
		mark(j, i, world, 'f');
	} else  mark(j, i, world, 'e');
	{
		void know_marked();
		short *markedwhat, *markednum;
		char string[30]="";
		know_marked(&markedwhat, &markednum);
		if (*markedwhat) notify(real_name(*markedwhat));
		else notify("");
	}
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
		case GRASS:   wattrset(pwin, COLOR_PAIR(BLACK_GREEN));  return '|'; break;
		case STONE:   wattrset(pwin, COLOR_PAIR(BLACK_WHITE));  return 's'; break;
		case CHEST:   wattrset(pwin, COLOR_PAIR(BLACK_YELLOW)); return 'c'; break;
		case PILE:    wattrset(pwin, COLOR_PAIR(WHITE_BLACK));  return 'h'; break;
		case CLOCK:   wattrset(pwin, COLOR_PAIR(BLUE_YELLOW));  return 'c'; break;
		case COMPASS: wattrset(pwin, COLOR_PAIR(RED_BLACK));    return 'c'; break;
		case CHICKEN: wattrset(pwin, COLOR_PAIR(RED_WHITE));    return 'c'; break;
//		case 3:       wattrset(pwin, COLOR_PAIR(WHITE_BLUE));   return 'p'; break;
		case AIR:     wstandend(pwin);                          return ' '; break;
		case FIRE:
			if (time%2) {
				wattrset(pwin, COLOR_PAIR(RED_YELLOW));
				return 'F';
			} else {
				wattrset(pwin, COLOR_PAIR(YELLOW_RED));
				return 'f';
			}
		break;
		case IRON_INGOT:
			wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'i';
		break;
		case IRON_HELMET:
			wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'h';
		break;
		case IRON_CHESTPLATE:
			wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'p';
		break;
		case IRON_BOOTS:
			wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'b';
		break;
		case IRON_GREAVES:
			wattrset(pwin, COLOR_PAIR(BLACK_WHITE));
			return 'g';
		break;
		case WORKBENCH:
			wattrset(pwin, COLOR_PAIR(BLACK_YELLOW));
			return 'w';
		break;
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

char *real_name(id) 
short id; {
switch (id) {
	case AIR:             return "Nothing";            break;
	case IRON_INGOT:      return "Iron ingot";         break;
	case STONE:           return "Stone";              break;
	case CHEST:           return "Chest";              break;
	case WORKBENCH:       return "Workbench";          break;
	case GRASS:           return "Grass";              break;
	case CHICKEN:         return "Chicken";            break;
	case FIRE:            return "Fire";               break;
	case IRON_HELMET:     return "Iron helmet";        break;
	case PILE:            return "Pile of something";  break;
	case CLOCK:           return "Clock";              break;
	case COMPASS:         return "Compass";            break;
	case IRON_GREAVES:    return "Iron greaves";       break;
	case IRON_BOOTS:      return "Iron boots";         break;
	case IRON_CHESTPLATE: return "Iron chestplate";    break;
	default:              return "Some unknown thing"; break;
}
tolog("realname finish\n");
}
