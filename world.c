/*Eyecube, sandbox game.
* Copyright (C) 2011 Alexander Kromm, see README file for details.
*/

#include "header.h"
#include <ncurses.h>
#include <stdlib.h>

extern char  signal, view;
extern short xp, yp, zp, earth[][3*WIDTH][HEAVEN+1], jump, notflag;

extern unsigned time;

//TODO: char instead of short
struct item radar[9];
extern short radar_dist;
extern struct for_sky sky[][39];

struct something *animalstart=NULL,
                 *cheststart =NULL,
                 *thingstart =NULL,
                 *heapstart  =NULL,
                 *lightstart =NULL;
void tolog();

char skymap[][78]={"                          .         . .                 .                     ",
"                    .   .       .   .                 .                       ",
"                                  .     .         .                           ",
"                          . .           .                                     ",
"                                        .           .                         ",
"                                .   .   .   .   .                             ",
"                          .           . . .     .                             ",
"                        .               .           .                         ",
"                    .                                                         ",
"                      .     .         .       .   .       .               . . ",
"                  .   . .         .   .   .               .         .         ",
".   .   . .     .       . . .   .                                             ",
"    . .       .                   .     .                   .   .         .   ",
"          .         .           .   .     . .       .                   .     ",
". .                 . .     .         .   .                     .             ",
"      .   .                                     .             . .   .     .   ",
"        .   .                             .   .       .             .         ",
"  .           .         .   .     .                   .   .                   ",
"      . .         .                                     .       .         .   ",
"        .               .               .               .   .         .       ",
"  .     .                   .   .     .     . . .                             ",
". .   .               . .               .     .     .                 .   .   ",
"              .     .                     .                 .     .     .     ",
"      .   .       . .           .           .     .                 .         ",
"  .     .                                           . .         .   .         ",
"      .             .         .                                 .   .         ",
"              .   .         .     .     .   .             .     .             ",
".   .     .     .                   .                                         ",
"            .     .             . . .   .                             .   .   ",
"            .                 .             .     . .     .         .         ",
"                      .         .   .     .               .                   ",
"                      .       .           .             .                     ",
"                    .         .             .   .                             ",
"                                          .             .                     ",
"                                                    . .                       ",
"                  .     .         .         .                                 ",
"                    . .         .   .                 . .                     ",
"                                  .       .     .                             ",
"                  .     .       .       .                                     "};

//drops block
int fall(x, y, z)
short x, y, z; {
	tolog("fall start\n");
	int   property();
	short height;
	for (height=0; property(earth[x][y][z-height-1], 'p'); ++height);
	if (height>0) {
		short env=earth[x][y][z-height];
		earth[x][y][z-height]=earth[x][y][z];
		earth[x][y][z]=env;
	}
	tolog("fall finish\n");
	return height;
}

//moves player
int step(x, y)
short x, y; {
	tolog("step start\n");
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
	tolog("step finish\n");
	return notc;
}

//pushes block from target xyz to changed xyz
void push(x_target, y_target, z_target, x_change, y_change)
short x_target, y_target, z_target,
      x_change, y_change; {
	tolog("push start\n");
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
	tolog("push finish\n");
}

//all mechanics events
void allmech() {
	tolog("allmech start\n");
	void map(), sounds_print(),
	     chiken_move();
	//sky section
	if (time>=6*60) {
		if (0==time%((24*60-1-6*60)/39)) {
			short sun_pos=38-((time-6*60)*39/(24*60-1-6*60));
			if (38!=sun_pos) sky[sun_pos+1][19].sun=NOTHING;
			sky[sun_pos][19].sun=SUN;
		}
	} else {
		if (0==time%((6*60)/39)) {
			short sun_pos=38-(time*39/(6*60));
			if (38!=sun_pos) sky[sun_pos+1][19].sun=NOTHING;
			sky[sun_pos][19].sun=MOON;
		}
	}
	switch (time) {
		case 0: {
			register short x, y;
			for (x=0; x<39; ++x)
			for (y=0; y<39; ++y) 
				sky[x][y].sky=('.'==skymap[y][2*x]) ?
					BLACK_STAR : BLACK_SKY;
			sky[ 0][19].sun=NOTHING;
			sky[38][19].sun=MOON;
		} break;
		case 6*60: sky[0][19].sun=NOTHING; sky[38][19].sun=SUN; //no break;
		case 23*60: {
			register short x, y;
			for (x=0; x<39; ++x)
			for (y=0; y<39; ++y)
				sky[x][y].sky=('.'==skymap[y][2*x]) ?
					BLUE_STAR : BLUE_SKY;
		} break;
		case 7*60: {
			register short x, y;
			for (x=0; x<39; ++x)
			for (y=0; y<39; ++y)
				sky[x][y].sky=BLUE_SKY;
		} break;
		case 12*60: {
			register short x, y;
			for (x=0; x<39; ++x)
			for (y=0; y<39; ++y)
				sky[x][y].sky=CYAN_SKY;
		} break;
		case 18*60: {
			register short x, y;
			for (x=0; x<39; ++x)
			for (y=0; y<39; ++y)
				sky[x][y].sky=BLUE_SKY;
		} break;
		default: break;
	}
	//physics section
	time=(24*60-1==time) ? 0 : time+1;
	if (signal=='w') {
		void move_down_chain();
		struct something *car;
		//move animals
		for (car=animalstart; NULL!=car; car=car->next) chiken_move(car);
		//heap lifetime
		if (0==time%60) {
			struct something *car_last=heapstart;
			car=heapstart;
			while (NULL!=car) if (0==--(car->arr[63]))
				if (car==heapstart) {
					heapstart=car->next;
					earth[car->arr[0]][car->arr[1]][car->arr[2]]=0;
					free(car->arr);
					free(car);
					car=heapstart;
				} else {
					car_last->next=car->next;
					earth[car->arr[0]][car->arr[1]][car->arr[2]]=0;
					free(car->arr);
					free(car);
					car=car_last->next;
				}
			else {
				car_last=car;
				car=car->next;
			}
		}
		//everything falls
		if (NULL!=heapstart  ) move_down_chain(&heapstart  );
		if (NULL!=animalstart) move_down_chain(&animalstart);
	}
	{ //gravity
		short height=0;
		for ( ; property(earth[xp][yp][zp-1], 'p'); ++height, --zp);
		if (height>1) { //gravity
			notify("You fell down.", 0);
			notflag=4;
			//damage
		}
	}
	{
		void sounds();
		sounds(animalstart);
	}
	sounds_print();
	map();
	tolog("allmech finish\n");
}

//this checks if sowething sounds and modifies radar array
void sounds(start)
struct something *start; {
	tolog("sounds start\n");
	int   property();
	char  name;
	for ( ; NULL!=start; start=start->next) {
		if (abs(start->arr[0]-xp)<=radar_dist &&
		    abs(start->arr[1]-yp)<=radar_dist &&
		    abs(start->arr[2]-zp)<=NEAR &&
		    (name=property(earth[start->arr[0]][start->arr[1]][start->arr[2]],
		   'o')) &&
		   (!(random_linux()%3))) {
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
	tolog("sounds finish\n");
}

void change_radar(i, height, name)
short i, height;
char  name; {
	tolog("change_radar start\n");
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
	tolog("change_radar finish\n");
}


//this moves an animal
void chiken_move(animalp)
struct something *animalp; {
	tolog("chiken_move start\n");
	int   random_linux();
	short c=(unsigned)random_linux()%5,
	      save=earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]];
	earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=0;
	if      (c==0 && animalp->arr[0]!=3*WIDTH-1 &&
		property(earth[animalp->arr[0]+1][animalp->arr[1]][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0]+1 && yp==animalp->arr[1] && zp==animalp->arr[2]))
			++(animalp->arr[0]);
	else if (c==1 && animalp->arr[0]!=0 &&
		property(earth[animalp->arr[0]-1][animalp->arr[1]][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0]-1 && yp==animalp->arr[1] && zp==animalp->arr[2]))
			--(animalp->arr[0]);
	else if (c==2 && animalp->arr[1]!=3*WIDTH-1 &&
		property(earth[animalp->arr[0]][animalp->arr[1]+1][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0] && yp==animalp->arr[1]+1 && zp==animalp->arr[2]))
			++(animalp->arr[1]);
	else if (c==3 && animalp->arr[1]!=0 &&
		property(earth[animalp->arr[0]][animalp->arr[1]-1][animalp->arr[2]], 'p')
		&& !(xp==animalp->arr[0] && yp==animalp->arr[1]-1 && zp==animalp->arr[2]))
			--(animalp->arr[1]);
	earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=save;
	tolog("chiken_move_finish\n");
}

//this adds new active thing to list of loaded animals
struct something *spawn(x, y, z, file)
short x, y, z;
FILE  *file; {
//	tolog("spawn start\n");
	int  property();
	char type=property(earth[x][y][z], 'n');
	if (type) {
		struct something **start,
		                 *thing_new;
		switch (type) {
			case 'a': start=&animalstart; break; //animal
			case 'c': start=&cheststart;  break; //chest
			case 'h': start=&heapstart;   break; //heap
			case 'l': start=&lightstart;  break; //light
			case 't': start=&thingstart;  break; //thing
		}
		(thing_new=malloc(sizeof(struct something)))->next=*start;
		*start=thing_new;
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
					//put something to all new chests
					thing_new->arr[4 ]=COMPASS;
					thing_new->arr[34]=9;
					//and some stones
					/*thing_new->arr[5 ]=STONE;
					thing_new->arr[35]=9;*/
					//
					if ('h'==type) thing_new->arr[i]=24; //hours
				} else
					while (i<63+(('h'==type) ? 1 : 0))
						thing_new->arr[i++]=getc(file);
			} break;
			case 'l': {//light
				int light_radius();
				thing_new->arr=malloc(4*sizeof(short));
				thing_new->arr[3]=1; //hour
			} break;
			case 't': //thing
				thing_new->arr=malloc(3*sizeof(short));
			break;
		}
		thing_new->arr[0]=x;
		thing_new->arr[1]=y;
		thing_new->arr[2]=z;
//		tolog("spawn finish\n");
		return thing_new;
	} else {
		return NULL;
//		tolog("spawn finish\n");
	}
}

//this erases list of loaded animals
void eraseanimals() {
	tolog("eraseanimals start\n");
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
	tolog("eraseanimals finish\n");
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
struct something *erase_by_xyz(x, y, z, chain)
short x, y, z;
struct something *chain; {
	tolog("erase_by_xyz start\n");
	struct something *car, *save;
	for (car=chain; !(x==car->arr[0] &&
	                  y==car->arr[1] &&
	                  z==car->arr[2]); car=car->next) save=car;
	if (car!=chain) {
		save->next=car->next;
		save=heapstart;
	} else save=heapstart->next;
	free(car->arr);
	free(car);
	tolog("erase_by_xyz finish\n");
	return save;
}

//moves down all elements of a chain and does some service
void move_down_chain(chain_start)
struct something **chain_start; {
	tolog("move_down_chain start\n");
	int  fall(), property();
	struct something *chain=*chain_start,
	                 *chain_last=chain;
	short empty_flag;
	do {
		empty_flag=0;
		chain->arr[2]-=fall(chain->arr[0], chain->arr[1], chain->arr[2]);
		if (property(earth[chain->arr[0]][chain->arr[1]][chain->arr[2]],   'c') &&
		    property(earth[chain->arr[0]][chain->arr[1]][-1+chain->arr[2]], 'c'))
				{
			//falling into
			tolog("falling into\n");
			short i, j;
			struct something *findanimal(),
			                 *lower_chest=findanimal(chain->arr[0],
			                 chain->arr[1], chain->arr[2]-1);
			for (i=3; i<=33; ++i) if (chain->arr[i])
				for (j=3; j<=33; ++j) if (0==lower_chest->arr[j] ||
						(lower_chest->arr[j]==chain->arr[i] &&
						 lower_chest->arr[j+30]<9 &&
						 property(chain->arr[i], 's'))) {
					lower_chest->arr[j]=chain->arr[i];
					while (lower_chest->arr[j+30]<9 &&
							chain->arr[i+30]>0) {
						++lower_chest->arr[j+30];
						--chain->arr[i+30];
					}
					if (0==chain->arr[i+30]) {
						chain->arr[i]=0;
						break;
					}
				}
			tolog("fell into\n");
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
	} while (NULL!=chain);
	tolog("move_down_chain finish\n");
}

//drops item from hand or from closing chest
int drop_thing(thing)
struct item *thing; {
	tolog("drop_thing start\n");
	int   property();
	void  focus();
	short x, y, z;
	focus(&x, &y, &z);
	if (thing->what && (property(earth[x][y][z], 'p') ||
			property(earth[x][y][z], 'c'))) {
		struct something *drop_into;
		short i;
		if (property(earth[x][y][z], 'c')) {
			struct something *findanimal();
			drop_into=findanimal(x, y, z);
		} else {
			struct something *spawn();
			earth[x][y][z]=8;
			drop_into=spawn(x, y, z, NULL);
		}
		for (i=3; i<33; ++i) if (0==drop_into->arr[i] || 
				(drop_into->arr[i]==thing->what &&
				 drop_into->arr[i+30]<9 &&
				 property(thing->what, 's'))) {
			drop_into->arr[i]=thing->what;
			while (drop_into->arr[i+30]<9 && thing->num>0) {
				++drop_into->arr[i+30];
				--thing->num;
			}
			if (0==thing->num) break;
		}
		thing->what=0;
		if (thing->num) {
			thing->num=0;
			return 14; //losing thing
		} else return 11; //"You drop something."
	} else return 12; //"Nothing to drop"
}
