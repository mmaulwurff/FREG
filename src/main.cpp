	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include <QtGui>
#include <QString>
#include "header.h"
#include "world.h"
#include "screen.h"

int main(int argc, char *argv[]) {
	QApplication freg(argc, argv);

	QString worldName="The_Land_Of_Doubts";
	unsigned short numShreds=3;
	World * earth;
	FILE * file=fopen((worldName+"_save").toAscii().constData(), "r");
	if ( NULL!=file ) {
		unsigned long longitude, latitude;
		unsigned long time;
		unsigned short spawnX, spawnY, spawnZ;
		fscanf(file, "longitude: %ld\nlatitude: %ld\nspawnX: %hd\n spawnY: %hd\n spawnZ: %hd\ntime: %ld\nSize:%hd\n",
			&longitude, &latitude,
			&spawnX, &spawnY, &spawnZ,
			&time);
		fclose(file);
		earth=new World(worldName, longitude, latitude,
			spawnX, spawnY, spawnZ, time, numShreds);
	} else
		earth=new World(worldName);

	Screen screen(earth);
	screen.show();

/*	FILE * scenario=fopen("scenario.txt", "r");
	int c='.';
	int print_flag=1; //print only if needed, needed nearly everytime
	while ( 'Q'!=c ) {
		if ( NULL!=scenario ) {
			c=fgetc(scenario);
			if ( EOF==c || '\n'==c ) {
				fclose(scenario);
				scenario=NULL;
				c=screen.Getch();
			} else
				fprintf(stderr, "Scenario used, key is: '%c'.\n", c);
		} else
			c=screen.Getch();*/
	return freg.exec();
}
