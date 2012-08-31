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
