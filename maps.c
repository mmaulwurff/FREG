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

extern char  signal;
extern short earth[][192][HEAVEN+1],
             sky[][39],
             xp, yp, zp,
             spx, spy,
             jump,
             eye[],
             eyes[],
             view,
             pl;

extern struct item {
	short what,
	      num;
} inv[][3],
  cloth[];

extern struct animal {
	short x, y, z,
	      life;
	struct animal *next;
} *animalstart,
  *findanimal();
extern struct thing {
	short x, y, z;
	struct thing *next;
} *thingstart;

void pocketshow(),
     eraseanimals();
int  property();

//makes square file name from square coordinates
void makename(x, y, name)
short x, y;
char name[]; {
//	fprintf(stderr, "makename\n");
	short i=0;
	name[i++]='m';
	name[i++]='a';
	name[i++]='p';
	name[i++]='/';
	if (x<0) {
		x=abs(x);
		name[i++]='-';
	}
	do {
		name[i++]='0'+x % 10;
		x/=10;
	} while (x!=0);
	name[i++]='+';
	if (y<0) {
		y=abs(y);
		name[i++]='-';
	}
	do {
		name[i++]='0'+y % 10;
		y/=10;
	} while (y!=0);
	name[i]='\0';
}

void load() {
//	fprintf(stderr, "load\n");
	eraseanimals();
	short n=0;
	for (n=0; n<=8; ++n) {
		//fprintf(stderr, "loop %d\n", n);
		short i, j, k,
		      nx=64*(n%3),
		      ny=64*(n/3);
		char name[50]; //50!?
		makename(spx+n%3-1, spy+n/3-1, name);
		FILE *file=fopen(name, "rb");
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
			earth[40+nx][40+ny][k]=4;
			spawn(40+nx, 40+ny, k, 9);
			//fire
			earth[45+nx][45+ny][80]=5;
			earth[46+nx][45+ny][80]=5;
			earth[45+nx][46+ny][80]=5;
			earth[46+nx][46+ny][80]=5;
		} else {//loader
			for (i=nx; i<=63+nx; ++i)
			for (j=ny; j<=63+ny; ++j)
			for (k=0; k<=HEAVEN; ++k) {
				earth[i][j][k]=getc(file);
				if (property(earth[i][j][k], 'n'))
					//animals
					spawn(i, j, k, 8/*getc(file)*/);
				/*else if (property(earth[i][j][k], 'h')) {
					//things
					struct thing *thingcar;
					if (thingstart==NULL) thingstart=thingcar=
						malloc(sizeof(struct thing));
					else {
						for (thingcar=thingstart;
							thingcar->next!=NULL;
							thingcar=thingcar->next);
						thingcar=thingcar->next=
							malloc(sizeof(struct thing));
					}
					thingcar->x=i;
					thingcar->y=j;
					thingcar->z=k;
					thingcar->next=NULL;
				}*/
			}
			fclose(file);
		}
	}
	//	signal='w'; //resume movements
}

void save() {
//	fprintf(stderr, "save\n");
	short n;
	for (n=0; n<=8; ++n) {
		char name[50];
		makename(spx+n%3-1, spy+n/3-1, name);
		FILE *file=fopen(name, "wb");
		short i, j, k;
		for (i=64*(n%3); i<=63+64*(n%3); ++i)
		for (j=64*(n/3); j<=63+64*(n/3); ++j)
		for (k=0; k<=HEAVEN; ++k) {
			fputc(earth[i][j][k], file);
		//	if (property(earth[i][j][k], 'n')) //animal
		//		fputc(/*findanimal(i, j, k)->life*/8, file);
		}
		fclose(file);
	}
}

void loadgame() {
//	fprintf(stderr, "loadgame\n");
	//ask what to load - todo
	short i, j;
	FILE* file=fopen("save", "r");
	if (file!=NULL) { //load
		xp=      getc(file);
		yp=      getc(file);
		zp=      getc(file);
		jump=    getc(file);
		eye[0]=  getc(file)-1;
		eye[1]=  getc(file)-1;
		eyes[0]= getc(file)-1;
		eyes[1]= getc(file)-1;
		pl=      getc(file);
		view=    getc(file);
		spx=(((getc(file))=='-') ? (-1) : 1)*getc(file);
		spy=(((getc(file))=='-') ? (-1) : 1)*getc(file);
		for (i=0; i<=38; ++i)
		for (j=0; j<=38; ++j)
			sky[i][j]=getc(file);
		for (i=0; i<=9; ++i)
		for (j=0; j<=2; ++j) {
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
		view='u';
		for (i=0; i<=38; ++i)
		for (j=0; j<=38; ++j)
			sky[i][j]=0;
		sky[19][19]=2;
		for (i=0; i<=9; ++i)
		for (j=0; j<=2; ++j) {
			inv[i][j].what=0;
			inv[i][j].num =0;
		}
		for (i=0; i<=4; ++i) {
			cloth[i].what=0;
			cloth[i].num =0;
		}
	}
	pocketshow();
}

void savegame() {
//	fprintf(stderr, "savegame\n");
//ask save name should be here - todo
	save();
	short i, j,
	      n=0;
	FILE* file=fopen("save", "wb");
//write saving time should be here
	fputc(xp,        file);
	fputc(yp,        file);
	fputc(zp,        file);
	fputc(jump,      file);
	fputc(eye[0]+1,  file);
	fputc(eye[1]+1,  file);
	fputc(eyes[0]+1, file);
	fputc(eyes[1]+1, file);
	fputc(pl,        file);
	fputc(view,      file);
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
	for (j=0; j<=2; ++j)
		fputc(inv[i][j].what, file);
	for (i=0; i<=9; ++i)
	for (j=0; j<=2; ++j)
		fputc(inv[i][j].num, file);
	for (i=0; i<=4; ++i) {
		fputc(cloth[i].what, file);
		fputc(cloth[i].num,  file);
	}
	fclose(file);
}

//this checks if new squares should be loaded
//TODO: borders, or tor?
void onbound() {
//	fprintf(stderr, "onbound\n");
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
}
