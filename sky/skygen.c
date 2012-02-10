#include <stdio.h>

main() {
	FILE *file=fopen("sky.txt", "w");
	short x, y;
	for (x=0; x<39; ++x) {
		for (y=0; y<39; ++y)
			if (!(x<9 && y<9) && !(x>29 && y<9) &&
			    !(x<9 && y>29) && !(x>29 && y>29) && random_prob(19))
				fprintf(file, ". ");
			else fprintf(file, "  ");
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
