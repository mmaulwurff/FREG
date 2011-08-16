#include "header.h"

extern struct item {
	short what,
	      num;
} inv[][3];
extern short view,
             cloth[];
extern WINDOW *world,
              *pocketwin;
extern short *craft;

void map(),
     pocketshow();

short cur[]={1, 0, 0, 0, 0};

void mark(x, y, wind, c)
short  x, y;
WINDOW *wind;
char   c; {
	wattrset(wind, COLOR_PAIR(8));
	if (c=='e') { //empty
		(void)mvwprintw(wind, y, x-1, ">");
		(void)mvwprintw(wind, y, x+2, "<");
	} else { //full
		(void)mvwprintw(wind, y, x-1, "!");
		(void)mvwprintw(wind, y, x+2, "!");
	}
}

void keytoinv(key)
int key; {
	short pocketflag=0;
	switch (key) {
		case 'u': case 'f': case 'h': case 'k':
		case 'r': view=key; pocketflag=1; free(craft); break; //front view
		case KEY_LEFT:
			//chest
			if (cur[0]==1) cur[1]=(cur[1]==0) ? 9 : cur[1]-1;
		break;
		case KEY_RIGHT:
			//chest
			if (cur[0]==1) cur[1]=(cur[1]==9) ? 0 : cur[1]+1;
		break;
		case KEY_UP:
			//chest
			if      (cur[0]==1) cur[2]=(cur[2]==0) ? 2 : cur[2]-1;
			//player
			else if (cur[0]==2) cur[2]=(cur[2]==0) ? 3 : cur[2]-1;
		break;
		case KEY_DOWN:
			//chest
			if      (cur[0]==1) cur[2]=(cur[2]==2) ? 0 : cur[2]+1;
			//player
			else if (cur[0]==2) cur[2]=(cur[2]==3) ? 0 : cur[2]+1;
		break;
		case '\t':
			cur[1]=cur[2]=0;
			cur[0]=(cur[0]==3) ? 0 : cur[0]+1;
			//all function inventory types should be here
			if (!(view=='t' || view=='n') && cur[0]==0) cur[0]=1;
		break;
		case '\n': {
			short *markedwhat, *markednum;
			if (cur[0]==1) { //backpack
				markedwhat=&inv[cur[1]][cur[2]].what;
				markednum =&inv[cur[1]][cur[2]].num;
			}
			if (cur[3]==0) { //get
				cur[3]=*markedwhat;
				       *markedwhat=0;
				cur[4]=*markednum;
				       *markednum=0;
			} else if (cur[3]==*markedwhat) { //add
				while (*markednum!=9 &&	cur[4]!=0) {
					++*markednum;
					--cur[4];
				}
				if (cur[4]==0) cur[3]=0;
			} else { //change
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
	if (pocketflag) pocketshow();
}

void invview() {
	short i, j;
	//left arm
	wattrset(world, COLOR_PAIR(3));
	(void)mvwprintw(world, 4, 5, "U");
	//right arm
	if (inv[cloth[4]][2].what) (void)mvwprintw(world, 4, 1, "%c%d",
			getname(cloth[4], 2, HEAVEN+3), inv[cloth[4]][2].num);
	else (void)mvwprintw(world, 4, 2, "U");
	//shoulders
	if (cloth[1]) (void)getname(1, 0, HEAVEN+2);
	else wattrset(world, COLOR_PAIR(1));
	(void)mvwprintw(world, 3, 2, "    ");
	for (i=0; i<=3; ++i)
		if (cloth[i]) {
			char name=getname(i, 0, HEAVEN+2);
			(void)mvwprintw(world, 2+i, 3, "%c%c", name, name);
		} else switch (i) {
			case 0: //head
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 2, 3, "''");
			break;
			case 1:	case 2: //body & legs
				wattrset(world, COLOR_PAIR(1));
				(void)mvwprintw(world, 2+i, 3, "  ");
			break;
			case 3: //feet
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 5, 3, "db");
			break;
		}
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+19+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(j, i, HEAVEN+3), inv[j][i].num);
/*	switch (view) {
		default: //5 - normal
			for (i=0; i<=1)
	}*/
	switch(cur[0]) {
		//chest
		case 1: {
			short temp1=cur[2]+19+((cur[2]==2) ? 1 : 0),
			      temp2=cur[1]*3+7;
			if (cur[3]!=0) {
				(void)mvwprintw(world, temp1, temp2, "%c%d",
					getname(0, 0, HEAVEN+4), cur[4]);
				mark(temp2, temp1, world, 'f');
			} else	mark(temp2, temp1, world, 'e');
		} break;
		//player
		case 2: mark(3, cur[2]+2, world); break;
	}
	(void)wclear(pocketwin);
	(void)wrefresh(pocketwin);
}
