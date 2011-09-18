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
#include <ncurses.h>
#include <stdlib.h>

extern short xp, yp, zp,
             jump,
             eye[], eyes[],
	     pl,
	     earth[][192][HEAVEN+1];
extern char  view, view_last;
extern struct item inv[][3],
                   cloth[];

short cur[]={1, 0, 0, 0, 0};
short notflag=1; //flag used for optimization, for not notifying when nothing happens
struct item *craft;

void keytoinv(key)
int key; {
	void map();
	short pocketflag=0;
	switch (key) {
		case 'u': case 'f': case 'h': case 'k': case 'r': case 27: //views;
			free(craft);
			view=(27==key) ? view_last : key;
			pocketflag=1;
		break;
		case KEY_LEFT:
			switch (cur[0]) {
				 //chest
				case 1: cur[1]=(cur[1]==0) ? 9 : cur[1]-1; break;
				case 3: //workbench
					if (cur[1]==0) {
						cur[1]=3;
						cur[2]=0;
					} else if (cur[1]==3) {
						cur[1]=(view=='w') ? 2 : 1;
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
					if (cur[1]==((view=='w') ? 2 : 1)) {
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
				case 1: cur[2]=(cur[2]==0) ? ((view=='c') ? 5 : 2) :
					cur[2]-1;
				break;
				//player
				case 2: cur[2]=(cur[2]==0) ? 3 : cur[2]-1; break;
				case 3: //workbench
					if (cur[1]!=3) cur[2]=(cur[2]==0) ?
						((view=='w') ? 2 : 1) : cur[2]-1;
				break;
			}
		break;
		case KEY_DOWN:
			switch (cur[0]) {
				//chest
				case 1: cur[2]=(cur[2]==((view=='c') ? 5 : 2)) ?
					0 : cur[2]+1;
				break;
				//player
				case 2: cur[2]=(cur[2]==3) ? 0 : cur[2]+1; break;
				case 3: //workbench
					if (cur[1]!=3)
						cur[2]=(cur[2]==((view=='w') ? 2 : 1)) ?
							0 : cur[2]+1;
				break;
			}
		break;
		case '\t':
			cur[1]=cur[2]=0;
			cur[0]=(cur[0]==3) ? 0 : cur[0]+1;
			//all function inventory types should be here
			if (!(view=='n') && cur[0]==0) cur[0]=1;
		break;
		case '\n': {
			short *markedwhat,
			      *markednum;
			switch (cur[0]) {
				case 1: //backpack
					if (cur[2]>2) {
						struct something *open_chest(),
							         *chest=open_chest();
						markedwhat=&(chest->arr
							[cur[1]+(cur[2]-3)*10+3]);
						markednum =&(chest->arr
							[cur[1]+(cur[2]-3)*10+33]);
					} else {
						markedwhat=&inv[cur[1]][cur[2]].what;
						markednum =&inv[cur[1]][cur[2]].num;
					}
				break;
				case 2: //player
					markedwhat=&cloth[cur[2]].what;
					markednum =&cloth[cur[2]].num;
				break;
				case 3: {//workbench
					short i=(cur[1]!=3) ? 1+cur[2]+cur[1]*2 : 0;
					markedwhat=&craft[i].what;
					markednum= &craft[i].num;
				} break;
			}
//			fprintf(stderr, "marked is %d\n", *markedwhat);
//			fprintf(stderr, "cur[2] is %d\n", cur[2]);
			if (cur[3]==0) { //get
				cur[3]=*markedwhat;
				*markedwhat=0;
				cur[4]=*markednum;
				*markednum=0;
			} else if (cur[3]==*markedwhat && cur[0]!=2 &&
					 property(*markedwhat, 's') &&
					!property(*markedwhat, 'a')) { //add
				for ( ; *markednum!=9 && cur[4]!=0; --cur[4])
					++*markednum;
				if (cur[4]==0) cur[3]=0;
			} else if (cur[0]!=2 || (cur[0]==2 && cur[2]==0 &&
					property(cur[3], 'a')=='h'))
/*				(cur[2]==0 && property(*markedwhat, 'a')=='h') ||
				(cur[2]==1 && property(*markedwhat, 'a')=='a') ||
				(cur[2]==2 && property(*markedwhat, 'a')=='l') ||
				(cur[2]==3 && property(*markedwhat, 'a')=='b'))*/ {
				//change (put)
				short save=*markedwhat;
				*markedwhat=cur[3];
				cur[3]=save;
				save=*markednum;
				*markednum=cur[4];
				cur[4]=save;
			}
		} break;
	}
	map();
	if (pocketflag) {
		void pocketshow();
		pocketshow();
	}
}

//this is game physics and interface
void keytogame(key)
int key; {
	void  pocketshow();
	int   step(),
	      property();
	short notc=0,
	      save,
	      mapflag=1;
	switch(key) {
		//player movement
		//TODO: read keys from file
		//these are optimized for Dvorak programmer layout
		case KEY_LEFT:
			if ((notc=step(-eye[1]*(abs(eye[0])-1),
			                eye[0]*(abs(eye[1])-1))==1) || notc==5)
				mapflag=0;
		break;
		case KEY_RIGHT:
			if ((notc=step( eye[1]*(abs(eye[0])-1),
			               -eye[0]*(abs(eye[1])-1))==1) || notc==5)
				mapflag=0;
		break;
		case KEY_UP:
			if ((notc=step( eye[0],  eye[1])==1) || notc==5)
				mapflag=0;
	       	break;
		case KEY_DOWN:
			if ((notc=step(-eye[0], -eye[1])==1) || notc==5)
				mapflag=0;
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
			craft=malloc(5*sizeof(struct item));
			for (save=0; save<5; ++save) craft[save].what=craft[save].num=0;
			//right, no break here
		case 'u': case 'f': case 'h': case 'k': case 'r':
			view_last=view;
			view=key;
		break;
		case 'p':
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
		case '5': case '6': case '7': case '8': case '9':
			cloth[4].num=key-'0';
			pocketshow();
		break;
		case '+': 
			cloth[4].num=(cloth[4].num==9) ? 0 : cloth[4].num+1;
			pocketshow();
		break;
		case '-':
			cloth[4].num=(cloth[4].num==0) ? 9 : cloth[4].num-1;
			pocketshow();
		break;
		case '\n': { //use
			void  focus();
			short x, y, z;
			focus(&x, &y, &z);
			switch (property(earth[x][y][z], 'n')) {
				case 'c':
					craft=malloc(5*sizeof(struct item));
					for (save=0; save<5; ++save)
						craft[save].what=craft[save].num=0;
					view='c';
				break;
				default : notc=10; break;
			}
		} break;			   
		default  : notc=8; mapflag=0; break;
	}
	//falling down
	for (save=0; property(earth[xp][yp][zp-1], 'p'); ++save, --zp);
       	if (save>1) {
		notc=4;
		//damage should be here
	}
	if (mapflag) {
		void map();
		map();
	}
	if (notflag!=notc) {
		switch (notc) {
			case  0: notify("Nothing special happened.",    0); break;
			case  1: notify("This is the wall.",            0); break;
			case  2: notify("Ready to jump.",               0); break;
			case  3: notify("Can't jump.",                  0); break;
			case  4: notify("You fell down.",               0); break;
			case  5: notify("Something is over your head.", 0); break;
			case  6: notify("Game saved.",                  0); break;
			case  7: notify("Game loaded.",                 0); break;
			case  9: notify("Something unknown!",           0); break;
			case 10: notify("Can't use",                    0); break;
			case 30: notify("Nothing except air",           0); break;
			case 31: notify("Grass or leaves",              0); break;
			case 32: notify("Stone",                        0); break;
			case 33: notify("It is somebody!",              0); break;
			case 34: notify("Chiken",                       0); break;
			case 35: notify("Careful! Fire",                0); break;
			case  8:
			default: notify("?",                            0); break;
		}
		notflag=notc;
	}
}

