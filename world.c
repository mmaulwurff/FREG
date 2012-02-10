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
#include <ncurses.h>

extern struct for_sky sky[][39];
extern struct item    *craft;

extern short    xp, yp, zp, earth[][3*WIDTH][HEAVEN+1], jump, notflag, view, *opened,
                radar_dist;
extern unsigned time;

//TODO: char instead of short
struct item radar[9];

struct something *animalstart   =NULL,
                 *cheststart    =NULL,
                 *thingstart    =NULL,
                 *heapstart     =NULL,
                 *lightstart    =NULL,
                 *workbenchstart=NULL,
                 *signstart     =NULL;
struct list_to_clear *clear_start=NULL;
void tolog();

char skymap[][78]={
"                          .         . .                 .                     ",
"                    .   .       .   .                 .                       ",
"                                  .        ,      .                           ",
"                          . .                                                 ",
"                                       .            .                         ",
"                                .   .       .   .                             ",
"                          .               .     .                             ",
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
"                                . . .     .             .                     ",
"                              .       .             . .                       ",
"                  .     .     .       .     .                                 ",
"                    . .       .       .               . .                     ",
"                                . . .     .     .                             ",
"                  .     .       .       .                                     "};

//drops block
int fall(x, y, z)
short x, y, z; {
	tolog("fall start\n");
	int   passable();
	short height;
	for (height=0; passable(earth[x][y][z-height-1]); ++height);
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
	int   passable(), movable(), chestlike();
	short notc=0;
	if (jump==1)
		if (passable(earth[xp][yp][zp+2])) ++zp;
		else notc=5;
	else if (jump==2)
		if (passable(earth[xp+x][yp+y][zp  ]) &&
		    passable(earth[xp+x][yp+y][zp+1])) {
			xp+=x;
			yp+=y;
		} else notc=1;
	if (passable(earth[xp+x][yp+y][zp+1]))
		if (passable(earth[xp+x][yp+y][zp])) {
			xp+=x;
			yp+=y;
		} else if (movable(earth[xp+x][yp+y][zp]) &&
				passable(earth[xp+2*x][yp+2*y][zp])) {
			void push();
			push(xp+x, yp+y, zp, x, y);
			xp+=x;
			yp+=y;
			notc=13;
		} else if (PILE==earth[xp+x][yp+y][zp] &&
				chestlike(earth[xp+2*x][yp+2*y][zp])) {
			struct something *findanimal(),
					 *push_into=findanimal(xp+2*x, yp+2*y, zp),
					 *push_from=findanimal(xp+  x, yp+  y, zp);
			int chest_size(), minus(),
			    pour_into();
			fprintf(stderr, "chestsize: %d\nminus: %d\n",
				chest_size(earth[xp+2*x][yp+2*y][zp]),
				minus(earth[xp+2*x][yp+2*y][zp]));
			if (pour_into(push_from->arr, chest_size(earth[xp+x][yp+y][zp]),
					push_into->arr,
					chest_size(earth[xp+2*x][yp+2*y][zp]),
					minus(earth[xp+2*x][yp+2*y][zp]))) {
				void new_clear();
				earth[xp+x][yp+y][zp]=0;
				push_from->arr[0]=HEAVEN+1;
				new_clear(push_from);
				xp+=x;
				yp+=y;
			}
		} else notc=1;
	else notc=1;
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
	if (HEAVEN+1!=target->arr[0]) {
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
	tolog("push finish\n");
}

//all mechanics events
void allmech() {
	void del_expired();
	tolog("allmech start\n");
	//one-time events
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
	/* every hour events*/ {
		if (0==time%60) del_expired(&heapstart);
	}
	/* every second events */ {
		void move_down_chain(),
		     chiken_move(),
		     sounds(), sounds_print(),
		     map();
		struct something *car;
		time=(24*60-1==time) ? 0 : time+1;
		del_expired(&lightstart);
		//everything falls
		if (NULL!=heapstart  ) move_down_chain(&heapstart  );
		if (NULL!=animalstart) move_down_chain(&animalstart);
		//move animals
		for (car=animalstart; NULL!=car; car=car->next) chiken_move(car);
		{ //gravity
			void  notify();
			int   passable();
			short height=0;
			for ( ; passable(earth[xp][yp][zp-1]); ++height, --zp);
			if (height>1) { //gravity
				notify("You fell down.", 0);
				notflag=4;
				//damage
			}
		}
		sounds(animalstart);
		sounds_print();
		if (VIEW_SURFACE==view || VIEW_FLOOR==view || VIEW_HEAD==view ||
				VIEW_SKY==view || VIEW_FRONT==view) map();
	}
	tolog("allmech finish\n");
}

void del_expired(start)
struct something **start; {
	void new_clear();
	struct something *car_last=*start,
	                 *car=*start;
	short life_index;
	if (*start==heapstart) life_index=63;
	else if (*start==lightstart) life_index=3;
	else return;
	while (NULL!=car) if (0==--(car->arr[life_index]))
		if (car==*start) {
			*start=car->next;
			earth[car->arr[0]][car->arr[1]][car->arr[2]]=0;
			car->arr[0]=HEAVEN+1;
			new_clear(car);
			car=*start;
		} else {
			car_last->next=car->next;
			earth[car->arr[0]][car->arr[1]][car->arr[2]]=0;
			car->arr[0]=HEAVEN+1;
			new_clear(car);
			car=car_last->next;
		}
	else {
		car_last=car;
		car=car->next;
	}
}
	
//this creates nem item in clear_list
void new_clear(address)
struct something *address; {
	struct list_to_clear *new_item;
//	earth[address->arr[0]][address->arr[1]][address->arr[2]]=0;
	address->arr[1]=1;
	address->arr[2]=1;
	(new_item=malloc(sizeof(struct list_to_clear)))->next=clear_start;
	clear_start=new_item;
	new_item->address=address;
}

//this cheks items of list to clear and frees them when needed
void free_clear_list(proc)
short proc; {
	struct list_to_clear *car, *prevcar=car;
	for (car=clear_start; NULL!=car; ) {
		car->address->arr[proc+1]=0;
		if (!car->address->arr[1+!proc]) {
			struct list_to_clear *to_free;
			if (clear_start==car) {
				clear_start=car->next;
				free(car->address->arr);
				free(car->address);
				free(car);
				car=clear_start;
			} else {
				prevcar->next=car->next;
				free(car->address->arr);
				free(car->address);
				free(car);
				car=prevcar->next;
			}
		} else car=car->next;
	}
}

//this checks if something happens and modifies radar array
void sounds(start)
struct something *start; {
	tolog("sounds start\n");
	char name;
	for ( ; NULL!=start; start=start->next) {
		if (abs(start->arr[0]-xp)<=radar_dist &&
		    abs(start->arr[1]-yp)<=radar_dist &&
		    abs(start->arr[2]-zp)<=NEAR &&
		    (name=getname(earth[start->arr[0]][start->arr[1]][start->arr[2]],
				  NULL))!='?' && !(random_linux()%3)) {
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
	if (HEAVEN+1!=animalp->arr[0]) {
		int   random_linux();
		int   passable();
		short c=(unsigned)random_linux()%5,
		      save=earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]];
		earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=0;
		if (c==0 && animalp->arr[0]!=3*WIDTH-1 &&
				passable(earth[animalp->arr[0]+1][animalp->arr[1]]
					[animalp->arr[2]]) &&
				!(xp==animalp->arr[0]+1 && yp==animalp->arr[1] &&
					zp==animalp->arr[2]))
			++(animalp->arr[0]);
		else if (c==1 && animalp->arr[0]!=0 &&
				passable(earth[animalp->arr[0]-1][animalp->arr[1]]
					[animalp->arr[2]]) &&
				!(xp==animalp->arr[0]-1 && yp==animalp->arr[1] &&
					zp==animalp->arr[2]))
			--(animalp->arr[0]);
		else if (c==2 && animalp->arr[1]!=3*WIDTH-1 &&
				passable(earth[animalp->arr[0]][animalp->arr[1]+1]
					[animalp->arr[2]]) &&
				!(xp==animalp->arr[0] && yp==animalp->arr[1]+1 &&
					zp==animalp->arr[2]))
			++(animalp->arr[1]);
		else if (c==3 && animalp->arr[1]!=0 &&
				passable(earth[animalp->arr[0]][animalp->arr[1]-1]
					[animalp->arr[2]]) &&
				!(xp==animalp->arr[0] && yp==animalp->arr[1]-1 &&
				       	zp==animalp->arr[2]))
			--(animalp->arr[1]);
		earth[animalp->arr[0]][animalp->arr[1]][animalp->arr[2]]=save;
	}
	tolog("chiken_move_finish\n");
}

//this adds new active thing to list of loaded animals
struct something *spawn(x, y, z, file)
short x, y, z;
FILE  *file; {
//	tolog("spawn start\n");
	struct something **start,
	                 *thing_new;
	short length;
	switch (earth[x][y][z]) {
		case CHICKEN:   start=&animalstart;    length= 4; break;
		case CHEST:     start=&cheststart;     length=63; break;
		case PILE:      start=&heapstart;      length=64; break;
		case FIRE:      start=&lightstart;     length= 4; break;
		case WORKBENCH: start=&workbenchstart; length=23; break;
		case SIGN:      start=&signstart;      length=63; break;
//		case : start=&thingstart;  length=3;  break;
		default : return NULL; break;
	}
	(thing_new=malloc(sizeof(struct something)))->next=*start;
	*start=thing_new;
	thing_new->arr=malloc(length*sizeof(short));
	if (file) {
		short i;
		for (i=3; i<length; ++i) thing_new->arr[i]=getc(file);
	} else switch (earth[x][y][z]) {
		case CHICKEN: thing_new->arr[ 3]= 9; break; //HP
		case FIRE:    thing_new->arr[ 3]= 5; break; //minutes
		case PILE:    thing_new->arr[63]=24; //hours //no break
		case CHEST: {
			short i;
			for (i=3; i<63; ++i) thing_new->arr[i]=0;
			//put something to all new chests
			thing_new->arr[4 ]=IRON_INGOT;
			thing_new->arr[34]=9;
			//and some stones
			/*thing_new->arr[5 ]=STONE;
			thing_new->arr[35]=9;*/
		} break;
		case WORKBENCH: {
//			short i;
//			for (i=3; i<23; ++i) thing_new->arr[i]=9;
		} break;
		case SIGN: {
			short i;
			while (thing_new->arr[3+i]="This is a sign, hello player! Eyecube greet you."[i++]);
		} break;
		default: break;
	}
	thing_new->arr[0]=x;
	thing_new->arr[1]=y;
	thing_new->arr[2]=z;
//	tolog("spawn finish\n");
	return thing_new;
}

//this erases list of loaded animals
void eraseanimals(final)
short final; {
	tolog("eraseanimals start\n");
	void erasein();
	erasein(animalstart, final);
	animalstart=NULL;
	erasein(thingstart,  final);
	thingstart =NULL;
	erasein(cheststart,  final);
	cheststart =NULL;
	erasein(heapstart,   final);
	heapstart  =NULL;
	tolog("eraseanimals finish\n");
}

//inside eraseanimals()
void erasein(car, final)
struct something *car; {
	void new_clear();
	struct something *animalerase;
	while (car!=NULL) {
		animalerase=car;
		car=car->next;
		if (final) {
			free(animalerase->arr);
			free(animalerase);
		} else {
			animalerase->arr[0]=HEAVEN+1;
			new_clear(animalerase);
		}
	}
}

//erases one thing from list with coordinates
struct something *erase_by_xyz(x, y, z, chain)
short x, y, z;
struct something *chain; {
	tolog("erase_by_xyz start\n");
	void new_clear();
	struct something *car, *save;
	for (car=chain; !(x==car->arr[0] &&
	                  y==car->arr[1] &&
	                  z==car->arr[2]); car=car->next) save=car;
	if (car!=chain) {
		save->next=car->next;
		save=heapstart;
	} else save=heapstart->next;
	car->arr[0]=HEAVEN+1;
	new_clear(car);
	tolog("erase_by_xyz finish\n");
	return save;
}

//moves down all elements of a chain and does some service
void move_down_chain(chain_start)
struct something **chain_start; {
	tolog("move_down_chain start\n");
	int  fall(), chestlike(), dangerous();
	struct something *chain=*chain_start,
	                 *chain_last=chain;
	short empty_flag;
	do {
		empty_flag=0;
		if (HEAVEN+1!=chain->arr[0]) {
			chain->arr[2]-=fall(chain->arr[0], chain->arr[1], chain->arr[2]);
			if (chestlike(earth[chain->arr[0]][chain->arr[1]]
					[chain->arr[2]]) &&
					chestlike(earth[chain->arr[0]][chain->arr[1]]
						[chain->arr[2]-1])) {
				struct something *findanimal(),
				                 *lower_chest=findanimal(chain->arr[0],
				                 chain->arr[1], chain->arr[2]-1);
				empty_flag=pour_into(chain->arr, 30,
				                     lower_chest->arr, 30);
			}
			if (dangerous(earth[chain->arr[0]][chain->arr[1]]
					[chain->arr[2]-1]) || empty_flag) {
				//delete if danger or empty
			 	//TODO: environment instead of zero:
				void new_clear();
				earth[chain->arr[0]][chain->arr[1]][chain->arr[2]]=0;
				if (chain==*chain_start) *chain_start=chain->next;
				chain_last->next=chain->next;
				chain->arr[0]=HEAVEN+1;
				new_clear(chain);
				chain=chain_last->next;
			}
		}
		if (NULL!=chain) {
			chain_last=chain;
			chain=chain->next;
		}
	} while (NULL!=chain);
	tolog("move_down_chain finish\n");
}

//this pours from things one array to another
int pour_into(from, from_length, to, to_length, min)
short *from, *to,
       from_length, to_length,
       min; {
	tolog("pour_into start\n");
	int   stackable();
	short i, j,
	      empty_flag=1;
	fprintf(stderr, "min=%d\n", min);
	for (i=3; i<from_length+3; ++i) if (from[i]) {
		if (stackable(from[i]))
			for (j=3; j<to_length+3-min; ++j) {
				if (0==to[j]) to[j]=from[i];
				if (to[j]==from[i]) {// && to[j+30]<stack_len) {
					while (to[j+to_length]<9 && from[i+from_length]) {
						++to  [j+  to_length];
						--from[i+from_length];
					}
					if (0==from[i+from_length]) {
						from[i]=0;
						break;
					}
				}
			}
		else
			for (j=3; j<=to_length+3-min; ++j)
				if (0==to[j]) {
					to  [j]=from[i];
					from[i]=0;
					to  [j+to_length]  =from[i+from_length];
					from[i+from_length]=0;
				}
	}
	for (i=3; i<=from_length; ++i) if (from[i]) {
		//if empty, delete
		empty_flag=0;
		break;
	}
	tolog("pour into finish\n");
	return empty_flag;
}

//drops item from hand or from closing chest
int drop_thing(thing)
struct item *thing; {
	tolog("drop_thing start\n");
	if (thing->what) {
		int   passable(), chestlike();
		short x, y, z;
		void  focus();
		focus(&x, &y, &z);
		if ((passable(earth[x][y][z]) || chestlike(earth[x][y][z]))) {
			struct something *drop_into;
			int   stackable();
			short i, for_len=30;
			switch (chestlike(earth[x][y][z])) {
				case  1: {
					struct something *findanimal();
					drop_into=findanimal(x, y, z);
					//if drop_into is already deleted, make new
					if (HEAVEN+1!=drop_into->arr[0]) break;
				}
				default: {
					earth[x][y][z]=PILE;
					struct something *spawn();
					drop_into=spawn(x, y, z, NULL);
					break;
				}
			}
			if (stackable(thing->what))
				for (i=3; i<for_len+3; ++i) {
					if (0==drop_into->arr[i])
						drop_into->arr[i]=thing->what;
					if (drop_into->arr[i]==thing->what) {
						while (drop_into->arr[i+for_len]<9 &&
								thing->num>0) {
							++drop_into->arr[i+for_len];
							--thing->num;
						}
						if (0==thing->num) {
							thing->what=0;
							break;
						}
					}
				}
			else for (i=3; i<for_len+3; ++i)
				if (0==drop_into->arr[i]) {
					drop_into->arr[i]=thing->what;
					drop_into->arr[i+for_len]=thing->num;
					thing->what=0;
					thing->num =0;
					break;
				}
			if (thing->what==0) return 11; //"You drop something"
			else return 12; //"No place to throw it"
		} else return 12; //"No place to throw it"
	} else return 14; //"Nothing to throw away"
}

void check_craft(workbench)
short *workbench; {
	tolog("check_craft started\n");
	void  notify();
	short full_list[COMPONENTS_NUMBER+1], i, sum=0,
	      *product_id, *product_num;
	for (i=1; i<=COMPONENTS_NUMBER; ++i) full_list[i]=0;
	if (VIEW_WORKBENCH==view) {
		product_id =&workbench[12];
		product_num=&workbench[22];
	      	for (i=3; i<12; ++i) if (workbench[i]<=COMPONENTS_NUMBER) {
			full_list[workbench[i]]+=workbench[i+10];
			sum+=workbench[i+10];
		} else return;
	} else if (craft[1].what<=COMPONENTS_NUMBER && craft[2].what<=COMPONENTS_NUMBER) {
		product_id =&craft[0].what;
		product_num=&craft[0].num;
		full_list[craft[1].what] =craft[1].num;
		full_list[craft[2].what]+=craft[2].num;
		sum+=craft[1].num+craft[2].num;
	} else return;
	//recipes
	if (4==full_list[IRON_INGOT] && 4==sum && VIEW_WORKBENCH==view) {
		*product_id=IRON_BOOTS;
		*product_num=9;
	} else if (8==full_list[IRON_INGOT] && 8==sum && VIEW_WORKBENCH==view) {
		*product_id=IRON_CHESTPLATE;
		*product_num=9;
	} else if (5==full_list[IRON_INGOT] && 5==sum && VIEW_WORKBENCH==view) {
		*product_id=IRON_HELMET;
		*product_num=9;
	} else if (7==full_list[IRON_INGOT] && 7==sum && VIEW_WORKBENCH==view) {
		*product_id=IRON_GREAVES;
		*product_num=9;
	} else *product_id=*product_num=0;
	tolog("check_craft finished\n");
}
