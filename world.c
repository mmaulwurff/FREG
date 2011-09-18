#include "header.h"
#include <ncurses.h>
#include <stdlib.h>

extern char  signal, view;
extern short xp, yp, zp, earth[][192][HEAVEN+1], jump, notflag;

short mechtoggle=0;

struct something *animalstart=NULL,
                 *cheststart =NULL,
                 *thingstart =NULL;
//drops block
int fall(x, y, z)
short x, y, z; {
	short h,
	      save=earth[x][y][z];
	for (h=0; earth[x][y][z-1]==0; --z, ++h);
	earth[x][y][z]=save;
	return(h);
}

//moves player
int step(x, y)
short x, y; {
	void  onbound();
	int   property();
	short notc=0;
	if (jump==1)
		if (property(earth[xp][yp][zp+2], 'p')) ++zp;
		else notc=5;
	else if (jump==2) {
		if (property(earth[xp+x][yp+y][zp  ], 'p') &&
		    property(earth[xp+x][yp+y][zp+1], 'p')) {
			xp+=x;
			yp+=y;
		}
		else notc=1;
	}
	if (property(earth[xp+x][yp+y][zp  ], 'p') &&
	    property(earth[xp+x][yp+y][zp+1], 'p')) {
		xp+=x;
		yp+=y;
	}
	else notc=1;
	jump=0;
	onbound();
	return(notc);
}

//all mechanics events
void allmech() {
	void map(),
	     chiken_move();
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
	if      (c==0 && animalp->arr[0]!=191 &&
		property(earth[animalp->arr[0]+1][animalp->arr[1]][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0]+1 && yp==animalp->arr[1] && zp==animalp->arr[2]))
			++(animalp->arr[0]);
	else if (c==1 && animalp->arr[0]!=0 &&
		property(earth[animalp->arr[0]-1][animalp->arr[1]][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0]-1 && yp==animalp->arr[1] && zp==animalp->arr[2]))
			--(animalp->arr[0]);
	else if (c==2 && animalp->arr[1]!=191 &&
		property(earth[animalp->arr[0]][animalp->arr[1]+1][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0] && yp==animalp->arr[1]+1 && zp==animalp->arr[2]))
			++(animalp->arr[1]);
	else if (c==3 && animalp->arr[1]!=0 &&
		property(earth[animalp->arr[0]][animalp->arr[1]-1][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0] && yp==animalp->arr[1]-1 && zp==animalp->arr[2]))
			--(animalp->arr[1]);
	earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=save;
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
				(*animalcar)->arr=malloc(63*sizeof(short));
				short i=3;
				if (file==NULL) {
					while (i<63) (*animalcar)->arr[i++]=0;
					//put helmet to all new chests
					(*animalcar)->arr[4 ]=6;
					(*animalcar)->arr[34]=1;
				} else
					while (i<63) (*animalcar)->arr[i++]=getc(file);
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
	void erasein();
	signal='s';
	erasein(animalstart);
	animalstart=NULL;
	erasein(thingstart );
	thingstart =NULL;
	erasein(cheststart );
	cheststart =NULL;
	signal='w';
}

//inside eraseanimals()
void erasein(car)
struct something *car; {
	while (car!=NULL) {
		struct something *animalerase=car;
		car=car->next;
		free(animalerase->arr);
		free(animalerase);
	}
}
