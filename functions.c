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
#include <stdlib.h>
#include <stdio.h>

extern struct something *animalstart, *cheststart, *thingstart, *heapstart;
extern short    xp, yp, zp,
                view, eye[], pl,
                earth[][192][HEAVEN+1];
extern unsigned time;
void tolog();

//all properties for all blocks
int property(id, c)
short id;
char  c; {
//	tolog("property start\n"); //no finish
	switch (c) {
		case 'a': //Armour
			switch (id) {
				//helmets
				case  6: return 'h'; break;
				//body armour
				//return 'a'; break;
				//legs armour
				//return 'l'; ibreak;
				//boots
				//return 'b'; break;
				default: return  0 ; break;
			}
		break;
		case 'c': //chest-like
			if (7==id || 8==id) return 1;
			else return 0;
		case 'd': //dangerous
			switch (id) {
				case  5: return 1; //fire
				default: return 0;
			}
		break;
		case 'l': //gives Light
			if (5==id) return 5;
			else return 0;
		break;
		case 'm': //Movable
			if(4==id || 8==id) return 1;
			else return 0;
		break;
		case 'n': //active things
			switch (id) {
				//animals
				case  4: return 'a'; break;
				//thing
//				case  : return 't'; break;
//				//chest
				case  7: return 'c'; break;
				case  8: return 'h'; break;
				//light
				case  5: return 'l'; break;
				default: return  0 ; break;
			}
		break;
		case 'o': //sOunds
			if (4==id) {
				char getname();
				return getname(id, NULL);
			} else return 0;
		break;
		case 'p': //Passable: air
			if (id==0) return 1;
			else return 0;
		break;
		case 's': //Stackable (armour is already mentioned)
			if (6==id) return 0;
			else return 1;
		break;
		case 't': //Transparent
			if (0==id) return 1;
			else return 0;
		break;
		default: break;
	}
}

int light_radius(x, y, z)
short x, y, z; {
	switch(earth[x][y][z]) {
		case  5: return 5; break; //fire 
		default: return 0; break;
	}
}

//this finds pointer to a thing with coordinates
struct something *findanimal(x, y, z)
short x, y, z; {
	tolog("findanimal start\n");
	struct something *car;
	if (property(earth[x][y][z], 'n')) {
		switch (property(earth[x][y][z], 'n')) {
			case 'a': car=animalstart; break;
			case 'c': car=cheststart;  break;
			case 'h': car=heapstart;   break;
			default : return NULL;     break;
		}
		if (NULL==car) fprintf(stderr, "find null\n");
		while (car!=NULL)
			if (x==car->arr[0] &&
			    y==car->arr[1] &&
			    z==car->arr[2]) return car;
			else car=car->next;
	} else return NULL;
}

//returns pointer to chest to open
struct something *open_chest() {
	tolog("open_chest start\n");
	struct something *findanimal();
	void   focus();
	short  x, y, z;
	focus(&x, &y, &z);
	tolog("open_chest finish\n");
        return(findanimal(x, y, z));
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
	if (pl || view==4) {
		if (eye[0]==0) {
			*px=xp;
			*py=(eye[1]==-1) ? yp-1 : yp+1;
		} else {
			*px=(eye[0]==-1) ? xp-1 : xp+1;
			*py=yp;
		}
		switch (view) {
			case 1:  *pz=zp-1; break; //floor
			case 2:  *pz=zp+1; break; //head
			case 3:  *pz=zp+2; break; //sky
			default: *pz=zp;   break; //surface and front
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
int visible2x3(x1, y1, z1, x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	int visible2(), property();
	short savecor;
	if (visible2(x1, y1, z1, x2, y2, z2)) return 1;
	else if ((x2!=x1 && property(earth[x2+(savecor=(x2>x1) ? (-1) : 1)][y2][z2], 't')
		&& visible2(x1, y1, z1, x2+savecor, y2, z2)) ||
		(y2!=y1 && property(earth[x2][y2+(savecor=(y2>y1) ? (-1) : 1)][z2], 't')
		&& visible2(x1, y1, z1, x2, y2+savecor, z2)) ||
		(z2!=z1 && property(earth[x2][y2][z2+(savecor=(z2>z1) ? (-1) : 1)], 't')
		&& visible2(x1, y1, z1, x2, y2, z2+savecor))) return 1;
	else return 0;
}

int visible2(x1, y1, z1,
             x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	int visible2_0(), visible2_1();
	fprintf(stderr, "%d-%d-%d %d:%d\n", x2, y2, z2, visible2_0(x1, y1, z1, x2, y2, z2,
		visible2_1(x1, y1, z1, x2, y2, z2)));
	return visible2_1(x1, y1, z1, x2, y2, z2);
}

//this is the vibility checker. it is ~perfect
int visible2_0(x1, y1, z1,
             x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
//	tolog("visible2 start\n");
	//TODO: optimize loops
	int property();
	register short i, j, k,
	               imin, jmin, kmin,
		       zmin, zmax;
	register int   newmin,
	               min=19*19+10*10+HEAVEN*HEAVEN;
	zmin=(z1>1) ? z1 : 0;
	zmax=(z1<HEAVEN-1) ? z1 : HEAVEN;
	for (i=x1-1; i<=x1+1; ++i)
	for (j=y1-1; j<=y1+1; ++j)
	for (k=zmin-1; k<=zmax+1; ++k)
		if ((newmin=(i-x2)*(i-x2)+
		            (j-y2)*(j-y2)+
			    (k-z2)*(k-z2))<min) {
			imin=i;
			jmin=j;
			kmin=k;
			min=newmin;
		}
//	tolog("visible2 ~finish\n");	
	if (min==0) return 1;
	else if (!property(earth[imin][jmin][kmin], 't')) return 0;
	else return visible2_0(imin, jmin, kmin, x2, y2, z2);
}

int visible2_1(x1, y1, z1,
               x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	int property();
	register struct {
		unsigned x : 2;
		unsigned y : 2;
		unsigned z : 2;
	} shift, minshift;
	register int newmin, min;

	//MOAR optimization needed!
	if (x2<x1) shift.x=0;
	else if (x2>x1) shift.x=2;
	else shift.x=1;

	if (y2<y1) shift.y=0;
	else if (y2>y1) shift.y=2;
	else shift.y=1;

	if (z2<z1) shift.z=0;
	else if (z2>z1) shift.z=2;
	else shift.z=1;

	//corners
	if ((shift.x==shift.y && (0==shift.x || 2==shift.y)) || 2==abs(shift.x-shift.y)) {
		minshift.x=shift.x;
		minshift.y=shift.y;
		minshift.z=shift.z;
		min=(x1+shift.x-1-x2)*(x1+shift.x-1-x2)+
		    (y1+shift.y-1-y2)*(y1+shift.y-1-y2)+
		    (z1+shift.z-1-z2)*(z1+shift.z-1-z2);
		if (min>(newmin=((x1-x2)*(x1-x2))+
		    (y1+shift.y-1-y2)*(y1+shift.y-1-y2)+
		    (z1+shift.z-1-z2)*(z1+shift.z-1-z2))) {
			minshift.x=0;
			minshift.y=shift.y;
			minshift.z=shift.z;
			min=newmin;
		}
		if (min>(newmin=(x1+shift.x-1-x2)*(x1+shift.x-1-x2)+
		    (y1-y2)*(y1-y2)+
		    (z1+shift.z-1-z2)*(z1+shift.z-1-z2))) {
			minshift.x=shift.x;
			minshift.y=0;
			minshift.z=shift.z;
			min=newmin;
		}
		if (min>(x1+shift.x-1-x2)*(x1+shift.x-1-x2)+
		    (y1+shift.y-1-y2)*(y1+shift.y-1-y2)+
		    (z1-z2)*(z1-z2)) {
			minshift.x=shift.x;
			minshift.y=shift.y;
			minshift.z=0;
		}
	} else if (1==abs(shift.x-shift.y)) { //edges
		minshift.x=1;
		minshift.y=shift.y;
		minshift.z=shift.z;
		min=(x1-x2)*(x1-x2)+
		    (y1+shift.y-1-y2)*(y1+shift.y-1-y2)+
		    (z1+shift.z-1-z2)*(z1+shift.z-1-z2);
		if (min>(newmin=(x1+shift.x-1-x2)*(x1+shift.x-1-x2)+
		    (y1-y2)*(y1-y2)+
		    (z1+shift.z-1-z2)*(z1+shift.z-1-z2))) {
			minshift.x=shift.x;
			minshift.y=1;
			minshift.z=shift.z;
			min=newmin;
		}
		if (min>(x1+shift.x-1-x2)*(x1+shift.x-1-x2)+
		    (y1+shift.y-1-y2)*(y1+shift.y-1-y2)+
		    (z1-z2)*(z1-z2)) {
			minshift.x=shift.x;
			minshift.y=shift.y;
			minshift.z=1;
		}
	} else { //centers
		minshift.x=shift.x;
		minshift.y=shift.y;
		minshift.z=shift.z;
	}
	if (x1+minshift.x-1==x2 && y1+minshift.y-1==y2 && z1+minshift.z-1==z2) return 1;
	else if (!property(earth[x1+minshift.x-1][y1+minshift.y-1][z1+minshift.z-1], 't'))
		return 0;
	else return visible2_1(x1+minshift.x-1, y1+minshift.y-1, z1+minshift.z-1,
		x2, y2, z2);
}

//returns 1 if block is illuminated so visible
int illuminated(x, y, z)
short x, y, z; {
	if (property(earth[x][y][z], 'l')) return 1;
	else if (abs(xp-x)<2 && abs(yp-y)<2 && abs(zp-z)<3) return 1;
	else if (time>=6*60) {
		register short i;
		int   property(),
		      nigth_illuminated();
		for (i=z; i<HEAVEN && property(earth[x][y][i], 't'); ++i);
		if (HEAVEN>=i) return 1;
		else return night_illuminated(x, y, z);
	} else return night_illuminated(x, y, z);
}

int night_illuminated(x, y, z)
short x, y, z; {
	int property();
	register short i, j, k,
	               kmin=(z>5) ? z-5 : 0,
	               kmax=(z<HEAVEN-5) ? z+5 : HEAVEN,
	               flag=0;
	for (i=x-5; i<=x+5 && !flag; ++i)
	for (j=y-5; j<=y+5 && !flag; ++j)
	for (k=kmin; k<=kmax && !flag; ++k)
		if (property(earth[i][j][k], 'l') && visible2x3(i, j, k, x, y, z))
			flag=1;
	return flag;
}
