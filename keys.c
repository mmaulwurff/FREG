/*Eyecube, sandbox game.
* Copyright (C) 2011 Alexander Kromm, see README file for details.
*/

#include "header.h"
#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>

extern short xp, yp, zp,
             jump, eye[], eyes[], pl,
             earth[][3*WIDTH][HEAVEN+1],
             radar_dist;
extern short view, view_last;
extern struct item inv[][3],
                   cloth[];
extern struct something *heapstart;

struct item *craft;
void  tolog();
short cur[]={1, 0, 0, 0, 0}; /*0.field (0.chest, 1.player, 2.function),
                                1.x, 2.y, 3.id, 4.number*/
short last_view2,
      notflag=1, //flag used for optimization, for not notifying when nothing happens
      *opened;

//menu keys
void key_to_menu(key)
int key; {
	tolog("key_to_menu start\n");
	void notify();
	switch (key) {
		case 27: case 'm':
			view=last_view2;
			notify("Game is resumed.", 0);
		break;
		default: break;
	}
	tolog("key_to_menu finish\n");
}

//inventory keys
void keytoinv(key)
int key; {
	tolog("keytoinv start: ");
	fprintf(stderr, "%c - %d\n", key, key);
	void map(), notify();
	short pocketflag=0,
	      notc=0;
	switch (key) {
		case 'm':
			last_view2=view;
			view=VIEW_MENU;
			notify("Pause", 0);
		break;
		case 'u': case 'f': case 'h': case 'k': case 'r': case 27: { //views;
			fprintf(stderr, "views\n");
			int drop_tning();
			if (VIEW_CHEST==view) { //close chest
				void  focus();
				short x, y, z;
				focus(&x, &y, &z);
				if (PILE==earth[x][y][z]) { //erase if empty
					short i, flag_erase=1;
					struct something *findanimal(),
					                 *heap=findanimal(x, y, z);
					for (i=3; i<33; ++i) if (0!=heap->arr[i]) {
						flag_erase=0;
						break;
					}
					if (flag_erase) {
						struct something *erase_by_xyz();
						earth[x][y][z]=0;
						heapstart=erase_by_xyz(x, y, z,
							heapstart);
					}
				}
			}
			if (VIEW_WORKBENCH!=view) free(craft);
			notc=drop_thing(&cur[3], &cur[4]);
			switch (key) {
				case 'u': view=VIEW_SURFACE; break;
				case 'f': view=VIEW_FLOOR;   break;
				case 'h': view=VIEW_HEAD;    break;
				case 'k': view=VIEW_SKY;     break;
				case 'r': view=VIEW_FRONT;   break;
				default : view=view_last;    break;
			}
			pocketflag=1;
			fprintf(stderr, "27 finished\n");
		} break;
		case KEY_LEFT:
			switch (cur[0]) {
				 //chest
				case 1: cur[1]=(cur[1]==0) ? 9 : cur[1]-1; break;
				case 3: //workbench
					if (cur[1]==0) {
						cur[1]=3;
						cur[2]=0;
					} else if (cur[1]==3) {
						cur[1]=(VIEW_WORKBENCH==view) ? 2 : 0;
						cur[2]=1;
					} else --cur[1];
				break;
			}
		break;
		case KEY_RIGHT:
			switch (cur[0]) {
				//chest
				case 1: cur[1]=(cur[1]==9) ? 0 : cur[1]+1; break;
				case 3: //workbench
					if (cur[1]==((VIEW_WORKBENCH==view) ? 2 : 0)) {
						cur[1]=3;
						cur[2]=0;
					} else if (cur[1]==3) {
						cur[1]=0;
						cur[2]=1;
					} else ++cur[1];
				break;
			}
		break;
		case KEY_UP:
			switch (cur[0]) {
				//chest
				case 1: cur[2]=(cur[2]==0) ?
					((VIEW_CHEST==view) ? 5 : 2) : cur[2]-1;
				break;
				//player
				case 2: cur[2]=(cur[2]==0) ? 3 : cur[2]-1; break;
				case 3: //workbench
					if (cur[1]!=3) cur[2]=(cur[2]==0) ?
						((VIEW_WORKBENCH==view) ? 2 : 1) :
							cur[2]-1;
				break;
			}
		break;
		case KEY_DOWN:
			switch (cur[0]) {
				//chest
				case 1: cur[2]=(cur[2]==((VIEW_CHEST==view) ? 5 : 2)) ?
					0 : cur[2]+1;
				break;
				//player
				case 2: cur[2]=(cur[2]==3) ? 0 : cur[2]+1; break;
				case 3: //workbench
					if (cur[1]!=3) cur[2]=
						(cur[2]==((VIEW_WORKBENCH==view) ?
							2 : 1)) ? 0 : cur[2]+1;
				break;
			}
		break;
		case '\t':
			cur[1]=cur[2]=0;
			cur[0]=(cur[0]==3) ? 0 : cur[0]+1;
			//all function inventory types should be here
			if (!(VIEW_FURNACE==view) && cur[0]==0) cur[0]=1;
		break;
		case 'x': {//put or add one element
			void  know_marked();
			int   stackable();
			short *markedwhat, *markednum;
			know_marked(&markedwhat, &markednum);
			if (stackable(*markedwhat) && stackable(cur[3])) {
				if (cur[3]==*markedwhat && *markednum!=9 && 0!=cur[3]) {
					--cur[4]; //add
					++*markednum;
					if (cur[4]==0) cur[3]=0;
				} else if (0==cur[3] && 0!=*markedwhat) { //get
					cur[3]=*markedwhat;
					cur[4]=1;
					if (--*markednum==0) *markedwhat=0;
				} else if (0==*markedwhat && 0!=cur[3]) {//put
					*markedwhat=cur[3];
					*markednum=1;
					if (--cur[4]==0) cur[3]=0;
				}
				if (3==cur[0]) check_craft(opened);
				break;
			}
		} //no break
		case ' ': case '\n': {
			void  know_marked();
			int   stackable(), armour();
			short *markedwhat, *markednum;
			know_marked(&markedwhat, &markednum);
			if (VIEW_WORKBENCH==view && markedwhat==&(opened[12])) {
				if (0==opened[12]) break;
				else {
					short i;
					for (i= 3; i<12; ++i) opened[i]=0;
					for (i=13; i<22; ++i) opened[i]=0;
				}
			} else if (VIEW_WORKBENCH!=view &&
					markedwhat==&(craft[0].what))
				if (0==craft[0].what) break;
				else {
					craft[1].what=0;
					craft[1].num =0;
					craft[2].what=0;
					craft[2].num =0;
				}
//			fprintf(stderr, "marked is %d\n", *markedwhat);
//			fprintf(stderr, "cur[2] is %d\n", cur[2]);
			if (cur[3]==*markedwhat && stackable(*markedwhat)) { //add
				while (*markednum!=9 && cur[4]!=0) {
					--cur[4];
					++*markednum;
				}
				if (cur[4]==0) cur[3]=0;				
			} else if (cur[0]!=2 || 0==cur[3] || (cur[0]==2 && (
					(cur[2]==0 && armour(cur[3])=='h') ||
					(cur[2]==1 && armour(cur[3])=='a') ||
					(cur[2]==2 && armour(cur[3])=='l') ||
					(cur[2]==3 && armour(cur[3])=='b'))) &&
					!(3==cur[0] && 0==*markedwhat)) {
				//put/change/get
				short save=*markedwhat;
				*markedwhat=cur[3];
				cur[3]=save;
				save=*markednum;
				*markednum=cur[4];
				cur[4]=save;
			}
			if (3==cur[0]) check_craft(opened);
		} break;
		case '!': radar_dist=(NEAR==radar_dist) ? FAR : NEAR; break;
		default : break;
	}
	if (VIEW_MENU!=view) map();
	if (pocketflag) {
		void pocketshow();
		pocketshow();
	}
	if (notflag!=notc) {
		switch(notc) {
//			case 8 is reserved
			case 11: notify("You drop something.",          0); break;
			case 12: notify("Nothing to drop",              0); break;
			case  8: //no break
			default: notify("?",                            0); break;
		}
		notflag=notc;
	}
	tolog("keytoinv finish\n");
}

//returns 
void know_marked(markedwhat, markednum)
short **markedwhat, **markednum; {
	tolog("know_marked start\n");
	switch (cur[0]) {
		case 1: //backpack
			if (cur[2]>2) {
				*markedwhat=&(opened[cur[1]+(cur[2]-3)*10+3]);
				*markednum =&(opened[cur[1]+(cur[2]-3)*10+33]);
			} else {
				*markedwhat=&inv[cur[1]][cur[2]].what;
				*markednum =&inv[cur[1]][cur[2]].num;
			}
		break;
		case 2: //player
			*markedwhat=&cloth[cur[2]].what;
			*markednum =&cloth[cur[2]].num;
		break;
		case 3: {//workbench
			if (VIEW_WORKBENCH==view) {
				short i=(cur[1]!=3) ? 3+cur[2]+cur[1]*3 : 12;
				*markedwhat=&(opened[i   ]);
				*markednum= &(opened[i+10]);
			} else if (3==cur[1]) {
				*markedwhat=&(craft[0].what);
				*markednum =&(craft[0].num);
			} else if (0==cur[2]) {
				*markedwhat=&(craft[1].what);
				*markednum =&(craft[1].num);
			} else {
				*markedwhat=&(craft[2].what);
				*markednum =&(craft[2].num);
			}
		} break;
		default: break;
	}
	tolog("know_marked finish\n");
}

//this is game physics and interface
void keytogame(key)
int key; {
	tolog("keytogame start: ");
	fprintf(stderr, "%c - %d\n", key, key);
	void  pocketshow(), sounds_print(), notify();
	int   step(),
	      passable();
	short notc=0,
	      save,
	      mapflag=1, moveflag=0;
	switch(key) {
		//player movement
		//TODO: read keys from file
		//these are optimized for Dvorak programmer layout
		case 'm':
			last_view2=view;
			view=VIEW_MENU;
			notify("Pause", 0);
			mapflag=0;
		break;
		case KEY_LEFT:
			if ((notc=step(-eye[1]*(abs(eye[0])-1),
			                eye[0]*(abs(eye[1])-1))==1) || notc==5)
				mapflag=0;
			else moveflag=1;
		break;
		case KEY_RIGHT:
			if ((notc=step( eye[1]*(abs(eye[0])-1),
			               -eye[0]*(abs(eye[1])-1))==1) || notc==5)
				mapflag=0;
			else moveflag=1;
		break;
		case KEY_UP:
			if ((notc=step( eye[0],  eye[1])==1) || notc==5)
				mapflag=0;
			else moveflag=1;
	       	break;
		case KEY_DOWN:
			if ((notc=step(-eye[0], -eye[1])==1) || notc==5)
				mapflag=0;
			else moveflag=1;
		break;
		case ' '://one ' ' - jump forward and up
			 //two ' ' - jump forward two blocks
			jump=(jump==1) ? 2 : 1;
			notc=2;
			mapflag=0;
		break;
		//camera position
		case ',': //returns previous camera position
			//'w' for qwerty
			save=eye[0];
			eye[0]=eyes[0];
			eyes[0]=save;
			save=eye[1];
			eye[1]=eyes[1];
			eyes[1]=save;
		break;
		case 'e': //turn to right
			//'d' for qwerty
			save=eyes[0]=eye[0];
			eyes[1]=eye[1];
			//this mathematics does the turns
			eye[0]=eye[1]*(abs(eye[0])-1);
			eye[1]= -save*(abs(eye[1])-1);
		break;
		case 'o'://turn back
			//'s' for qwerty
			eyes[0]=eye[0];
			eyes[1]=eye[1];
			eye[0]=-eye[0];
			eye[1]=-eye[1];
		break;
		case 'a': //turn to left
			save=eyes[0]=eye[0];
			eyes[1]=eye[1];
			eye[0]=-eye[1]*(abs(eye[0])-1);
			eye[1]=   save*(abs(eye[1])-1);
		break;
		case 'v': pl=(pl) ? 0 : 1; break; //toggle player visibility on map
		case 'S': savegame(); notc=6; mapflag=0; break;
		case 'L': {
			void loadgame();
			loadgame();
			notc=7;
		} break;
		case 'i':
			view_last=view;
			view=VIEW_INVENTORY;
			craft=malloc(3*sizeof(struct item));
			for (save=0; save<3; ++save) craft[save].what=craft[save].num=0;
			if (cur[0]==1 && cur[2]>2) cur[2]=0;
		break;
		case 'u': view_last=view; view=VIEW_SURFACE; break;
		case 'f': view_last=view; view=VIEW_FLOOR;   break;
		case 'h': view_last=view; view=VIEW_HEAD;    break;
		case 'k': view_last=view; view=VIEW_SKY;     break;
		case 'r': view_last=view; view=VIEW_FRONT;   break;
		case 'p': //return to previous view
			save=view;
			view=view_last;
			view_last=save;
		break;	
		case '?': { //ask "what's this?"
			void focus();
			short wx, wy, wz;
			focus(&wx, &wy, &wz);
			notc=30+earth[wx][wy][wz];
			mapflag=0;
		} break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': //choose weapon
			cloth[4].num=key-'0';
			pocketshow();
		break;
		case '+': //next weapon
			cloth[4].num=(cloth[4].num==9) ? 0 : cloth[4].num+1;
			notc=36;
			pocketshow();
		break;
		case '-': //previous weapon
			cloth[4].num=(cloth[4].num==0) ? 9 : cloth[4].num-1;
			notc=36;
			pocketshow();
		break;
		case '\n': { //use
			void  focus();
			short x, y, z;
			focus(&x, &y, &z);
			if (!passable(earth[x][y][z-1])) { //thing isn't falling
				switch (earth[x][y][z]) {
					case CHESTLIKE: {
						short *open_chest();
						opened=open_chest();
						craft=malloc(3*sizeof(struct item));
						for (save=0; save<3; ++save)
							craft[save].what=
								craft[save].num=0;
						view_last=view;
						view=VIEW_CHEST;
					} break;
					case WORKBENCH: {
						short *open_chest();
						opened=open_chest();
						view_last=view;
						view=VIEW_WORKBENCH;
					} break;
					default : notc=10; break;
				}
			}
		} break;
		case 'x': { //drop weapon
			int  drop_thing();
			void pocketshow();
			notc=drop_thing(&inv[cloth[4].num][2]);
			pocketshow();
		} break;
		case '!': radar_dist=(NEAR==radar_dist) ? FAR : NEAR; break;
		default  : notc=8; mapflag=0; break;
	}
	//falling down
	if (moveflag) {
		for (save=0; passable(earth[xp][yp][zp-1]); ++save, --zp);
	       	if (save>1) {
			notc=4;
			//damage should be here
		}
	}
	if (notflag!=notc) {
		switch (notc) { //max number of chars in line: 30
			//               |                            |
			case  0: notify("Nothing special happened.",    0); break;
			case  1: notify("You can't move this way.",     0); break;
			case  2: notify("You're ready to jump.",        0); break;
			case  3: notify("You can't jump.",              0); break;
			case  4: notify("You fall down.",               0); break;
			case  5: notify("Something is over your head.", 0); break;
			case  6: notify("Game is saved.",               0); break;
			case  7: notify("Game is loaded.",              0); break;
			//case 8 is reserved for ?
			case  9: notify("Something unknown!",           0); break;
			case 10: notify("You can't use this.",          0); break;
			case 11: notify("You drop something.",          0); break;
			case 12: notify("No place to throw it.",        0); break;
			case 13: notify("You move a block.",            0); break;
			case 14: notify("Nothing to throw away.",       0); break;
			case 31: notify("Grass or leaves",              0); break;
			case 32: notify("Stone",                        0); break;
			case 33: notify("It is somebody!",              0); break;
			case 34: notify("Chiken",                       0); break;
			case 35: notify("Careful! Fire",                0); break;
			case 36: notify("Weapon",            cloth[4].num); break;
			case  8: //no break
			default: notify("?",                            0); break;
		}
		notflag=notc;
	}
	if (mapflag) {
		void map();
		map();
		sounds_print();
	}
	tolog("keytogame finish\n");
}
