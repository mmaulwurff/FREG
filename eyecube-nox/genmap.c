//generate simple map without attributes and physics
#include <stdio.h>
#include "subs.h"

#define WIDTH 20
#define HEIGHT 100

void gen_shred(int i, int j) { //this function will be used in server
	char name[99]; //99?
	sprintf(name, "map/%dx%d\0", i, j);
	FILE * out=fopen(name, "w");
	fprintf(out, "This file is an eyecube shred.\nWidth:%d\nHeight:%d\nType:normal\n", WIDTH, HEIGHT);
	unsigned short x, y, z;
	for (x=0; x<WIDTH; ++x)
	for (y=0; y<WIDTH; ++y) {
		for (z=0; z<HEIGHT/4; ++z) fputc(STONE, out);
		for (   ; z<HEIGHT/2; ++z) fputc(SOIL, out);
		for (   ; z<HEIGHT;   ++z) fputc(AIR, out);
	}
	fclose(out);
}

main() {
	int i, j;
	for (i=-10; i<=10; ++i)
	for (j=-10; j<=10; ++j) gen_shred(i, j);
}
