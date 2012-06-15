/*Copyright (C) 2011 Alexander Kromm
 *
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
 *along with Eyecube.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "header.h"
#include <stdlib.h>
#include <stdio.h>

extern struct something *animalstart, *cheststart, *thingstart, *heapstart, *lightstart,
                        *workbenchstart, *signstart;
extern short    xp, yp, zp,
                view, eye[], pl,
                earth[][3*WIDTH][HEAVEN+1];
extern unsigned time;
void tolog();

//TODO: make little functions from this:
//all properties for all blocks

//properties section

int arraylength(id)
short id; {
	switch (id) {
		case CHICKEN: case FIRE: return 4; break;
		case PILE:  return 64; break;
		case CHEST: return 63; break;
		default:    return  0; break;
	}
}

int armour(id)
short id; {
	switch (id) {
		//helmets
		case IRON_HELMET: return 'h'; break;
		//body armour
		case IRON_CHESTPLATE: return 'a'; break;
		//legs armour
		case IRON_GREAVES: return 'l'; break;
		//boots
		case IRON_BOOTS: return 'b'; break;
		default: return 0; break;
	}
}

int chestlike(id)
short id; {
	if (CHEST==id || PILE==id || WORKBENCH==id) return 1;
	else return 0;
}

int dangerous(id)
short id; {
	switch (id) {
		case FIRE: return 1;
		default: return 0;
	}
}

int light(id)
short id; {
	if (FIRE==id) return 5;
	else return 0;
}

int movable(id)
short id; {
	if(CHICKEN==id || PILE==id) return 1;
	else return 0;
}

int passable(id)
short id; {
	if (AIR==id) return 1;
	else return 0;
}

int stackable(id)
short id; {
	if (IRON_HELMET==id || CLOCK==id || COMPASS==id || IRON_BOOTS==id ||
			IRON_GREAVES==id || IRON_CHESTPLATE==id) return 0;
	else return 1;
}

int transparent(id)
short id; {
	if (AIR==id) return 1;
	else return 0;
}

int chest_size(id)
short id; {
	switch (id) {
		case CHEST: case PILE: return 30; break;
		case WORKBENCH: return 10; break;
		default: return 0; break;
	}
}

int minus(id)
short id; {
	if (WORKBENCH==id) return 1;
	else return 0;
}

int light_radius(x, y, z)
short x, y, z; {
	switch(earth[x][y][z]) {
		case FIRE: return 5; break;
		default: return 0; break;
	}
}

//end of properties section

//this finds pointer to a thing with coordinates
struct something *findanimal(x, y, z)
short x, y, z; {
	tolog("findanimal start\n");
	struct something *car;
	switch (earth[x][y][z]) {
		case ANIMALS:   car=animalstart;    break;
		case CHEST:     car=cheststart;     break;
		case PILE:      car=heapstart;      break;
		case LIGHTS:    car=lightstart;     break;
		case WORKBENCH: car=workbenchstart; break;
		case SIGN:      car=signstart;      break;
//		case 't': car=thingstart;  break;
		default: return NULL; break;
	}
	while (car!=NULL)
		if (x==car->arr[0] &&
		    y==car->arr[1] &&
		    z==car->arr[2]) return car;
		else car=car->next;
	return NULL;
}

//returns pointer to chest to open
short *open_chest() {
	tolog("open_chest start\n");
	struct something *findanimal();
	void   focus();
	short  x, y, z;
	focus(&x, &y, &z);
	tolog("open_chest finish\n");
        return(findanimal(x, y, z)->arr);
}

//makes square file name from square coordinates
void makename(x, y, name)
short x, y;
char name[]; {
	tolog("makename start\n");
	short i=0;
	name[i++]='m';
	name[i++]='a';
	name[i++]='p';
	name[i++]='/';
	if (x<0) {
		x=abs(x);
		name[i++]='-';
	}
	do {
		name[i++]='0'+x % 10;
		x/=10;
	} while (x!=0);
	name[i++]='+';
	if (y<0) {
		y=abs(y);
		name[i++]='-';
	}
	do {
		name[i++]='0'+y % 10;
		y/=10;
	} while (y!=0);
	name[i]='\0';
	tolog("makename finish\n");
}

//this shows what block player is focused on
void focus(px, py, pz)
short *px, *py, *pz; {
	tolog("focus start\n");
	if (pl || VIEW_FRONT==view) {
		if (eye[0]==0) {
			*px=xp;
			*py=(eye[1]==-1) ? yp-1 : yp+1;
		} else {
			*px=(eye[0]==-1) ? xp-1 : xp+1;
			*py=yp;
		}
		switch (view) {
			case VIEW_FLOOR: *pz=zp-1; break;
			case VIEW_HEAD:  *pz=zp+1; break;
			case VIEW_SKY:   *pz=zp+2; break;
			default:         *pz=zp;   break;
		}
	} else {
		*px=xp;
		*py=yp;
		*pz=zp-1;
	}
	tolog("focus finish\n");
}

//visibility checkers section

//this could be perfect visibility function, but it is not
int visible2x3(x1, y1, z1,
               x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
//	tolog("visible2x3 started\n");
	int visible3(), transparent();
	short savecor;
	if (visible3(x1, y1, z1, x2, y2, z2)) return 1;
	else if ((x2!=x1 && transparent(earth[x2+(savecor=(x2>x1) ? (-1) : 1)][y2][z2])
		&& visible3(x1, y1, z1, x2+savecor, y2, z2)) ||
		(y2!=y1 && transparent(earth[x2][y2+(savecor=(y2>y1) ? (-1) : 1)][z2])
		&& visible3(x1, y1, z1, x2, y2+savecor, z2)) ||
		(z2!=z1 && transparent(earth[x2][y2][z2+(savecor=(z2>z1) ? (-1) : 1)])
		&& visible3(x1, y1, z1, x2, y2, z2+savecor)))
			return 1;
	else return 0;
}

//there isn't the only way for light from one point to another
//this finds one of them, and it is not the straight line.
//it works fast
int visible3(x, y, z, xtarget, ytarget, ztarget)
short x, y, z,
      xtarget, ytarget, ztarget; {
	int transparent();
	if (x==xtarget && y==ytarget && z==ztarget) return 1;
	else if (!transparent(earth[x][y][z])) return 0;
	else {
		if (x!=xtarget) x+=(xtarget>x) ? 1 : -1;
		if (y!=ytarget) y+=(ytarget>y) ? 1 : -1;
		if (z!=ztarget) z+=(ztarget>z) ? 1 : -1;
		return visible3(x, y, z, xtarget, ytarget, ztarget);
	}
}

//returns 1 if block is illuminated so visible
int illuminated(x, y, z)
short x, y, z; {
	int light();
	if (light(earth[x][y][z])) return 1;
	else if (abs(xp-x)<2 && abs(yp-y)<2 && abs(zp-z)<3) return 1;
	else {
		int night_illuminated();
		if (time>=6*60) {
			register short i;
			int transparent();
			for (i=z; i<HEAVEN && transparent(earth[x][y][i]); ++i);
			if (HEAVEN>=i) return 1;
			else return night_illuminated(x, y, z);
		} else return night_illuminated(x, y, z);
	}
}

int night_illuminated(x, y, z)
short x, y, z; {
	int light();
	register short i, j, k,
	               kmin=(z>5) ? z-5 : 0,
	               kmax=(z<HEAVEN-5) ? z+5 : HEAVEN,
	               flag=0;
	short *xsave, *ysave, *zsave;
	for (i=x-5; i<=x+5 && !flag; ++i)
	for (j=y-5; j<=y+5 && !flag; ++j)
	for (k=kmin; k<=kmax && !flag; ++k)
		if (light(earth[i][j][k]) && visible2x3(i, j, k, x, y, z))
			flag=1;
	return flag;
}
