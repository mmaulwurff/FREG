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
