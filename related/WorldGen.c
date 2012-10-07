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

#include <stdio.h>

#define SIZE 1000
#define WORLD_NAME "The_Land_of_Doubts"

main() {
	char world[SIZE][SIZE];
	printf("Creating FREG world: %s...\n", WORLD_NAME);
	register i, j;
	for (i=0; i<SIZE; ++i)
	for (j=0; j<SIZE; ++j) 
		world[i][j]='.';
	
	//borders
	//double border will help to avoid future mistakes
	for (i=0; i<SIZE; ++i) {
		world[i][0]='#';
		world[i][1]='#';
		world[i][SIZE-2]='#';
		world[i][SIZE-1]='#';
	}
	for (j=2; j<SIZE-2; ++j) {
		world[0][j]='#';
		world[1][j]='#';
		world[SIZE-2][j]='#';
		world[SIZE-1][j]='#';
	}

	//world generation starts here

	FILE * outfile=fopen(WORLD_NAME, "w");
	fprintf(outfile, "This is a map of %s, the world for Free-Roaming Elementary Game (FREG).\nYou can edit this file with your text editor, but don't touch double borders [#].\nSize: %dx%d\n", WORLD_NAME, SIZE, SIZE);
	fprintf(outfile, "Map legend:\n'#' - nullstone mountain\n'.' - plain\n");
	if (NULL!=outfile) {
		for (i=0; i<SIZE; ++i, fprintf(outfile, "\n"))
		for (j=0; j<SIZE; ++j)
			fprintf(outfile, "%c", world[i][j]);
		fclose(outfile);
		printf("World created successfully.\n");
	} else
		printf("World created, but not saved. Try again or not.\n");
}
