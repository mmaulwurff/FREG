#include "header.h"
#include <ncurses.h>
#include <stdlib.h>

extern char  signal, view;
extern short xp, yp, zp, earth[][192][HEAVEN+1], jump, notflag;

short mechtoggle=0;

struct something *animalstart=NULL,
                 *cheststart =NULL,
                 *thingstart =NULL,
		 *heapstart  =NULL;
//drops block
int fall(x, y, z)
short x, y, z; {
	int   property();
	short h;
	for (h=0; property(earth[x][y][z-h-1], 'p'); ++h);
//	fprintf(stderr, "fall: h=%d\n", h);
	if (h>0) {
		short env=earth[x][y][z-h];
		earth[x][y][z-h]=earth[x][y][z];
		earth[x][y][z]=env;
	}
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
//	fprintf(stderr, "allmech: moving is ok\n");
	{ //gravity
		short height=0;
		for ( ; property(earth[xp][yp][zp-1], 'p'); ++height, --zp);
		if (height>1) { //gravity
			notify("You fell down.", 0);
			notflag=4;
			//damage
		}
	}
	//clock should be here
	//sand falls, heaps dissapear, night and day come
	{ //everything falls
		void move_down_chain();
		if (NULL!=heapstart  ) move_down_chain(&heapstart  );
		if (NULL!=animalstart) move_down_chain(&animalstart);
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
struct something *spawn(x, y, z, file)
short x, y, z;
FILE  *file; {
	//fprintf(stderr, "spawn\n");
	int  property();
	char type=property(earth[x][y][z], 'n');
	if (type) {
		short if_heap=0;
		struct something **animalcar,
				 **start,
				 *save;
		switch (type) {
			case 'a': start=&animalstart; break; //animal
			case 'c': start=&cheststart;  break; //chest
			case 'h': start=&heapstart;   break; //heap
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
			case 'h': case 'c': { //heap or chest
				(*animalcar)->arr=
					malloc((('h'==type) ? 1 : 0)+63*sizeof(short));
				short i=3;
				if (file==NULL) {
					while (i<63) (*animalcar)->arr[i++]=0;
					//put helmet to all new chests
					(*animalcar)->arr[4 ]=6;
					(*animalcar)->arr[34]=1;
					//
					if ('h'==type) (*animalcar)->arr[i]=24;
				} else
					while (i<63+(('h'==type) ? 1 : 0))
						(*animalcar)->arr[i++]=getc(file);
			} break;
			case 't': //thing
				(*animalcar)->arr=malloc( 3*sizeof(short));
			break;
		}
		(*animalcar)->arr[0]=x;
		(*animalcar)->arr[1]=y;
		(*animalcar)->arr[2]=z;
		(*animalcar)->next=NULL;
		save=*animalcar;
		free(animalcar);
		return save;
	} else return NULL;
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
	erasein(heapstart  );
	heapstart  =NULL;
	signal='w';
}

//inside eraseanimals()
void erasein(car)
struct something *car; {
	struct something *animalerase;
	while (car!=NULL) {
		animalerase=car;
		car=car->next;
		free(animalerase->arr);
		free(animalerase);
	}
}

//erases one thing from list with coordinates
void erase_by_xyz(x, y, z, chain)
short x, y, z;
struct something **chain; {
//	fprintf(stderr, "erase_by_xyz started\n");
	struct something *to_erase;
	if (!(x==(*chain)->arr[0] && y==(*chain)->arr[1] && z==(*chain)->arr[2])) {
		while (NULL!=(*chain)->next &&
			!((*((*chain)->next)).arr[0]==x &&
			  (*((*chain)->next)).arr[1]==y &&
			  (*((*chain)->next)).arr[2]==z)) {
//			fprintf(stderr, "erase_by_xyz next\n");
			*chain=(*chain)->next;
		}
//		fprintf(stderr, "erase_by_xyz found to erase\n");
		to_erase=(*chain)->next;
		(*chain)->next=(*((*chain)->next)).next;
	} else {	
		to_erase=*chain;
		*chain=(*chain)->next;
	}
//	fprintf(stderr, "erase_by_xyz ready to erase\n");
	free(to_erase->arr);
	free(to_erase);
}

//moves down all elements of a chain
void move_down_chain(chain_start)
struct something **chain_start; {
	int  fall(), property();
	struct something *chain=*chain_start,
			 *chain_last=chain;
//	fprintf(stderr, "move_down_chain: started\n");
	do {
		chain->arr[2]-=fall(chain->arr[0], chain->arr[1], chain->arr[2]);
		if (property(earth[chain->arr[0]][chain->arr[1]][chain->arr[2]-1], 'd')) {
		 	//environment instead of zero
			earth[chain->arr[0]][chain->arr[1]][chain->arr[2]]=0;
			if (chain==*chain_start) *chain_start=chain->next;
			chain_last->next=chain->next;
			free(chain->arr);
			free(chain);
			chain=chain_last->next;
		}
		if (NULL!=chain) {
			chain_last=chain;
			chain=chain->next;
		}
//		fprintf(stderr, "move_down_chain: one\n");
	} while (NULL!=chain);
}
