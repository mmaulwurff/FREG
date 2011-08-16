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

extern short earth[][192][HEAVEN+1];
extern short view;

void map(),
     chiken_move();

short mechtoggle;

short chikenx=104,
      chikeny=104,
      chikenz=81;

void allmech() {
	mechtoggle=(mechtoggle) ? 0 : 1;
	chiken_move();
	if (view!='i') map();
}

void chiken_move() {
	FILE* file=fopen("/dev/urandom", "rt");
	short c=(unsigned)fgetc(file)%5;
	earth[chikenx][chikeny][chikenz]=0;
	if (c==0) chikenx++;
	else if (c==1) chikenx--;
	else if (c==2) chikeny++;
	else if (c==3) chikeny--;
	fclose(file);
	earth[chikenx][chikeny][chikenz]=4;
}
