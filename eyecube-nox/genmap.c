//generate simple map without attributes and physics
#include <stdio.h>

#define AIR 0
#define STONE 1
#define SOIL 2

char name[99]="map/";

char * makename(int x, int y) {
	short i=4+sprintf(&name[4], "%d", x);
	name[i++]='x';
	i+=sprintf(&name[i], "%d", y);
	name[i]='\0';
	return name;
}

main() {
	FILE * out;
	int i, j, x, y, z;
	for (i=-10; i<=10; ++i)
	for (j=-10; j<=10; ++j) {
		out=fopen(makename(i, j), "w");
		for (x=0; x<20; ++x)
		for (y=0; y<20; ++y) {
			for (z=0; z<110; ++z) fputc(STONE, out);
			for (   ; z<120; ++z) fputc(SOIL, out);
			for (   ; z<200; ++z) fputc(AIR, out);
		}
		fclose(out);
	}
}
