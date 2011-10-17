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
#include <ncurses.h>
#include <locale.h>

extern WINDOW      *world, *textwin, *pocketwin, *sound_window;
extern short       xp, yp, zp, pl, eye[], earth[][192][HEAVEN+1],
                   sky[][39], cur[], mechtoggle, radar_dist;
extern struct item inv[][3], cloth[], radar[], *craft;
extern struct something *animalstart;
extern char        view;

//global notify system.
void notify(not, noc)
char not[];
short noc; {
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
}

//this prints pockets contents
void pocketshow() {
	void  mark();
	char  getname();
	short x;
	(void)wclear(pocketwin);
	for (x=0; x<=9; ++x) (void)mvwprintw(pocketwin, 0, 7+x*3, "%c%d",
		getname(inv[x][2].what, pocketwin), inv[x][2].num);
	mark(7+cloth[4].num*3, 0, pocketwin, 'e');
	(void)wrefresh(pocketwin);
}

//this marks chosen item in inventory and pockets
void mark(x, y, wind, c)
short  x, y;
WINDOW *wind;
char   c; {
	wattrset(wind, COLOR_PAIR(8));
	if (c=='e') { //empty
		(void)mvwprintw(wind, y, x-1, ">");
		(void)mvwprintw(wind, y, x+2, "<");
	} else {      //full
		(void)mvwprintw(wind, y, x-1, "!");
		(void)mvwprintw(wind, y, x+2, "!");
	}
}

// this prints visible world
void map() {
	void  invview(), surf(), frontview(),
	      sound();
	(void)wclear(world);
	if (view=='u' || view=='f' || view=='h' || view=='k') surf();
	else if (view=='r') frontview();
	else invview();
	wstandend(world);
	switch (view) {
		case 'u': (void)mvwprintw(world, 22, 1,  "Surface"  ); break;
		case 'f': (void)mvwprintw(world, 22, 1,  "Floor"    ); break;
		case 'h': (void)mvwprintw(world, 22, 1,  "Head"     ); break;
		case 'k': (void)mvwprintw(world, 22, 1,  "Sky                 ^^"); break;
		case 'r': (void)mvwprintw(world, 22, 1,  "Front               ^^"); break;
		case 'c':
		case 'i': (void)mvwprintw(world,  1, 17, "Inventory"); break;
		default : (void)mvwprintw(world, 22, 1,  "Another"  ); break;
	}
	if (!pl) (void)mvwprintw(world, 22, 21, "^^");
	(void)box(world, 0, 0);
	(void)wrefresh(world);
}

//prints sounds
void sounds_print() {
	short i;
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
}

//this prints world in front view
void frontview() {
	void in_frontview();
	if (eye[0]==0)
		if (eye[1]==-1) in_frontview(xp-11,  1, yp-1, yp-21, -1, 1); //north
		else            in_frontview(xp+11, -1, yp+1, yp+21,  1, 1); //south
	else if (eye[0]==-1)    in_frontview(yp+11, -1, xp-1, xp-21, -1, 0); //west
	else                    in_frontview(yp-11,  1, xp+1, xp+21,  1, 0); //east
}

//this function is used inside frontview()
void in_frontview(xsave, xplus, zsave, zend, zplus, flag)
short xsave, xplus,
      zsave, zend,
      zplus, flag; {
	int   visible(),
	      visible2();
	char  getname();
	short x, y;
	for (x=1; x<=21; ++x)
	for (y=1; y<=21; ++y) {
		short save, p, z,
		      xnew, ynew=zp+20-y;
		if (flag) {
			xnew=xsave+x*xplus;
			z=zsave;
			while (z!=zend && earth[xnew][z][ynew]==0) z+=zplus;
			save=z;
			p=yp;
		} else {
			z=xsave+x*xplus;
			xnew=zsave;
			while (xnew!=zend && earth[xnew][z][ynew]==0) xnew+=zplus;
			save=xnew;
			p=xp;
		}
		if (visible2(xp, yp, zp+1, xnew, z, ynew) || visible(xnew, z, ynew)) {
			if (save!=zend) {
				//TODO: this can be without 'names'
				char name=getname(earth[xnew][z][ynew], world), name2;
				if (abs(save-p)==1)      name2=name;
				else if (abs(save-p)<11) name2=abs(save-p)-1+'0';
				else                     name2='+';
				(void)mvwprintw(world, y, 2*x-1, "%c%c", name, name2);
			} else if (earth[xnew][z][ynew]!=0) {
					wattrset(world, COLOR_PAIR(6));
					(void)mvwprintw(world, y, 2*x-1, "??");
			} else {
				wattrset(world, COLOR_PAIR(1));
				(void)mvwprintw(world, y, 2*x-1, ". ");
			}
		}
	}
}

//surface view
void surf() {
	void  in_surf();
	short skycor=(view==3) ? (-1) : 1;
	if (eye[0]==0)
		if (eye[1]==-1) //north
			in_surf(xp-11,        yp-20*skycor,  1,  skycor,  0,       0);
		else            //south
			in_surf(xp+11,        yp+20*skycor, -1, -skycor,  0,       0);
	else if (eye[0]==-1)    //west
			in_surf(xp-20*skycor, yp+11,         0,  0,       skycor, -1);
	else                    //east
			in_surf(xp+20*skycor, yp-11,         0,  0,      -skycor,  1);
}

//this function is inside surf function
void in_surf(xcor, ycor, xarr, yarr, xrarr, yrarr)
short xcor,  ycor,
      xarr,  yarr,
      xrarr, yrarr; {
	int   visible(),
	      visible2(),
	      property();
	char  getname();
	short x, y, z;
	for (y=1; y<=21; ++y)
	for (x=1; x<=21; ++x) {
		short st, en, plus,
		      newx=xcor+xarr*x+xrarr*y,
		      newy=ycor+yarr*y+yrarr*x;
		switch (view) {
			case  'f': st=zp;     en=0;      plus=-1; break; //floor
			case  'h': st=zp+1;   en=0;      plus=-1; break; //head
			case  'k': st=zp+2;   en=HEAVEN; plus= 1; break; //sky
			default  : st=HEAVEN; en=0;      plus=-1; break; //surface 
		}
		for (z=st; z!=en && property(earth[newx][newy][z], 't'); z+=plus);
		if (z==HEAVEN) {
			if (visible2(xp, yp, zp, newx, newy, z)) {
				switch (sky[newx-xp+20][newy-yp+20]) {
					//stars
					case  1: wattrset(world, COLOR_PAIR(3)); break;
					//sun
					case  2: wattrset(world, COLOR_PAIR(4)); break;
					//blue sky
					default: wattrset(world, COLOR_PAIR(1)); break;
				}
				(void)mvwprintw(world, y, 2*x-1, "  ");
			}
		} else if (visible2(xp, yp, zp+1, newx, newy, z) ||
			visible(newx, newy, z))
				if (z-zp>=10) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(earth[newx][newy][z], world),
					'+');
				else if (z-zp>-1) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(earth[newx][newy][z], world),
					z-zp+1+'0');
				else if (z-zp<-1) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(earth[newx][newy][z], world),
					'-');
				else {
					char name=getname(earth[newx][newy][z], world);
					(void)mvwprintw(world, y, 2*x-1,
					"%c%c", name, name);
				}
	}
	//show player or not
	if (pl && view!='k') {
		for (z=HEAVEN; z!=zp && earth[xp][yp][z]==0; --z);
		if (z==zp) {
			wattrset(world, COLOR_PAIR(1));
			(void)mvwprintw(world, 20, 21, "  ");
		}
	}
}

//this prints inventory
void invview() {
	void  mark();
	char  getname();
	short i, j;
	//left arm
	wattrset(world, COLOR_PAIR(3));
	(void)mvwprintw(world, 4, 5, "U");
	//right arm
	if (inv[cloth[4].num][2].what) (void)mvwprintw(world, 4, 1, "%c%d",
			getname(inv[cloth[4].num][2].what, world),
			        inv[cloth[4].num][2].num);
	else (void)mvwprintw(world, 4, 2, "U");
	//shoulders
	if (cloth[1].what) (void)getname(cloth[1].what, world);
	else wattrset(world, COLOR_PAIR(1));
	(void)mvwprintw(world, 3, 2, "    ");
	for (i=0; i<=3; ++i)
		if (cloth[i].what)
			(void)mvwprintw(world, 2+i, 3, "%c%d",
				getname(cloth[i].what, world), cloth[i].num);
		else switch (i) {
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
	//backpack
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+19+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(inv[j][i].what, world), inv[j][i].num);
	wstandend(world);
	//chest
	if ('c'==view) {
		(void)mvwprintw(world, 17, 1, "Chest");
		(void)mvwprintw(world, 16, 6, "|");
		(void)mvwprintw(world, 17, 6, "|");
		(void)mvwprintw(world, 18, 6, "|");
		struct something *open_chest(), *chest=open_chest();
//		struct something *chest=open_chest();
		for (j=0; j<=9; ++j)
		for (i=0; i<=2; ++i)
			(void)mvwprintw(world, i+16, j*3+7, "%c%d",
				getname(chest->arr[3+j+i*10], world),
				        chest->arr[33+j+i*10]);
	}
	//workbench
	(void)mvwprintw(world, 11, 9, "Workbench");
	if (view!='w')
		for (i=0; i<=1; ++i)
		for (j=0; j<=1; ++j)
			(void)mvwprintw(world, 12+i, 10+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	else
		for (i=0; i<=2; ++i)
		for (j=0; j<=2; ++j)
			(void)mvwprintw(world, 12+i, 7+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	(void)mvwprintw(world, 13, 17, "%c%d", getname(craft[0].what, world),
	                                               craft[0].num);
	//cursor (i, j are now coordinates)
	switch (cur[0]) {
		case 1: //chest
			i=cur[2]+((cur[2]>2) ? 13 : 19+((cur[2]==2) ? 1 : 0));
			j=cur[1]*3+7;
		break;
		case 2: //player
			i=cur[2]+2;
			j=3;
		break;
		case 3: //workbench
			i=(cur[1]!=3) ? cur[2]+12 : 13;
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
}

//return char name for IDs, also sets colors
char getname(block, pwin)
short  block;
WINDOW *pwin; {
	switch (block) {
		case 1: //grass
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(2));
			return '|';
		break;
		case 2: //stone
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(3));
			return 's';
		break;
		case 3: //player
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(2));
			return 'p';
		break;
		case 4: //chiken
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(5));
			return 'c';
		break;
		case 5: //fire
			if (mechtoggle) {
				if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(4));
				return 'F';
			} else {
				if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(7));
				return 'f';
			}
		break;
		case 6: //steel helmet
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(3));
			return 'h';
		break;
		case 7: //chest
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(9));
			return 'c';
		break;
		case 8: //heap
			if (NULL!=pwin) wattrset(pwin, COLOR_PAIR(6));
			return 'h';
		break;
		case 0:  if (NULL!=pwin) wstandend(pwin); return ' '; break; //air
		default: return '?'; break;
	}
}
