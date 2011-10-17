#include "header.h"
#include <ncurses.h>
#include <stdlib.h>

extern char  signal, view;
extern short xp, yp, zp, earth[][192][HEAVEN+1], jump, notflag;

short mechtoggle=0;

//TODO: char instead of short
struct item radar[9];
extern short radar_dist;

struct something *animalstart=NULL,
                 *cheststart =NULL,
                 *thingstart =NULL,
		 *heapstart  =NULL;

//drops block
int fall(x, y, z)
short x, y, z; {
	int   property();
	short height;
	for (height=0; property(earth[x][y][z-height-1], 'p'); ++height);
//	fprintf(stderr, "fall: height=%d\n", height);
	if (height>0) {
		short env=earth[x][y][z-height];
		earth[x][y][z-height]=earth[x][y][z];
		earth[x][y][z]=env;
	}
	return height;
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
		} else notc=1;
	}
	if (property(earth[xp+x][yp+y][zp+1], 'p'))
		if (property(earth[xp+x][yp+y][zp], 'p')) {
			xp+=x;
			yp+=y;
		} else if (property(earth[xp+x][yp+y][zp], 'm') &&
				property(earth[xp+2*x][yp+2*y][zp], 'p')) {
			void push();
			push(xp+x, yp+y, zp, x, y);
			xp+=x;
			yp+=y;
			notc=13;
		} else notc=1;
	jump=0;
	onbound();
	return notc;
}

//pushes block from target xyz to changed xyz
void push(x_target, y_target, z_target, x_change, y_change)
short x_target, y_target, z_target,
      x_change, y_change; {
	struct something *findanimal(),
	                 *target=findanimal(x_target, y_target, z_target);
	short save=earth[x_target+x_change]
	                [y_target+y_change]
	                [z_target];
	if (NULL!=target) {
		target->arr[0]+=x_change;
		target->arr[1]+=y_change;
	}
	earth[x_target+x_change][y_target+y_change][z_target]=
		earth[x_target][y_target][z_target];
	earth[x_target][y_target][z_target]=save;
}

//all mechanics events
void allmech() {
	void map(), sounds_print(),
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
	{
		void sounds();
		sounds(animalstart);
	}
	sounds_print();
	if (view!='i' && view!='w') map();
}

//this checks if sowething sounds and modifies radar array
void sounds(start)
struct something *start; {
//	fprintf(stderr, "sounds started\n");
	int   property();
	char  name;
	for ( ; NULL!=start; start=start->next) {
//		fprintf(stderr, "sounds inside: %d, %d, %d\n", abs(xp-start->arr[0]),
//				abs(yp-start->arr[1]), abs(zp-start->arr[2]));
		if (abs(start->arr[0]-xp)<=radar_dist &&
		    abs(start->arr[1]-yp)<=radar_dist &&
		    abs(start->arr[2]-zp)<=NEAR &&
		    (name=property(earth[start->arr[0]][start->arr[1]][start->arr[2]],
		   'o'))) {
			void change_radar();
			// ^
			if ((start->arr[1]-yp)<-abs(start->arr[0]-xp))
				change_radar(1, start->arr[2]-zp, name);
			// <
			else if (start->arr[0]-xp<-abs(start->arr[1]-yp))
				change_radar(3, start->arr[2]-zp, name);
			// v
			else if (start->arr[1]-yp>abs(start->arr[0]-xp))
				change_radar(7, start->arr[2]-zp, name);
			// >
			else if (start->arr[0]-xp>abs(start->arr[1]-yp))
				change_radar(5, start->arr[2]-zp, name);
			// <v
			else if (start->arr[0]-xp==yp-start->arr[1] &&
					start->arr[0]-xp<0)
				change_radar(6, start->arr[2]-zp, name);
			// >^
			else if (start->arr[0]-xp==yp-start->arr[1] &&
					start->arr[0]-xp>0)
				change_radar(2, start->arr[2]-zp, name);
			// <^
			else if (start->arr[0]-xp==start->arr[1]-yp &&
					start->arr[0]-xp<0)
				change_radar(0, start->arr[2]-zp, name);
			// >v
			else if (start->arr[0]-xp==start->arr[1]-yp &&
					start->arr[0]-xp>0)
				change_radar(8, start->arr[2]-zp, name);
			// .
			else change_radar(4, start->arr[2]-zp, name);
			}
	}
}

void change_radar(i, height, name)
short i, height;
char  name; {
//	fprintf(stderr, "change_radar started\n");
	if (' '!=radar[i].what && radar[i].what!=name) {
		radar[i].what='#';
		radar[i].num= (radar[i].num-+height)/2;
	} else {
		radar[i].what=name;
		radar[i].num =height;
	}
/*	fprintf(stderr, "%c%c%c%c%c%c\n%c%c%c%c%c%c\n%c%c%c%c%c%c\n",
		radar[0].what, radar[0].num, radar[1].what, radar[1].num,
		radar[2].what, radar[2].num,
		radar[3].what, radar[3].num, radar[4].what, radar[4].num,
		radar[5].what, radar[5].num,
		radar[6].what, radar[6].num, radar[7].what, radar[7].num,
		radar[8].what, radar[8].num);*/
}


//this moves an animal
void chiken_move(animalp)
struct something *animalp; {
//	fprintf(stderr, "chiken_move\n");
	int   random_linux();
	short c=(unsigned)random_linux()%5,
	      save=earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]];
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
		struct something **start,
		                 *thing_new;
		switch (type) {
			case 'a': start=&animalstart; break; //animal
			case 'c': start=&cheststart;  break; //chest
			case 'h': start=&heapstart;   break; //heap
			case 't': start=&thingstart;  break; //thing
		}
		(thing_new=malloc(sizeof(struct something)))->next=*start;
		*start=thing_new;
//		fprintf(stderr, "spawn3\n");
		switch (type) {
			case 'a': //animal
				thing_new->arr=malloc(4*sizeof(short));
				thing_new->arr[3]=(file==NULL) ? 9 : getc(file);
			break;
			case 'h': case 'c': { //heap or chest
				short i=3;
				thing_new->arr=
					malloc((('h'==type) ? 1 : 0)+63*sizeof(short));
				if (file==NULL) {
					while (i<63) thing_new->arr[i++]=0;
					//put helmet to all new chests
					thing_new->arr[4 ]=6;
					thing_new->arr[34]=9;
					//
					if ('h'==type) thing_new->arr[i]=24;
				} else
					while (i<63+(('h'==type) ? 1 : 0))
						thing_new->arr[i++]=getc(file);
			} break;
			case 't': //thing
				thing_new->arr=malloc(3*sizeof(short));
			break;
		}
		thing_new->arr[0]=x;
		thing_new->arr[1]=y;
		thing_new->arr[2]=z;
		return thing_new;
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

//moves down all elements of a chain and does some service
void move_down_chain(chain_start)
struct something **chain_start; {
	int  fall(), property();
	struct something *chain=*chain_start,
	                 *chain_last=chain;
	short empty_flag;
//	fprintf(stderr, "move_down_chain: started\n");
	do {
		empty_flag=0;
		chain->arr[2]-=fall(chain->arr[0], chain->arr[1], chain->arr[2]);
		if (property(earth[chain->arr[0]][chain->arr[1]][chain->arr[2]],   'c') &&
		    property(earth[chain->arr[0]][chain->arr[1]][chain->arr[2]-1], 'c')) {
			//falling into
			short i, j;
			struct something *findanimal(),
			                 *lower_chest=findanimal(chain->arr[0],
			                 chain->arr[1], chain->arr[2]-1);
			for (i=3; i<=33; ++i) if (chain->arr[i])
				for (j=3; j<=33; ++j) if (!lower_chest->arr[j]) {
//					fprintf(stderr, "one thing fell into\n");
					lower_chest->arr[j   ]=chain->arr[i   ];
					lower_chest->arr[j+30]=chain->arr[i+30];
					chain->arr[i]=chain->arr[i+30]=0;
					break;
				}
			empty_flag=1;
			for (i=3; i<=33; ++i) if (chain->arr[i]) { //if empty, delete
				empty_flag=0;
				break;
			}
		}
		if (property(earth[chain->arr[0]][chain->arr[1]][chain->arr[2]-1], 'd') ||
				empty_flag) { //delete if danger or empty
		 	//TODO: environment instead of zero:
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
