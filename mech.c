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
struct animal {
	short x, y, z,
	      life;
	struct animal *next;
} *animalstart=NULL;
struct thing {
	short x, y, z;
	struct thing *next;
} *thingstart=NULL;

extern short earth[][192][HEAVEN+1];
extern short view;

void map(),
     chiken_move();

short mechtoggle=0;

void allmech() {
//	fprintf(stderr, "allmech, %c\n", signal);
	if (signal=='w') {
		struct animal *animalcar=animalstart;
		while (animalcar!=NULL) {
			chiken_move(animalcar);
			animalcar=animalcar->next;
		}
	}
	mechtoggle=(mechtoggle) ? 0 : 1;
	if (view!='i' && view!='w') map();
}

//this moves an animal
void chiken_move(animalp)
struct animal *animalp; {
//	fprintf(stderr, "chiken_move\n");
	FILE* file=fopen("/dev/urandom", "r");
	short c=(unsigned)fgetc(file)%5,
	      save=earth[animalp->x][animalp->y][animalp->z];
	fclose(file);
	earth[animalp->x][animalp->y][animalp->z]=0;
	if      (c==0 && animalp->x!=191) ++(animalp->x);
	else if (c==1 && animalp->x!=0  ) --(animalp->x);
	else if (c==2 && animalp->y!=191) ++(animalp->y);
	else if (c==3 && animalp->y!=0  ) --(animalp->y);
	earth[animalp->x][animalp->y][animalp->z]=save;
}

//this finds pointer to an animal with coordinates
struct animal *findanimal(x, y, z)
short x, y, z; {
//	fprintf(stderr, "findanimal\n");
	struct animal *animalcar=animalstart;
	while (animalcar!=NULL)
		if (x==animalcar->x &&
		    y==animalcar->y &&
		    z==animalcar->z) return(animalcar);
		else animalcar=animalcar->next;
}

//this adds new animal to list of loaded animals
void spawn(x, y, z, life)
short x, y, z,
      life; {
//	fprintf(stderr, "spawn\n");
	struct animal *animalcar;
	if (animalstart==NULL) animalstart=animalcar=malloc(sizeof(struct animal));
	else {
		for (animalcar=animalstart; animalcar->next!=NULL;
			animalcar=animalcar->next);
		animalcar=animalcar->next=malloc(sizeof(struct animal));
	}
	animalcar->x=x;
	animalcar->y=y;
	animalcar->z=z;
	animalcar->life=life;
	animalcar->next=NULL;
}

//this erases list of loaded animals
void eraseanimals() {
//	fprintf(stderr, "eraseanimals\n");
	if (animalstart!=NULL) {
		signal='s';
		fprintf(stderr, "signal %c\n", signal);
		struct animal *animalcar=animalstart;
		while (animalcar) {
			struct animal *animalerase=animalcar;
			animalcar=animalcar->next;
			free(animalerase);
		}
		animalstart=NULL;
		signal='w';
	}
}
