#include <stdio.h>

main() {
	FILE *file=fopen("sky.txt", "w");
	short x, y;
	for (x=0; x<21; ++x) {
		for (y=0; y<21; ++y) fprintf(file, random_prob(19) ? "." : " ");
		fprintf(file, "\n");
	}
	fclose(file);	
}

int random_prob(prob)
short prob; {
	FILE *file=fopen("/dev/urandom", "rb");
	char c=fgetc(file);
	fclose(file);
	return (128+c<prob*2.55) ? 1 : 0;
}
