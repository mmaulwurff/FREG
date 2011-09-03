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

char signal;

struct something {
	struct something *next;
	short  *arr;
} *animalstart=NULL,
  *thingstart =NULL,
  *cheststart =NULL;

extern short earth[][192][HEAVEN+1];
extern short view;
extern short notflag;
extern short xp, yp, zp;

void map(),
     chiken_move(),
     erasein();

short mechtoggle=0;

void allmech() {
//	fprintf(stderr, "allmech, %c\n", signal);
	if (signal=='w') {
		struct something *animalcar=animalstart;
		while (animalcar!=NULL) {
			chiken_move(animalcar);
			animalcar=animalcar->next;
		}
	} 
	{
		short height=0;
		for ( ; property(earth[xp][yp][zp-1], 'p'); ++height, --zp);
		if (height>1) { //gravity
			notify("You fell down.", 0);
			notflag=4;
			//damage
		}
	}
	mechtoggle=(mechtoggle) ? 0 : 1;
	if (view!='i' && view!='w') map();
}

//this moves an animal
void chiken_move(animalp)
struct something *animalp; {
//	fprintf(stderr, "chiken_move\n");
	FILE* file=fopen("/dev/urandom", "rb");
	short c=(unsigned)fgetc(file)%5,
	      save=earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]];
	fclose(file);
	earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=0;
	if      (c==0 && animalp->arr[0]!=191) ++(animalp->arr[0]);
	else if (c==1 && animalp->arr[0]!=0  ) --(animalp->arr[0]);
	else if (c==2 && animalp->arr[1]!=191) ++(animalp->arr[1]);
	else if (c==3 && animalp->arr[1]!=0  ) --(animalp->arr[1]);
	earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=save;
}

//this finds pointer to an animal with coordinates
struct something *findanimal(x, y, z)
short x, y, z; {
//	fprintf(stderr, "findanimal\n");
	struct something *animalcar=animalstart;
	while (animalcar!=NULL)
		if (x==animalcar->arr[0] &&
		    y==animalcar->arr[1] &&
		    z==animalcar->arr[2]) return(animalcar);
		else animalcar=animalcar->next;
}

//this adds new active thing to list of loaded animals
void spawn(x, y, z, file)//, type)
short x, y, z;
FILE  *file; {
//char type; {
	//fprintf(stderr, "spawn\n");
	char type=property(earth[x][y][z], 'n');
	if (type) {
		struct something **animalcar,
				 **start;
		switch (type) {
			case 'a': start=&animalstart; break; //animal
			case 'c': start=&cheststart;  break; //chest
			case 't': start=&thingstart;  break; //thing
		}
//		fprintf(stderr, "spawn2\n");
		if (*start==NULL) {
			 animalcar=malloc(sizeof(struct something *));
			*animalcar=malloc(sizeof(struct something));
			*start=*animalcar;
		} else {
			animalcar=malloc(sizeof(struct something *));
			for (*animalcar=*start; (*animalcar)->next!=NULL;
				*animalcar=(*animalcar)->next);
			*animalcar=(*animalcar)->next=malloc(sizeof(struct something));
		}
//		fprintf(stderr, "spawn3\n");
		switch (type) {
			case 'a': //animal
				(*animalcar)->arr=malloc( 4*sizeof(short));
				(*animalcar)->arr[3]=(file==NULL) ? 9 : getc(file);
			break;
			case 'c': { //chest
				(*animalcar)->arr=malloc(33*sizeof(short));
				short i=3;
				if (file==NULL)
					while (i<33) (*animalcar)->arr[i++]=0;
				else
					while (i<33) (*animalcar)->arr[i++]=getc(file);
			} break;
			case 't': //thing
				(*animalcar)->arr=malloc( 3*sizeof(short));
			break;
		}
		(*animalcar)->arr[0]=x;
		(*animalcar)->arr[1]=y;
		(*animalcar)->arr[2]=z;
		(*animalcar)->next=NULL;
		free(animalcar);
	}
}

//this erases list of loaded animals
void eraseanimals() {
//	fprintf(stderr, "eraseanimals\n");
	signal='s';
	erasein(animalstart);
	animalstart=NULL;
	erasein(thingstart );
	thingstart =NULL;
	erasein(cheststart );
	cheststart =NULL;
	signal='w';
}

void erasein(car)
struct something *car; {
	while (car!=NULL) {
		struct something *animalerase=car;
		car=car->next;
		free(animalerase->arr);
		free(animalerase);
	}
}
