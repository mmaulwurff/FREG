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
} inv[][3];

extern short cloth[];

void pocketshow();

//makes square file name from square coordinates
void makename(x, y, name)
short x, y;
char name[]; {
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
	short x, y, n=0;
/*	for (x=0; x<=9; ++x)
	for (y=0; y<=2; ++y)
		inv[x][y].what=inv[x][y].num=2;*/
	for (x=-1; x<=1; ++x)
	for (y=-1; y<=1; ++y) {
		//coordinates for loops
		short i, j, k,
		      nx=n%3,
		      ny=n/3;
		//50!?
		char name[50];
		makename(x, y, name);
		FILE *file=fopen(name, "a+");
		if (getc(file)!='m') {
			//generator
			//TODO: randoms
			for (i=64*nx; i<=63+64*nx; ++i)
			for (j=64*ny; j<=63+64*ny; ++j)
			for (k=0; k<=HEAVEN-1; ++k)
				earth[i][j][k]=0;
			for (i=64*nx; i<=63+64*nx; ++i)
			for (j=64*ny; j<=63+64*ny; ++j)
			for (k=0; k<=80; ++k)
				earth[i][j][k]=1;
			//hills
			for (k=80; k<=88; ++k)
			for (i=64*nx+10+k-81; i<=64*nx+24-k+81; ++i)
			for (j=64*ny+10+k-81; j<=64*ny+24-k+81; ++j)
				earth[i][j][k]=2;
			//pits
			for (k=80; k>=76; --k)
			for (i=64*nx+20-k+80; i<=64*nx+24+k-80; ++i)
			for (j=64*ny+20-k+80; j<=64*ny+24+k-80; ++j)
				earth[i][j][k]=0;
			//walls (rocks?)
			for (i=10+64*nx; i<=20+64*nx; ++i)
			for (k=80; k<=85; ++k)
				earth[i][12+64*ny][k]=2;
			//chiken
			for (k=HEAVEN; earth[40+64*nx][40+64*ny][k-1]==0; --k);
			earth[40+64*nx][40+64*ny][k]=4;
			//sky
			/*for (i=0; i<=38; ++i)
			for (j=0; j<=38; ++j)
				sky[i][j]=0; //blue sky
			sky[20][20]=2; //sun*/
			//clouds ("1") will be here
			//fire
			earth[45+64*nx][45+64*ny][80]=5;
			earth[46+64*nx][45+64*ny][80]=5;
			earth[45+64*nx][46+64*ny][80]=5;
			earth[46+64*nx][46+64*ny][80]=5;
		}
		else {
			//loader
			for (i=64*nx; i<=63+64*nx; ++i)
			for (j=64*ny; j<=63+64*ny; ++j)
			for (k=0; k<=HEAVEN; ++k) earth[i][j][k]=getc(file);
		}
		++n;
		fclose(file);
	}
}

void save() {
	short n;
	for (n=0; n<=8; ++n) {
		char name[50];
		makename(spx+n%3-1, spy+n/3-1, name);
		short i, j, k;
		FILE* file=fopen(name, "w");
		fputc('m', file);
		for (i=64*(n%3); i<=63+64*(n%3); ++i)
		for (j=64*(n/3); j<=63+64*(n/3); ++j)
		for (k=0; k<=HEAVEN; ++k)
			fputc(earth[i][j][k], file);
		fclose(file);
	}
}

void loadgame() {
	//ask what to load - todo
	short i, j;
	char c;
	FILE* file=fopen("save", "a+");
	xp=      getc(file);
	yp=      getc(file);
	zp=      getc(file);
	jump=    getc(file);
	eye[0]=  getc(file)-1;
	eye[1]=  getc(file)-1;
	eyes[0]= getc(file)-1;
	eyes[1]= getc(file)-1;
	pl=      getc(file);
	view=getc(file);
	if ((c=getc(file))=='-') spx=-getc(file);
	else spx=getc(file);
	if ((c=getc(file))=='-') spy=-getc(file);
	else spy=getc(file);
	for (i=0; i<=38; ++i)
	for (j=0; j<=38; ++j)
		sky[i][j]=getc(file);
	for (i=0; i<=9; ++i)
	for (j=0; j<=2; ++j)
		inv[i][j].what=getc(file);
	for (i=0; i<=9; ++i)
	for (j=0; j<=2; ++j)
		inv[i][j].num=getc(file);
	load();
	pocketshow();	
	fclose(file);
}

void savegame() {
//ask save name should be here - todo
	short i, j;
	FILE* file=fopen("save", "w");
	short n=0;
	save();
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
	for (i=0; i<=4; ++i)
		fputc(cloth[i], file);
	fclose(file);
}

//this checks if new squares should be loaded
//TODO: borders, or tor?
void onbound() {
	if (xp<64) {
		xp+=64;
		save();
		--spx;
		load();
	} else if (xp>127) {
		xp-=64;
		save();
		++spx;
		load();
	} else if (yp<64) {
		yp+=64;
		save();
		--spy;
		load();
	} else if (yp>127) {
		yp-=64;
		save();
		++spy;
		load();
	}
}
