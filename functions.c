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

extern struct something *animalstart,
                        *cheststart,
                        *thingstart,
			*heapstart;
extern short xp, yp, zp,
             view,
	     eye[],
	     pl,
             earth[][192][HEAVEN+1];

//all properties for all blocks
int property(id, c)
short id;
char  c; {
//	fprintf(stderr, "property, id is %d\n", id);
	switch (c) {
		case 'a': //Armour
//			fprintf(stderr, "id is %d\n", id);
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
		case 'n': //active things
			switch (id) {
				//animals
				case  4: return 'a'; break;
				//thing
//				case  : return 't'; break;
//				//chest
				case  7: return 'c'; break;
				case  8: return 'h'; break;
				default: return  0 ; break;
			}
		break;
		case 'p': //Passable: air
			if (id==0) return 1;
			else return 0;
		break;
		case 's': //Stackable (armour is already mentioned)
			if (6!=id) return 0;
			else return 1;
		break;
		case 't': //Transparent
			if (id==0) return 1;
			else return 0;
		break;
		default: break;
	}
}

//this finds pointer to a thing with coordinates
struct something *findanimal(x, y, z)
short x, y, z; {
//	fprintf(stderr, "findanimal\n");
	struct something *car;
	switch (property(earth[x][y][z], 'n')) {
		case 'a': car=animalstart; break;
		case 'c': car=cheststart;  break;
		case 'h': car=heapstart;   break;
	}
	while (car!=NULL)
		if (x==car->arr[0] &&
		    y==car->arr[1] &&
		    z==car->arr[2]) return(car);
		else car=car->next;
}

//returns pointer to chest to open
struct something *open_chest() {
	struct something *findanimal();
	void   focus();
	short  x, y, z;
	focus(&x, &y, &z);
        return(findanimal(x, y, z));
}

//makes square file name from square coordinates
void makename(x, y, name)
short x, y;
char name[]; {
//	fprintf(stderr, "makename\n");
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
}

//this shows what block player is focused on
void focus(px, py, pz)
short *px, *py, *pz; {
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
}

//visibility checkers section

//this could be perfect visibility function, but it is not
/*int visible2x3(x1, y1, z1, x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	int visible2();
	if (visible2(x1, y1, z1, x2, y2, z2)) return 1;
	else {
		short count=0;
		count+=visible2(x1, y1, z1, x2-1, y2,   z2  );
		count+=visible2(x1, y1, z1, x2+1, y2,   z2  );
		count+=visible2(x1, y1, z1, x2,   y2-1, z2  );
		count+=visible2(x1, y1, z1, x2,   y2+1, z2  );
		count+=visible2(x1, y1, z1, x2,   y2,   z2-1);
		count+=visible2(x1, y1, z1, x2,   y2,   z2+1);
		if (count>0) return 1;
		else return 0;
	}*/
/*	else if (visible2(x1, y1, z1, x2-1, y2,   z2  )) return 1;
	else if (visible2(x1, y1, z1, x2+1, y2,   z2  )) return 1;
	else if (visible2(x1, y1, z1, x2,   y2-1, z2  )) return 1;
	else if (visible2(x1, y1, z1, x2,   y2+1, z2  )) return 1;
	else if (visible2(x1, y1, z1, x2,   y2,   z2-1)) return 1;
	else if (visible2(x1, y1, z1, x2,   y2,   z2+1)) return 1;
	else return 0;*/
//}

//this is visibility checker. it is not perfect, but it works
//(with the second one)
int visible(x, y, z)
short x, y, z; {
	int   visin();
	short count=0;
	if (z>zp+1) {
		count+=visin(zp+1, xp, yp,   z, x, y, 4);
		count+=visin(zp+1, yp, xp,   z, y, x, 5);
	} else {
		count+=visin(xp,   yp, zp+1, x, y, z, 0);
		count+=visin(yp,   xp, zp+1, y, x, z, 2);
	}
	return((count>1) ? 0 : 1);
}

//this function is inside visible function
int visin(nf, ns, nt, kf, ks, kt, wh)
short nf, ns, nt, kf, ks, kt, wh; {
	int   property();
	short count=0,
	      mff=(nf>kf) ? (-1) : (1),
	      mfs=(ns>ks) ? (-1) : (1),
	      mft=(nt>kt) ? (-1) : (1);
	while (!( nf==kf && ns==ks && nt==kt )) {
		short x, y, z;
		if ((abs(nf-kf)>=abs(ns-ks)) &&
		    (abs(nf-kf)>=abs(nt-kt)))    nf+=mff;
		else if (abs(ns-ks)>=abs(nt-kt)) ns+=mfs;
		else                             nt+=mft;
		switch (wh) {
			case  0: x=nf; y=ns; z=nt; break;
//			case  1: x=nf; z=ns; y=nt; break;
			case  2: y=nf; x=ns; z=nt; break;
//			case  3: y=nf; z=ns; x=nt; break;
			case  4: z=nf; x=ns; y=nt; break;
			//case 5:
			default: z=nf; y=ns; x=nt; break;
		}	
		if (!property(earth[x][y][z], 't') &&
			!( x==xp && y==yp && z<=zp-1 )) ++count;
	}
	return(count-1);
}

//this is the second vibility checker. it works with the first.
int visible2(x1, y1, z1,
             x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	//TODO: optimize loops
	int   property();
	short i, j, k,
	      imin, jmin, kmin,
	      newmin,
	      min=(x1-x2)*(x1-x2)+
	          (y1-y2)*(y1-y2)+
	          (z1-z2)*(z1-z2);
	for (i=x1-1; i<=x1+1; ++i)
	for (j=y1-1; j<=y1+1; ++j)
	for (k=z1-1; k<=z1+1; ++k)
		if ((newmin=(i-x2)*(i-x2)+
		            (j-y2)*(j-y2)+
			    (k-z2)*(k-z2))<min) {
			imin=i;
			jmin=j;
			kmin=k;
			min=newmin;
		}
	if (min==0) return(1);
	else if (!property(earth[imin][jmin][kmin], 't')) return(0);
	else if (visible2(imin, jmin, kmin, x2, y2, z2)) return(1);
}
