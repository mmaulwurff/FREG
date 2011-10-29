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

short       xp, yp, zp, //player coordinates
            spx, spy,   //player square position
            jump,       //shows if player jumps
            eye[2],     //camera position: 
            eyes[2],    //previous camera position
            pl,         //show player or no
            earth[192][192][HEAVEN+1], //current loaded world
            sky[39][39],
            radar_dist;
unsigned    time;
/*One real minute is an hour in Eyecube.
 *00:00 - 06:00 - night
 *06:00 - 12:00 - morning
 *12:00 - 18:00 - day
 *18:00 - 00:00 - evening
 */

char        view, /*view modes: sUrface, Floor, Head, sKy, fRont, Menu,
                    Inventory, Chest, Workbench, furNace */
            view_last; //save previous view
struct item inv[10][3], //inventory
	    cloth[5];   //(armour), cloth[4].num is an index of the last line of "inv"

//comment out fprintf to turn off debugging
void tolog(string)
char *string; {
	fprintf(stderr, string);
}

//loads a game
void loadgame() {
	tolog("loadgame start\n");
	//TODO: ask what to load
	int  read_int();
	void pocketshow(),
	     load();
	short i, j;
	FILE* file=fopen("save", "r");
	fprintf(stderr, "load\n");
	if (file!=NULL) { //load
		xp=        getc(file);
		yp=        getc(file);
		zp=        getc(file);
		jump=      getc(file);
		eye[0]=    getc(file)-1;
		eye[1]=    getc(file)-1;
		eyes[0]=   getc(file)-1;
		eyes[1]=   getc(file)-1;
		pl=        getc(file);
		view=      getc(file);
		view_last= getc(file);
		radar_dist=getc(file);
		time=read_int(file);
		spx=(((getc(file))=='-') ? (-1) : 1)*getc(file);
		spy=(((getc(file))=='-') ? (-1) : 1)*getc(file);
		for (i=0; i<=38; ++i) for (j=0; j<=38; ++j)
			sky[i][j]=getc(file);
		for (i=0; i<=9; ++i) for (j=0; j<=2; ++j) {
			inv[i][j].what=getc(file);
			inv[i][j].num=getc(file);
		}
		for (i=0; i<=4; ++i) {
			cloth[i].what=getc(file);
			cloth[i].num =getc(file);
		}
		fclose(file);
		load();
	} else { //new game
		int property();
		spy=spx=0;
		load();
		xp=80;
		yp=80;
		for (zp=HEAVEN; property(earth[xp][yp][zp-1], 'p'); --zp);
		jump=0;
		eye[0]=0; //north
		eye[1]=-1;
		eyes[0]=0;
		eyes[1]=-1;
		pl=1;
		time=6*60;
		radar_dist=FAR;
		view=view_last='u';
		for (i=0; i<=38; ++i)
		for (j=0; j<=38; ++j)
			sky[i][j]=0;
		sky[19][19]=2;
		for (i=0; i<=9; ++i) for (j=0; j<=2; ++j) {
			inv[i][j].what=0;
			inv[i][j].num =0;
		}
		for (i=0; i<=4; ++i) {
			cloth[i].what=0;
			cloth[i].num =0;
		}
	}
	pocketshow();
	tolog("loadgame finish\n");
}

//loads or generates squares
void load() {
	tolog("load start\n");
	struct something *spawn();
	void eraseanimals();
	eraseanimals();
	short n=0,
	      i, j, k,
	      nx, ny;
	char name[50]; //50!?
	FILE *file;
	for (n=0; n<=8; ++n) {
		nx=64*(n%3),
		ny=64*(n/3);
		makename(spx+n%3-1, spy+n/3-1, name);
		file=fopen(name, "rb");
		if (!file) {
			//generator
			//TODO: randoms
			for (i=nx; i<=63+nx; ++i)
			for (j=ny; j<=63+ny; ++j)
			for (k=0; k<=HEAVEN-1; ++k)
				earth[i][j][k]=0;
			for (i=nx; i<=63+nx; ++i)
			for (j=ny; j<=63+ny; ++j)
			for (k=0; k<=80; ++k)
				earth[i][j][k]=1;
			//hills
			for (k=80; k<=88; ++k)
			for (i=nx+10+k-81; i<=nx+24-k+81; ++i)
			for (j=ny+10+k-81; j<=ny+24-k+81; ++j)
				earth[i][j][k]=2;
			//pits
			for (k=80; k>=76; --k)
			for (i=nx+20-k+80; i<=nx+24+k-80; ++i)
			for (j=ny+20-k+80; j<=ny+24+k-80; ++j)
				earth[i][j][k]=0;
			//walls (rocks?)
			for (i=10+nx; i<=20+nx; ++i)
			for (k=80; k<=85; ++k)
				earth[i][12+ny][k]=2;
			//chiken
			for (k=HEAVEN; earth[40+nx][40+ny][k-1]==0; --k);
			earth[41+nx][47+ny][k]=4;
			(void)spawn(41+nx, 47+ny, k, NULL);
			//chest
			earth[41+nx][41+ny][k]=7;
			(void)spawn(41+nx, 41+ny, k, NULL);
			//fire
			earth[45+nx][45+ny][80]=5;
			earth[46+nx][45+ny][80]=5;
			earth[45+nx][46+ny][80]=5;
			earth[46+nx][46+ny][80]=5;
			earth[46+nx][46+ny][81]=5;
		} else {//loader
			for (i=nx; i<=63+nx;  ++i)
			for (j=ny; j<=63+ny;  ++j)
			for (k= 0; k<=HEAVEN; ++k) {
				earth[i][j][k]=getc(file);
				(void)spawn(i, j, k, file);
			}
			fclose(file);
		}
	}
	tolog("load finish\n");
}

void savegame() {
	tolog("savegame start\n");
//ask save name should be here - todo
	void save(), write_int();
	save();
	short i, j,
	      n=0;
	FILE* file=fopen("save", "wb");
//	write saving time should be here
	fputc(xp,         file);
	fputc(yp,         file);
	fputc(zp,         file);
	fputc(jump,       file);
	fputc(eye[0]+1,   file);
	fputc(eye[1]+1,   file);
	fputc(eyes[0]+1,  file);
	fputc(eyes[1]+1,  file);
	fputc(pl,         file);
	fputc(view,       file);
	fputc(view_last,  file);
	fputc(radar_dist, file);
	write_int(time, file);
	//spx and spy may be longs, so there should be another way
	if (spx<0) fputc('-', file);
	else       fputc('+', file);
	fputc(abs(spx),  file);
	if (spy<0) fputc('-', file);
	else       fputc('+', file);
	fputc(abs(spy),  file);
	for (i=0; i<=38; ++i)
	for (j=0; j<=38; ++j)
		fputc(sky[i][j], file);
	for (i=0; i<=9; ++i)
	for (j=0; j<=2; ++j) {
		fputc(inv[i][j].what, file);
		fputc(inv[i][j].num, file);
	}
	for (i=0; i<=4; ++i) {
		fputc(cloth[i].what, file);
		fputc(cloth[i].num,  file);
	}
	fclose(file);
	tolog("savegame finish\n");
}

void save() {
	tolog("save start\n");
	short n;
	for (n=0; n<=8; ++n) {
		struct something *findanimal();
		int  property();
		char name[50],
		     type;
		makename(spx+n%3-1, spy+n/3-1, name);
		FILE *file=fopen(name, "wb");
		short i, j, k,
		      if_heap=0;
		for (i=64*(n%3); i<=63+64*(n%3); ++i)
		for (j=64*(n/3); j<=63+64*(n/3); ++j)
		for (k=0;        k<=HEAVEN;      ++k) {
			fputc(earth[i][j][k], file);
			type=property(earth[i][j][k], 'n');
			switch (type) {
				case 'a': //animal
					fputc(findanimal(i, j, k)->arr[3], file);
				break;
				case 'h': case 'c': {//chest or heap
					short count=3;
					struct something *point=findanimal(i, j, k);
					while (count<63+('h'==type) ? 1 : 0)
						fputc(point->arr[count++], file);
				} break;
			}
		}
		fclose(file);
	}
	tolog("save finish\n");
}

//this checks if new squares should be loaded
//TODO: borders, or tor?
void onbound() {
	tolog("onbound start");
	void eraseanimals();
	if (xp<64) { //west
		xp+=64;
		save();
		eraseanimals();
		--spx;
		load();
	} else if (xp>127) { //east
		xp-=64;
		save();
		eraseanimals();
		++spx;
		load();
	} else if (yp<64) { //north
		yp+=64;
		save();
		eraseanimals();
		--spy;
		load();
	} else if (yp>127) { //south
		yp-=64;
		save();
		eraseanimals();
		++spy;
		load();
	}
	tolog("onbound finish\n");
}

//random number (linux only)
int random_linux() {
	tolog("random_linux start\n");
	FILE* file=fopen("/dev/urandom", "rb");
	char c=fgetc(file);
	fclose(file);
	tolog("random_linux finish\n");
	return c;
}

//returns 1 with prob probability, with prob=100 not always 1, with prob=0 always 0
int random_prob(prob)
short prob; {
	tolog("random_prob start\n");
	FILE* file=fopen("/dev/urandom", "rb");
	char c=fgetc(file);
	fclose(file);
	tolog("random_prob finish\n");
	return (128+c<prob*2.55) ? 1 : 0;
}

//write integer into a file
void write_int(d, file)
int  d;
FILE *file; {
	short i=0;
	do {
		fputc(d, file);
		d>>=8*sizeof(char);
	} while (++i<sizeof(int)/(2*sizeof(char)));
}

//read integer from a file
int read_int(file)
FILE  *file; {
	int   d=0;
	short i=0;
	do d+=fgetc(file)<<i*8*sizeof(char);
	while (++i<sizeof(int)/(2*sizeof(char)));
	return d;
}
