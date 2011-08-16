#include "header.h"

//TODO: make this fields
//player (lower half) coordinates
short xp, yp, zp;
//player square position
short spx, spy;
//shows if player jumps
short jump;
//camera position
short eye[2];
//save previous eyes for 'w' key
short eyes[2];
/* view mode:
 * sUrface
 * Floor
 * Head
 * sKy
 * fRont
 * Inventory
 * Chest
 * crafTtable
 * furNace
*/
char view;
//show player or no
short pl;
//9 loaded squares
short earth[192][192][HEAVEN+1];
//heaven
short sky[39][39];
WINDOW *world,
       *textwin,
       *pocketwin;
//pointer for crafttable
short *craft;
short mechtoggle;

struct item {
	short what,
	      num;
} inv[10][3];

short cloth[5];

extern short cur[];
//flag used for optimization, for not notifying when nothing happens
short notflag=1;

//functions from other files
//maps.c
void loadgame(),
     savegame(),
     onbound(),
//mech.c
     allmech(),
//inv.c
     mark(),
     keytoinv(),
     invview();
//this file
void *mech(),
     map(),
     frontview(),
     surf(),
     notify(),
     pocketshow(),
     start(),
     keytogame(),
     focus();
int  getname(),
     visible2(),
     visible(),
     visin(),
     fall(),
     step(),
     property();

void main() {
	int ch;
	pthread_t mechthread;
	//start parallel thread
	(void)pthread_create(&mechthread, NULL, mech, NULL);
	(void)initscr();
	(void)start_color();
	(void)cbreak();
	(void)noecho();
	(void)keypad(stdscr, TRUE);
	(void)init_pair(1, COLOR_WHITE,  COLOR_BLUE  );  //player, sky
	(void)init_pair(2, COLOR_BLACK,  COLOR_GREEN );  //grass, dwarf
	(void)init_pair(3, COLOR_BLACK,  COLOR_WHITE );  //stone, skin
	(void)init_pair(4, COLOR_RED,    COLOR_YELLOW);  //sun, fire1
	(void)init_pair(5, COLOR_RED,    COLOR_WHITE );  //chiken
	(void)init_pair(6, COLOR_WHITE,  COLOR_BLACK );  //?
	(void)init_pair(7, COLOR_YELLOW, COLOR_RED   );  //fire2
	(void)init_pair(8, COLOR_BLACK,  COLOR_RED   );  //pointer
	world=    newwin(24, 44, 0,  0);
	pocketwin=newwin(1,  44, 24, 0);
	textwin=  newwin(6,  44, 25, 0);
	(void)refresh();
	start();
	map();
	notify("Game started.", 0);
	//this is the game itself
	while ((ch=getch())!=(int)'q')
		if (view=='u' || view=='f' || view=='h' || view=='k' || view=='r')
			keytogame(ch);
		else keytoinv(ch);
	//stop parallel thread
	(void)pthread_cancel(mechthread);
	(void)delwin(world    );
	(void)delwin(textwin  );
	(void)delwin(pocketwin);
	(void)endwin();
}

//parallel thread
void *mech(void *vptr_args) {
	while (1) {
		sleep(1);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		//all mech and animal functions
		allmech();
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

// this prints visible world
void map() {
	(void)wclear(world);
	if (view=='u' || view=='f' || view=='h' || view=='k') surf();
	else if (view=='r') frontview();
	else invview();
	wstandend(world);
	switch (view) {
		case  'u': (void)mvwprintw(world, 22, 1, "Surface"  ); break;
		case  'f': (void)mvwprintw(world, 22, 1, "Floor"    ); break;
		case  'h': (void)mvwprintw(world, 22, 1, "Head"     ); break;
		case  'k': (void)mvwprintw(world, 22, 1, "Sky"      ); break;
		case  'r': (void)mvwprintw(world, 22, 1, "Front               ^^"); break;
		case  'i': (void)mvwprintw(world,  1, 1, "Inventory"); break;
		default  : (void)mvwprintw(world, 22, 1, "Another"  ); break;
	}
	if (!pl) (void)mvwprintw(world, 22, 21, "^^");
	(void)mvwprintw(world, 0, 0, "");
	(void)box(world, 0, 0);
	(void)wrefresh(world);
}

void frontview() {
	short x, y, zend, flag,
	      xplus, zplus,
	      xsave, zsave;
	if (eye[0]==0) {
		if (eye[1]==-1) {//north
			xsave=xp-11;
			xplus=1;
			zsave=yp-1;
			zend=yp-21;
			zplus=-1;
		} else {//south
			xsave=xp+11;
			xplus=-1;
			zsave=yp+1;
			zend=yp+21;
			zplus=1;
		}
		flag=1;
	} else if (eye[0]==-1) {//west
		xsave=yp+11;
		xplus=-1;
		zsave=xp-1;
		zend=xp-21;
		zplus=-1;
		flag=0;
	} else {//east
		xsave=yp-11;
		xplus=1;
		zsave=xp+1;
		zend=xp+21;
		zplus=1;
		flag=0;
	}	
	for (x=1; x<=21; ++x)
	for (y=1; y<=21; ++y) {
		short save, p, z,
		      xnew,ynew=zp+20-y;
		if (flag) {
			xnew=xsave+x*xplus;
			z=zsave;
			while (z!=zend && earth[xnew][z][ynew]==0) z+=zplus;
			save=z;
			p=yp;
		} else {
			z=xsave+x*xplus;
			xnew=zsave;
			while (xnew!=zend && earth[xnew][z][ynew]==0) xnew+=zplus;
			save=xnew;
			p=xp;
		} 
		if (visible2(xp, yp, zp+1, xnew, z, ynew) || visible(xnew, z, ynew)) {
			if (save!=zend) {
				//TODO: this can be without 'names'
				char name=getname(xnew, z, ynew), name2;
				if (abs(save-p)==1)      name2=name;
				else if (abs(save-p)<11) name2=abs(save-p)-1+'0';
				else                     name2='+';
				(void)mvwprintw(world, y, 2*x-1, "%c%c", name, name2);
			} else if (earth[xnew][z][ynew]!=0) {
					wattrset(world, COLOR_PAIR(6));
					(void)mvwprintw(world, y, 2*x-1, "??");
			} else {
				wattrset(world, COLOR_PAIR(1));
				(void)mvwprintw(world, y, 2*x-1, ". ");
			}
		}
	}
}

//surface view
void surf() {
	short x, y, z,
	      xcor,  ycor,
	      xarr,  yarr,
	      xrarr, yrarr,
	      skycor=(view==3) ? (-1) : 1;
	if (eye[0]==0) {
		if (eye[1]==-1) {//north
			xcor=xp-11;
			ycor=yp-20*skycor;
			xarr=1;
			yarr=skycor;
			xrarr=yrarr=0;
		} else {//south
			xcor=xp+11;
			ycor=yp+20*skycor;
			xarr=-1;
			yarr=-skycor;
			xrarr=yrarr=0;
		}
	} else if (eye[0]==-1) {//west
		xcor=xp-20*skycor;
		ycor=yp+11;
		xarr=yarr=0;
		xrarr=skycor;
		yrarr=-1;
	} else {//east
		xcor=xp+20*skycor;
		ycor=yp-11;
		xarr=yarr=0;
		xrarr=-skycor;
		yrarr=1;
	}
	for (y=1; y<=21; ++y)
	for (x=1; x<=21; ++x) {
		short st, en, plus,
		      newx=xcor+xarr*x+xrarr*y,
		      newy=ycor+yarr*y+yrarr*x;
		switch (view) {
			case  'f': st=zp;     en=0;      plus=-1; break; //floor
			case  'h': st=zp+1;   en=0;      plus=-1; break; //head
			case  'k': st=zp+2;   en=HEAVEN; plus= 1; break; //sky
			default  : st=HEAVEN; en=0;      plus=-1; break; //surface 
		}
		for (z=st; z!=en && earth[newx][newy][z]==0; z+=plus);
		if (z==HEAVEN) {
			if (visible2(xp, yp, zp, newx, newy, z)) {
				switch (sky[newx-xp+20][newy-yp+20]) {
					case  1: wattrset(world, COLOR_PAIR(3)); break;
					case  2: wattrset(world, COLOR_PAIR(4)); break;
					default: wattrset(world, COLOR_PAIR(1)); break;
				}
				(void)mvwprintw(world, y, 2*x-1, "  ");
			}
		} else if (visible2(xp, yp, zp+1, newx, newy, z) ||
			visible(newx, newy, z))
				if (z-zp>=10) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(newx, newy, z), '+');
				else if (z-zp>-1) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(newx, newy, z), z-zp+1+'0');
				else if (z-zp<-1) (void)mvwprintw(world, y, 2*x-1,
					"%c%c", getname(newx, newy, z), '-');
				else {
					char name=getname(newx, newy, z);
					(void)mvwprintw(world, y, 2*x-1,
					"%c%c", name, name);
				}
	}
	//show player or not
	if (pl && view!='k') {
		for (z=HEAVEN; z!=zp && earth[xp][yp][z]==0; --z);
		if (z==zp) {
			wattrset(world, COLOR_PAIR(1));
			(void)mvwprintw(world, 20, 21, "  ");
		}
	}
}

int getname(x, y, z)
short x, y, z; {
	WINDOW *pwin;
	short block, sum;
	if (z<HEAVEN+1) { //normal
		block=earth[x][y][z];
		pwin=world;
	} else if (z==HEAVEN+1) { //pockets
		block=inv[x][y].what;
		pwin=pocketwin;
	} else if (z==HEAVEN+2) { //cloth except weapon
		block=cloth[x];
		pwin=world;
	} else if (z==HEAVEN+3) { //inventory
		block=inv[x][y].what;
		pwin=world;
	} else /*if (z==HEAVEN+4)*/ { //cursor
		block=cur[3];
		pwin=world;
	}
	switch (block) {
		case 1: //grass
			wattrset(pwin, COLOR_PAIR(2));
			return('|'); break;
		case 2: //stone
			wattrset(pwin, COLOR_PAIR(3));
			return('s'); break;
		case 3: //reserved for player
			wattrset(pwin, COLOR_PAIR(2));
			return('p'); break;
		case 4: //chiken
			wattrset(pwin, COLOR_PAIR(5));
			return('c'); break;
		case 5: //fire
			sum=(x+y+z)%2;
			if (( mechtoggle &&  sum) ||
			    (!mechtoggle && !sum)) {
				wattrset(pwin, COLOR_PAIR(4));
				return('F');
			} else {
				wattrset(pwin, COLOR_PAIR(7));
				return('f');
			}
		break;
		case 0: if (z>HEAVEN) {
				wstandend(pwin);
				return(' ');
				break;
			}
		default: return('?'); break;
	}
}

//this is the second vibility checker. it works with the first.
int visible2(x1, y1, z1,
             x2, y2, z2)
short x1, y1, z1,
      x2, y2, z2; {
	//TODO: optimize loops
	short i, j, k,
	      imin, jmin, kmin,
	      newmin,
	      min=(x1-x2)*(x1-x2)+
	          (y1-y2)*(y1-y2)+
	          (z1-z2)*(z1-z2);
	for (i=x1-1; i<=x1+1; ++i)
	for (j=y1-1; j<=y1+1; ++j)
	for (k=z1-1; k<=z1+1; ++k)
		if ((newmin=(i-x2)*(i-x2)+
		            (j-y2)*(j-y2)+
			    (k-z2)*(k-z2))<min) {
			imin=i;
			jmin=j;
			kmin=k;
			min=newmin;
		}
	if (min==0) return(1);
	else if (!property(earth[imin][jmin][kmin], 't')) return(0);
	else if (visible2(imin, jmin, kmin, x2, y2, z2)) return(1);
}

//TODO: perfect visibility function (visible2x3)
//this is visibility checker. it is not perfect, but it works
//(with the second one)
int visible(x, y, z)
short x, y, z; {
	short count=0;
	if (z>zp+1) {
		count+=visin(zp+1, xp, yp,   z, x, y, 4);
		count+=visin(zp+1, yp, xp,   z, y, x, 5);
	} else {
		count+=visin(xp,   yp, zp+1, x, y, z, 0);
		count+=visin(yp,   xp, zp+1, y, x, z, 2);
	}
	return((count>1) ? 0 : 1);
}

//this function is inside visible function
int visin(nf, ns, nt, kf, ks, kt, wh)
short nf, ns, nt, kf, ks, kt, wh; {
	short count=0,
	      mff=(nf>kf) ? (-1) : (1),
	      mfs=(ns>ks) ? (-1) : (1),
	      mft=(nt>kt) ? (-1) : (1);
	while (!( nf==kf && ns==ks && nt==kt )) {
		short x, y, z;
		if ((abs(nf-kf)>=abs(ns-ks)) &&
		    (abs(nf-kf)>=abs(nt-kt)))    nf+=mff;
		else if (abs(ns-ks)>=abs(nt-kt)) ns+=mfs;
		else                             nt+=mft;
		switch (wh) {
			case  0: x=nf; y=ns; z=nt; break;
//			case  1: x=nf; z=ns; y=nt; break;
			case  2: y=nf; x=ns; z=nt; break;
//			case  3: y=nf; z=ns; x=nt; break;
			case  4: z=nf; x=ns; y=nt; break;
			//case 5:
			default: z=nf; y=ns; x=nt; break;
		}	
		if (!property(earth[x][y][z], 't') &&
			!( x==xp && y==yp && z<=zp-1 )) ++count;
	}
	return(count-1);
}

//global notify system.
void notify(not, noc)
char not[];
short noc; {
	(void)wclear(textwin);
	wstandend(textwin);
	mvwaddstr(textwin, 1, 1, not);
	if (noc) wprintw(textwin, "%d", noc);
	(void)mvwprintw(textwin, 2, 1, "x: %d", xp);
	(void)mvwprintw(textwin, 3, 1, "y: %d", yp);
	(void)mvwprintw(textwin, 4, 1, "z: %d", zp);
	(void)mvwprintw(textwin, 0, 0, "");
	(void)box(textwin, 0, 0);
	(void)wrefresh(textwin);
}

//this prints pockets contents
void pocketshow() {
	short x;
	(void)wclear(pocketwin);
	for (x=0; x<=9; ++x) (void)mvwprintw(pocketwin, 0, 7+x*3, "%c%d",
		getname(x, 2, HEAVEN+1), inv[x][2].num);
	mark(7+cloth[4]*3, 0, pocketwin, 'e');
	(void)wrefresh(pocketwin);
}

//this was map constructor
void start() {
	mechtoggle=0;
	loadgame();
//	equip.weap=0;
}
	
//this is game physics and interface
void keytogame(key)
int key; {
	short notc=0,
	      save,
	      mapflag=1,
	      wx, wy, wz;
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
		case ','://returns previous camera position
			//'w' for qwerty
			save=eye[0];
			eye[0]=eyes[0];
			eyes[0]=save;
			save=eye[1];
			eye[1]=eyes[1];
			eyes[1]=save;
		break;
		case 'e'://turn to right
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
		case 'a'://turn to left
			save=eyes[0]=eye[0];
			eyes[1]=eye[1];
			eye[0]=-eye[1]*(abs(eye[0])-1);
			eye[1]=   save*(abs(eye[1])-1);
		break;
		case 'v': pl=(pl) ? 0 : 1; break; //toggle player visibility on map
		case 'S': savegame(); notc=6; mapflag=0; break;
		case 'L': loadgame(); notc=7;            break;
		case 'i': craft=malloc(5*sizeof(short));
		case 'u': case 'f': case 'h': case 'k': case 'r': view=key; break;
		case '?':
			focus(&wx, &wy, &wz);
			notc=30+earth[wx][wy][wz];
			mapflag=0;
		break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			cloth[4]=key-'0';
			pocketshow();
		break;
		case '+': 
			cloth[4]=(cloth[4]==9) ? 0 : cloth[4]+1;
			pocketshow();
		break;
		case '-':
			cloth[4]=(cloth[4]==0) ? 9 : cloth[4]-1;
			pocketshow();
		break;
		default : notc=8; mapflag=0; break;
	}
	//falling down
	for (save=0; earth[xp][yp][zp-1]==0; ++save, --zp);
       	if (save>1) {
		notc=4;
		//damage should be here
	}
	if (mapflag) map();
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
			case  8: notify("?",                            0); break;
			case  9: notify("Something unknown!",           0); break;
			case 30: notify("Nothing except air",           0); break;
			case 31: notify("Grass or leaves",              0); break;
			case 32: notify("Stone",                        0); break;
			case 33: notify("It is somebody!",              0); break;
			case 34: notify("Chiken",                       0); break;
			case 35: notify("Careful! Fire",                0); break;
			default: notify("?",                            0); break;
		}
		notflag=notc;
	}
}

int fall(x, y, z)
short x, y, z; {
	short h,
	      save=earth[x][y][z];
	for (h=0; earth[x][y][z-1]==0; --z, ++h);
	earth[x][y][z]=save;
	return(h);
}

int step(x, y)
short x, y; {
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

//this shows what block player is focused on
void focus(px, py, pz)
short *px, *py, *pz; {
	if (pl || view==4) {
		if (eye[0]==0) {
			*px=xp;
			*py=(eye[1]==-1) ? yp-1 : yp+1;
		} else {
			*px=(eye[0]==-1) ? xp-1 : xp+1;
			*py=yp;
		}
		switch (view) {
			case 1:  *pz=zp-1; break; //floor
			case 2:  *pz=zp+1; break; //head
			case 3:  *pz=zp+2; break; //sky
			default: *pz=zp;   break; //surface and front
		}
	} else {
		*px=xp;
		*py=yp;
		*pz=zp-1;
	}
}

//all properties for all blocks
int property(id, c)
short id;
char  c; {
	switch (c) {
		case 's': //stackable
			if (id==6) return(0);
			else return(1);
		break;
		case 't': //transparent
			if (id==0) return(1);
			else return(0);
		break;
		case 'p': //passable
			if (id==0) return(1);
			else return(0);
		break;
	}
}
